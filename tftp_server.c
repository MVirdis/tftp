#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>

#include "tftp.h"
#include "user.h"

int id_counter = 0;
char* directory;
int port;
int server_socket;
int exit_status;
struct sockaddr_in client_addr;
socklen_t client_addr_len;
struct sockaddr_in server_addr;
char buffer[MAX_REQ_LEN];
user_list_t active_users;
struct user* new_user;
pthread_t thread;

void* serve_user(void* args) {
	struct user* new_user = (struct user*) args;
	#ifdef VERBOSE
	printf("[Server-tt] Servo un nuovo utente.\n");
	#endif

	

	return NULL;
}

int main(int argc, char** argv) {
	// Inizializzo la lista degli utenti
	init_user_list(&active_users);

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
			#ifdef VERBOSE
			printf("[Server] Ricevuta richiesta da un client.\n");
			#endif
			// Aggiungo un nuovo utente alla lista
			new_user = malloc(sizeof(struct user));
			new_user->id = id_counter++;
			new_user->addr = malloc(sizeof(struct sockaddr));
			memcpy(new_user->addr, &client_addr, sizeof(struct sockaddr));
			pthread_cond_init(&new_user->acked, NULL);
			new_user->filename = get_filename(buffer);
			new_user->next = NULL;
			if(add(&active_users, new_user)) {
				#ifdef VERBOSE
				printf("[Server] Errore inserimento utente in lista.\n");
				#endif
				deallocate(new_user);
				id_counter--;
				continue;
			}
			// Avvio un nuovo thread che gestira' l'invio col client
			if(pthread_create(&thread, NULL, serve_user, (void*)new_user)) {
				#ifdef VERBOSE
				printf("[Server] Errore creazione nuovo thread.\n");
				#endif
				deallocate(new_user);
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
