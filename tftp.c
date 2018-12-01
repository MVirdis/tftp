#include "tftp.h"

#include <string.h>

int get_message_type(char* message) {
	short opcode = 0;
	// Prelevo l'opcode
	memcpy(&opcode, message, 2);
	return (int)opcode;
}
