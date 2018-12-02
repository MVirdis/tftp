#define MAX_FILENAME_LEN 32
#define MAX_MODE_LEN 9
#define MAX_REQ_LEN (2+MAX_FILENAME_LEN+1+MAX_MODE_LEN+1)
#define MAX_DATA_LEN 516

#define CHUNK_SIZE 512

#define RRQ 1
#define WRQ 2
#define DATA 3
#define ACK 4
#define ERROR 5

#define TEXT_MODE "netascii"
#define BIN_MODE "octet"

int get_opcode(char* buff);
void set_opcode(char* buff, int opcode);
char* get_filename(char* buff);
void set_blocknumber(char* buff, int num);
