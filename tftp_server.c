#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

#include "tftp.h"
#include "transfer.h"
#include "file_utils.h"

int server_socket;
transfer_list_t active_transfers;

void handle_kill(int sig) {
	close(server_socket);
}

void* handle_transfer(void* args) {
	struct transfer* new_transfer = (struct transfer*) args;
	char data_packet[MAX_DATA_LEN];
	char error_packet[MAX_ERROR_LEN];
	char file_chunk[CHUNK_SIZE];
	int chunks;
	int filesize;
	int done=0;
	int next_size=0;
	int error = 0;
	#ifdef VERBOSE
	char paddr[INET_ADDRSTRLEN];
	struct sockaddr_in* addr;
	#endif

	// Mutua esclusione su new_transfer
	pthread_mutex_lock(&new_transfer->mutex);

	#ifdef VERBOSE
	addr = (struct sockaddr_in*)new_transfer->addr;
	inet_ntop(AF_INET, &addr->sin_addr, paddr, INET_ADDRSTRLEN);
	#endif

	// Preparo l'opcode di errore
	set_opcode(error_packet, ERROR);

	if ((filesize = get_file_size(new_transfer->filepath)) == -1) {
		#ifdef VERBOSE
		printf("Errore file not found.\n");
		#endif
		set_errornumber(error_packet, FILE_NOT_FOUND);
		set_errormessage(error_packet, "File not found.");
		next_size = 16;
		error = 1;
	}

	// Calcolo del numero di blocchi
	chunks = (int) (filesize/CHUNK_SIZE + (filesize%CHUNK_SIZE == 0? 0:1));

	#ifdef VERBOSE
	if (error == 0) {
		printf("Inizio trasferimento di %s (%d bytes) in %d blocchi verso %s:%d.\n",
			   new_transfer->filepath, filesize, chunks, paddr, ntohs(addr->sin_port));
	}
	#endif

	while(done < chunks && error == 0) {
		if (done == chunks-1) // Se il prossimo e' l'ultimo blocco
			next_size = filesize - done*CHUNK_SIZE;
		else
			next_size = CHUNK_SIZE;

		// Creo un pacchetto data con il blocco
		set_opcode(data_packet, DATA);
		set_blocknumber(data_packet, done);
		if ((get_file_chunk(file_chunk, new_transfer->filepath,
							done*CHUNK_SIZE, next_size, new_transfer->filemode)) == -1) {
			set_errornumber(error_packet, FILE_NOT_FOUND);
			set_errormessage(error_packet, "File not found.");
			next_size = 16;
			error = 1;
			break;
		}
		set_data(data_packet, file_chunk, next_size);

		// Se ho inviato tutto il pacchetto data_packet
		if(sendto(server_socket, data_packet, DATA_HEADER_LEN+next_size, 0,
			   new_transfer->addr, sizeof(struct sockaddr)) == DATA_HEADER_LEN+next_size) {			
			#ifdef VERBOSE
			printf("Inviato blocco %d a %s:%d\n", done, paddr, ntohs(addr->sin_port));
			#endif
			done++;
			// Sospendo il thread in attesa che arrivi l'ack appropriato
			pthread_cond_wait(&new_transfer->acked, &new_transfer->mutex);
		}
	}

	if (error == 0) {
		#ifdef VERBOSE
		printf("Trasferimento di %s verso %s:%d completato.\n",
			   new_transfer->filepath, paddr, ntohs(addr->sin_port));
		#endif
	} else { // In caso di errore next_size contiene la dimens. del messaggio
		#ifdef VERBOSE
		printf("Si è verificato un errore durante il trasferimento di %s verso "
			   "%s:%d\n",
			   new_transfer->filepath, paddr, ntohs(addr->sin_port));
		#endif
		sendto(server_socket, error_packet, ERROR_HEADER_LEN+next_size, 0,
			   new_transfer->addr, sizeof(struct sockaddr));
	}

	pthread_mutex_unlock(&new_transfer->mutex);
	remove_transfer(&active_transfers, new_transfer);

	pthread_exit(NULL);
}

int main(int argc, char** argv) {
	// Variabili
	int id_counter = 0;
	char* directory;
	char* filepath;
	char* filename;
	int port;
	int exit_status;
	int mode;
	struct sockaddr_in client_addr;
	socklen_t client_addr_len;
	struct sockaddr_in server_addr;
	char buffer[MAX_REQ_LEN];
	struct transfer* new_transfer;
	pthread_t thread;
	#ifdef VERBOSE
	char paddr[INET_ADDRSTRLEN];
	#endif

	// Associo il gestore del segnale kill
	signal(SIGKILL, handle_kill);

	// Inizializzo la lista degli utenti
	init_transfer_list(&active_transfers);

	// Controllo parametri server
	if(argc < 2) {
		printf("Errore parametri.\nChiamare con ./tftp_server <porta> <directory>\n");
		return 0;
	} else if(argc == 2) {
		directory = argv[1];
		port = 69; // Porta di default
	} else {
		port = atoi(argv[1]);
		directory = argv[2];
	}

	#ifdef VERBOSE
		printf("TFTP Directory: %s\n", directory);
		printf("TFTP Port: %d\n", port);
	#endif

	// Inizializza struttura indirizzo server
	memset(&server_addr, 0, sizeof(struct sockaddr_in));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

	// Socket UDP del server
	server_socket = socket(AF_INET, SOCK_DGRAM, 0);

	// Associo l'indirizzo e la porta al socket
	exit_status = bind(server_socket, (struct sockaddr*) &server_addr, sizeof(struct sockaddr_in));
	if (exit_status == -1) {
		printf("Non è stato possibile assegnare la porta %d al server.\n", port);
		printf("Se si utilizza la porta di default (69) eseguire il server da root.\n");
		close(server_socket);
		return 0;
	}

	while(1) {
		// Ricevo un messaggio
		client_addr_len = sizeof(struct sockaddr_in);
		recvfrom(server_socket, buffer, MAX_REQ_LEN, 0,
				 (struct sockaddr*)&client_addr, &client_addr_len);
		if (get_opcode(buffer) == RRQ) {
			// Aggiungo un nuovo utente alla lista
			filename = get_filename(buffer);
			filepath = malloc(strlen(directory) + strlen(filename) +1);
			strcpy(filepath, directory);
			strcat(filepath, filename);

			if (strcmp(get_filemode(buffer), TEXT_MODE) == 0)
				mode = TEXT;
			else
				mode = BIN;

			new_transfer = create_transfer(id_counter++, (struct sockaddr*)&client_addr, filepath, mode);
			
			#ifdef VERBOSE
			inet_ntop(AF_INET, &client_addr.sin_addr, paddr, INET_ADDRSTRLEN);
			printf("Ricevuta richiesta di download per %s in modalità %s da %s:%d \n",
				   filepath, get_filemode(buffer), paddr, ntohs(client_addr.sin_port));
			#endif
			// Libero le stringhe che non servono più
			free(filename);
			free(filepath);

			if(add(&active_transfers, new_transfer)) {
				deallocate(new_transfer);
				id_counter--;
				continue;
			}

			// Avvio un nuovo thread che gestira' l'invio col client
			if(pthread_create(&thread, NULL, handle_transfer, (void*)new_transfer)) {
				deallocate(new_transfer);
				id_counter--;
				continue;
			}
		} else if (get_opcode(buffer) == ACK) {
			// Risveglio il thread che sta gestendo il mittente dell'ack
			new_transfer = get_transfer_byaddr(active_transfers, &client_addr);

			// Transfer non trovato puo' capitare se arriva un ack dopo che
			// il trasferimento si e' chiuso con errore
			if (new_transfer == NULL)
				continue;

			// Mutua esclusione sulla struttura new_transfer
			pthread_mutex_lock(&new_transfer->mutex);
			// Nel frattempo il transfer potrebbe essere stato rimosso dalla lista
			new_transfer = get_transfer_byaddr(active_transfers, &client_addr);
			if (new_transfer == NULL)
				continue;
			pthread_cond_signal(&new_transfer->acked); // Risveglio il thread
			pthread_mutex_unlock(&new_transfer->mutex);
		} else { // Messaggio in nessun formato accettabile
			#ifdef VERBOSE
			printf("Non gestito pacchetto con opcode %d.\n",
				   get_opcode(buffer));
			#endif
		}
	}

	return 0;
}
