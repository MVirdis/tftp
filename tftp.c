#include "tftp.h"

#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>

int get_opcode(char* message) {
	short opcode = 0;
	// Prelevo l'opcode
	memcpy(&opcode, message, 2);
	return (int)ntohs(opcode);
}

void set_opcode(char* buff, int opcode) {
	uint16_t net_opcode;
	if(!buff) return;
	net_opcode = (uint16_t) htons(opcode);
	memcpy(buff, &net_opcode, 2);
}

char* get_filemode(char* buff) {
	int i;
	if (buff == NULL) return NULL;
	if (get_opcode(buff) != RRQ && get_opcode(buff) != WRQ) return NULL;
	for(i=3; i < MAX_REQ_LEN && buff[i] != '\0'; ++i);
	++i;
	if (strcmp(buff+i, TEXT_MODE) == 0)
		return TEXT_MODE;
	else
		return BIN_MODE;
}

char* get_filename(char* buff) {
	char* filename;
	char* field_filename;
	int filename_len;
	if (!buff)
		return NULL;
	if (get_opcode(buff) != RRQ && get_opcode(buff) != WRQ)
		return NULL;
	field_filename = buff+2;
	filename_len = strlen(field_filename)+1;
	filename = malloc(filename_len);
	strcpy(filename, field_filename);
	return filename;
}

void set_blocknumber(char* buff, int num) {
	uint16_t blocknum;
	if(!buff) return;
	if(num < 0) return;
	if (get_opcode(buff) != DATA && get_opcode(buff) != ACK) return;
	blocknum = (uint16_t) htons(num);
	memcpy(buff+2, &blocknum, 2);
}

void set_data(char* buff, char* data, int size) {
	if (buff == NULL || data == NULL) return;
	if (size <= 0) return;
	if (get_opcode(buff) != DATA) return;
	memcpy(buff+DATA_HEADER_LEN, data, size);
}

void set_errornumber(char* buff, int num) {
	uint16_t errornum;
	if(!buff) return;
	if(num < 0) return;
	if (get_opcode(buff) != ERROR) return;
	errornum = (uint16_t) htons(num);
	memcpy(buff+2, &errornum, 2);
}

void set_errormessage(char* buff, char* message) {
	if (buff == NULL || message == NULL) return;
	strcpy(buff+ERROR_HEADER_LEN, message);
}
