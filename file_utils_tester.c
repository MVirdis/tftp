#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>

#include "file_utils.h"

int main() {
	// Testo get_file_size
	printf("Dimensione del file files/test.txt: %d\n", get_file_size("./files/test.txt"));

	// Testo get_file_chunk
	char data[515];
	get_file_chunk(data, "./files/test.txt", 1, 10, BIN);
	data[10] = '\0';
	printf("Contenuto del file da 1 a 11: %s\n", data);

	// Test set_file_chunk
	printf("Scrivo nel file ./files/testing.txt 512 caratteri 'a'\n");
	memset(data, 'a', 512);
	set_file_chunk(data, "./files/testing.txt", 0, 512, BIN);
	printf("Scrivo nel file ./files/testing.txt 512 caratteri '\\n' + 511*'a'\n");
	memset(data, 'a', 512);
	data[0] = '\n';
	set_file_chunk(data, "./files/testing.txt", 512, 512, BIN);
	printf("Sovrascrivo i primi 10 caratteri dalla posizione 1 con 'b'\n");
	memset(data, 'b', 10);
	set_file_chunk(data, "./files/testing.txt", 1, 10, BIN);

	return 0;
}
