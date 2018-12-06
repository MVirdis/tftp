#include "file_utils.h"

#include <stdio.h>
#include <sys/stat.h>

int get_file_size(char* filepath) {
	struct stat info;
	if (stat(filepath, &info) == -1)
		return -1;
	return info.st_size;
}

int get_file_chunk(char* buff, char* filepath, int offset, int size, int mode) {
	FILE* stream;
	int i;
	if (filepath == NULL || buff == NULL) return -1;
	if ((stream = fopen(filepath, "r")) == NULL) return -1;
	fseek(stream, offset, SEEK_SET);
	if (mode == BIN)
		fread(buff, size, 1, stream);
	else if (mode == TEXT)
		for(i=0; i<size; ++i)
			buff[i] = (char) fgetc(stream);
	fclose(stream);
	return 0;
}

int set_file_chunk(char* data, char* filepath, int offset, int size, int mode) {
	FILE* stream;
	int i;
	if (filepath == NULL || data == NULL) return -1;
	if (offset < 0) return -1;
	// Se devo scrivere il primo blocco sovrascrivo l'eventuale file se presente
	if (offset == 0)
		stream = fopen(filepath, "w");
	else
		stream = fopen(filepath, "a");
	if (stream == NULL) return -1;
	fseek(stream, offset, SEEK_SET);
	if (mode == BIN)
		fwrite(data, size, 1, stream);
	else if (mode == TEXT)
		for(i=0; i<size; ++i)
			fprintf(stream, "%c", data[i]);
	fclose(stream);
	return 0;
}

int append_file_chunk(char* data, char* filepath, int size, int mode) {
	FILE* stream;
	int i;
	if (data == NULL || filepath == NULL) return -1;
	if ((size <= 0 && mode == BIN) || (mode != BIN && mode != TEXT)) return -1;
	stream = fopen(filepath, "a");
	if (mode == BIN)
		fwrite(data, size, 1, stream);
	else if (mode == TEXT)
		for(i=0; i<size; ++i)
			fprintf(stream, "%c", data[i]);
	fclose(stream);
	return 0;
}
