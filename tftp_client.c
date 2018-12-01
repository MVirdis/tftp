#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "tftp.h"

char* server_ip;
int server_port;
int sock;
struct sockaddr_in server_addr;
int sent;

int main(int argc, char** argv) {
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

	// Inizializza indirizzo server
	memset(&server_addr, 0, sizeof server_addr);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(server_port);
	inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

	// Creazione socket UDP
	sock = socket(AF_INET, SOCK_DGRAM, 0);

	sent = sendto(sock, "Hellooo", 8, 0,
				  (struct sockaddr*)&server_addr, sizeof server_addr);

	close(sock);

	return 0;
}
