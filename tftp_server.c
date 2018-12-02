#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>

#include "tftp.h"
#include "transfer.h"
#include "file_utils.h"

int server_socket;
transfer_list_t active_transfers;

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
	printf("[Server-tt] Servo un nuovo utente.\n");
	#endif

	// Occupo il mutex
	pthread_mutex_lock(&new_transfer->mutex);

	filesize = get_file_size(new_transfer->filepath);
	chunks = (int) (filesize/CHUNK_SIZE +
					(filesize%CHUNK_SIZE == 0? 0:1));

	#ifdef VERBOSE
	printf("[Server-tt] Inizio trasferimento %s (%d bytes) in %d blocchi.\n",
		   new_transfer->filepath, filesize, chunks);
	#endif

	set_opcode(error_packet, ERROR);

	if(chunks == 0) {
		#ifdef VERBOSE
		printf("[Server-tt] Errore file not found.\n");
		#endif
		set_errornumber(error_packet, FILE_NOT_FOUND);
		set_errormessage(error_packet, "File not found.");
		error = 1;
	}

	while(done < chunks && error == 0) {
		if (done == chunks-1)
			next_size = filesize-done*CHUNK_SIZE;
		else
			next_size = CHUNK_SIZE;
		// Creo un pacchetto data con il blocco
		set_opcode(data_packet, DATA);
		set_blocknumber(data_packet, done);
		if ((get_file_chunk(file_chunk, new_transfer->filepath,
							done*CHUNK_SIZE, next_size, new_transfer->filemode)) == -1) {
			set_errornumber(error_packet, FILE_NOT_FOUND);
			set_errormessage(error_packet, "File not found.");
			error = 1;
			break;
		}
		set_data(data_packet, file_chunk, next_size);
		#ifdef VERBOSE
		printf("[Server-tt] Invio blocco %d/%d...\n", done+1, chunks);
		#endif
		if(sendto(server_socket, data_packet, DATA_HEADER_LEN+next_size, 0,
			   new_transfer->addr, sizeof(struct sockaddr)) > 0) {
			done++;
			#ifdef VERBOSE
			printf("[Server-tt] Thread sospeso in attesa di ack.\n");
			#endif
			pthread_cond_wait(&new_transfer->acked, &new_transfer->mutex);
		}
	}

	if (error == 0) {
		#ifdef VERBOSE
		printf("[Server-tt] trasferimento di %s completato.\n", new_transfer->filepath);
		#endif
	} else {
		#ifdef VERBOSE
		printf("[Server-tt] si è verificato un errore durante il trasferimento %d.\n",
			   new_transfer->id);
		#endif
		sendto(server_socket, error_packet, DATA_HEADER_LEN+next_size, 0,
			   new_transfer->addr, sizeof(struct sockaddr));
	}

	remove_transfer(&active_transfers, new_transfer);
	pthread_mutex_unlock(&new_transfer->mutex);

	return NULL;
}

int main(int argc, char** argv) {
	// Variabili
	int id_counter = 0;
	char* directory;
	char* filepath;
	char* filename;
	char paddr[INET_ADDRSTRLEN];
	int port;
	int exit_status;
	int mode;
	struct sockaddr_in client_addr;
	socklen_t client_addr_len;
	struct sockaddr_in server_addr;
	char buffer[MAX_REQ_LEN];
	struct transfer* new_transfer;
	pthread_t thread;

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

	exit_status = bind(server_socket, (struct sockaddr*) &server_addr, sizeof(struct sockaddr_in));
	if (exit_status == -1) {
		printf("Non è stato possibile assegnare la porta %d al server.\n", port);
		printf("Se si utilizza la porta di default eseguire il server da root.\n");
		close(server_socket);
		return 0;
	}

	while(1) {
		// Ricevo un messaggio
		client_addr_len = sizeof(struct sockaddr_in);
		recvfrom(server_socket, buffer, MAX_REQ_LEN, 0,
				 (struct sockaddr*)&client_addr, &client_addr_len);
		if (get_opcode(buffer) == RRQ) {
			// TODO controllare che l'utente non stia gia' scaricando
			// Aggiungo un nuovo utente alla lista
			filename = get_filename(buffer);
			#ifdef VERBOSE
			printf("[Server] Ricevuto nomefile: %s\n", filename);
			#endif
			filepath = malloc(strlen(directory) + strlen(filename) +1);
			strcpy(filepath, directory);
			strcat(filepath, filename);
			#ifdef VERBOSE
			printf("[Server] Calcolata directory: %s\n", filepath);
			#endif
			if (strcmp(get_filemode(buffer), TEXT_MODE) == 0)
				mode = TEXT;
			else
				mode = BIN;
			new_transfer = create_transfer(id_counter++, (struct sockaddr*)&client_addr, filepath, mode);
			
			#ifdef VERBOSE
			inet_ntop(AF_INET, &client_addr.sin_addr, paddr, INET_ADDRSTRLEN);
			printf("[Server] Ricevuta richiesta di download per %s in modalità %s da %s:%d \n",
				   filepath, get_filemode(buffer), paddr, ntohs(client_addr.sin_port));
			#endif
			// non serve più il nome file
			free(filename);
			free(filepath);
			if(add(&active_transfers, new_transfer)) {
				#ifdef VERBOSE
				printf("[Server] Errore inserimento utente in lista.\n");
				#endif
				deallocate(new_transfer);
				id_counter--;
				continue;
			}
			#ifdef VERBOSE
			print_transfer_list(active_transfers);
			#endif
			// Avvio un nuovo thread che gestira' l'invio col client
			if(pthread_create(&thread, NULL, handle_transfer, (void*)new_transfer)) {
				#ifdef VERBOSE
				printf("[Server] Errore creazione nuovo thread.\n");
				#endif
				deallocate(new_transfer);
				id_counter--;
				continue;
			}
		} else { // Messaggio in nessun formato noto
			#ifdef VERBOSE
			printf("[Server] Ricevuto pacchetto con opcode %d.\n",
				   get_opcode(buffer));
			#endif
		}
	}

	return 0;
}
