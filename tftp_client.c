#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdlib.h>

#include "tftp.h"

char* server_ip;
int server_port;

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

	return 0;
}
