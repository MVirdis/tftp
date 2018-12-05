#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "tftp.h"
#include "file_utils.h"

#define HELP "!help"
#define MODE "!mode"
#define GET "!get"
#define QUIT "!quit"
#define MAX_CMD_LINE_LEN 64
#define PROMPT "> "
#define GENERIC_ERROR_MSG "Comando non trovato. Usare !help per l'elenco dei comandi.\n"

void print_menu() {
	printf("\n\n\nSono disponibili i seguenti comandi:\n");
	printf("!help --> mostra l'elenco dei comandi disponibili\n");
	printf("!mode {txt|bin} --> imposta il modo di trasferimento dei files (testo o binario)\n");
	printf("!get filename nome_locale --> richiede al server il file di nome "
		   "<filename> e lo salva localmente con il nome <nome_locale>\n");
	printf("!quit --> termina il client\n");
}

int main(int argc, char** argv) {
	char* server_ip;
	int server_port;
	int sock;
	struct sockaddr_in server_addr;
	char command[MAX_CMD_LINE_LEN];
	char* tok;
	char* filemode;
	char* filename;
	char* data;
	char req_packet[MAX_REQ_LEN];
	char buffer[MAX_ERROR_LEN];
	int filename_len;
	int received;
	int error_number;
	int mode;
	int block;
	
	// Controllo parametri client
	if(argc < 2) {
		printf("Errore parametri.\nChiamare con ./tftp_client <IP server> <porta server>\n");
		return 0;
	} else if(argc == 2) {
		server_ip = argv[1];
		server_port = 69; // Porta di default
	} else {
		server_port = atoi(argv[2]);
		server_ip = argv[1];
	}

	#ifdef VERBOSE
	printf("Indirizzo Server: %s\n", server_ip);
	printf("Porta Server: %d\n", server_port);
	#endif

	// filemode a BIN di default
	filemode = malloc(10);
	strcpy(filemode, BIN_MODE);

	// Inizializza indirizzo server
	memset(&server_addr, 0, sizeof server_addr);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(server_port);
	inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

	// Creazione socket UDP
	sock = socket(AF_INET, SOCK_DGRAM, 0);

	// Associo alla porta UDP l'indirizzo del server per comodita'
	connect(sock, (struct sockaddr*)&server_addr, sizeof server_addr);

	print_menu();
	while(1) {
		printf(PROMPT);
		fgets(command, MAX_CMD_LINE_LEN, stdin);
		// Rimuovo il carattere \n finale
		command[strlen(command)-1] = '\0';

		if (strlen(command) == 0) {
			printf(GENERIC_ERROR_MSG);
			continue;
		}

		// Prelevo il primo token
		tok = strtok(command, " ");
		if (strcmp(tok, HELP) == 0) {
			print_menu();
		} else if (strcmp(tok, MODE) == 0) {
			tok = strtok(NULL, " ");
			if (tok == NULL) {
				printf("Formato dell'istruzione mode errato\n");
				printf("Utilizzo !mode {txt|bin}\n");
				continue;
			}
			if (strcmp(tok, "txt") == 0) {
				printf("Modo di trasferimento testuale configurato\n");
				strcpy(filemode, TEXT_MODE);
			} else if (strcmp(tok, "bin") == 0) {
				printf("Modo di trasferimento binario configurato\n");
				strcpy(filemode, BIN_MODE);
			} else {
				printf("Formato dell'istruzione mode errato\n");
				printf("Utilizzo !mode {txt|bin}\n");
			}
		} else if (strcmp(tok, QUIT) == 0) {
			if (filemode != NULL) free(filemode);
			break;
		} else if (strcmp(tok, GET) == 0) {
			if (filemode == NULL) {
				printf("Prima di richiedere un file settare la modalità "
					   "attraverso il comando mode\n");
				printf("Utilizzo !mode {txt|bin}\n");
				continue;
			}
			set_opcode(req_packet, RRQ);
			// Prelevo il filename
			tok = strtok(NULL, " ");
			if (tok == NULL) {
				printf("Formato dell'istruzione get errato\n");
				printf("Utilizzo !get filename nome_locale\n");
				continue;
			}
			filename_len = strlen(tok);
			set_filename(req_packet, tok);
			set_filemode(req_packet, filemode);
			tok = strtok(NULL, " ");
			if (tok == NULL) {
				printf("Formato dell'istruzione get errato\n");
				printf("Utilizzo !get filename nome_locale\n");
				continue;
			}
			if ((send(sock, req_packet, REQ_HEADER_LEN+filename_len+1+strlen(filemode)+1,0)) <= 0) {
				printf("Non è stato possibile inviare la richiesta\n");
				continue;
			}
			filename = get_filename(req_packet);
			printf("Richiesta file %s al server in corso.\n", filename);
			free(filename);
			block = -1;
			// Ricevo un pacchetto di risposta alla richiesta
			while(1) {
				received = recv(sock, buffer, MAX_ERROR_LEN, 0);
				if (get_opcode(buffer) == ERROR) {
					error_number = get_errornumber(buffer);
					if (error_number == FILE_NOT_FOUND) {
						printf("File non trovato.\n");
					} else if (error_number == ILLEGAL_TFTP_OP) {
						printf("Operazione tftp non valida.\n");
					}
					break;
				} else if (get_opcode(buffer) == DATA) {
					data = get_data(buffer);
					if (strcmp(filemode, TEXT_MODE) == 0) mode = TEXT;
					else mode = BIN;
					// |DATA_HEADER|      DATA      |\0|					
					block = get_blocknumber(buffer);
					// Se si tratta del primo blocco stampo
					if (block == 0)
						printf("Trasferimento file in corso.\n");
					printf("\rTrasferimento blocco %d", block);
					// tok punta al nome locale del file
					append_file_chunk(data, tok, received-DATA_HEADER_LEN, mode);
					free(data);
					// Preparo il pacchetto di ack
					set_opcode(buffer, ACK);
					set_blocknumber(buffer, block);
					send(sock, buffer, ACK_LEN, 0); // Invio l'ack
					// Se il campo data e' piu' piccolo di un blocco allora era l'ultimo
					if (received-DATA_HEADER_LEN < CHUNK_SIZE)
						break;
				} else {
					printf("Opcode errato\n");
					break;
				}
			}
			printf("\nTrasferimento completato (%d/%d blocchi)\n", block+1, block+1);
			printf("Salvataggio %s completato\n", tok);
		} else {
			printf(GENERIC_ERROR_MSG);
		}
	}

	close(sock);

	return 0;
}
