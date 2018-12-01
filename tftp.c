#include "tftp.h"

#include <string.h>
#include <arpa/inet.h>

int get_opcode(char* message) {
	short opcode = 0;
	// Prelevo l'opcode
	memcpy(&opcode, message, 2);
	return (int)ntohs(opcode);
}
