#include "tftp.h"

#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>

int get_opcode(char* message) {
	short opcode = 0;
	// Prelevo l'opcode
	memcpy(&opcode, message, 2);
	return (int)ntohs(opcode);
}

char* get_filename(char* buff) {
	char* filename;
	char* field_filename;
	int filename_len;
	if (get_opcode(buff) != RRQ && get_opcode(buff) != WRQ)
		return NULL;
	field_filename = buff+2;
	filename_len = strlen(field_filename)+1;
	filename = malloc(filename_len);
	strcpy(filename, field_filename);
	return filename;
}
