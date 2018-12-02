#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "tftp.h"

#define HELP "!help"
#define MODE "!mode"
#define GET "!get"
#define QUIT "!quit"
#define MAX_CMD_LINE_LEN 64

void print_menu() {
	printf("\n\n\nSono disponibili i seguenti comandi:\n");
	printf("!help --> mostra l'elenco dei comandi disponibili\n");
	printf("!mode {txt|bin} --> imposta il modo di trasferimento dei files (testo o binario)\n");
	printf("!get filename nome_locale --> richiede al server il nome del file "
		   "<filename> e lo salva localmente con il nome <nome_locale>\n");
	printf("!quit --> termina il client\n");
}

int main(int argc, char** argv) {
	char* server_ip;
	int server_port;
	int sock;
	struct sockaddr_in server_addr;
	char command[MAX_CMD_LINE_LEN];
	
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

	print_menu();
	fgets(command, MAX_CMD_LINE_LEN, stdin);
	// TODO esecuzione comando

	close(sock);

	return 0;
}
