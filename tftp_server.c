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

char* directory;
int port;
int server_socket;
int exit_status;
struct sockaddr_in client_addr;
socklen_t client_addr_len;
struct sockaddr_in server_addr;
char buffer[MAX_REQ_LEN];
user_list_t active_users;

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
		if (get_message_type(buffer) == RRQ) {
			// Suppongo che l'utente non stia gia' scaricando
			#ifdef VERBOSE
			printf("[Server] Ricevuta richiesta da un client.\n");
			#endif
		} else { // Messaggio in nessun formato noto
			#ifdef VERBOSE
			printf("[Server] Pacchetto sconosciuto da un client.\n");
			#endif
		}
	}

	return 0;
}
