#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include "tftp.h"

char* directory;
int port;

int main(int argc, char** argv) {
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

	return 0;
}
