CFLAGS = -Wall

all_verbose: server_verbose client_verbose

all: server client

server: tftp_server.o tftp.o file_utils.o transfer.o
	gcc $(CFLAGS) -lpthread tftp_server.c tftp.c file_utils.c transfer.c -o tftp_server

server_verbose: tftp_server.o tftp.o file_utils.o transfer.o
	gcc $(CFLAGS) -lpthread -DVERBOSE tftp_server.c tftp.c file_utils.c transfer.c -o tftp_server

client: tftp_client.o tftp.o file_utils.o
	gcc $(CFLAGS) tftp_client.c tftp.c file_utils.c -o tftp_client

client_verbose: tftp_client.o tftp.o file_utils.o
	gcc $(CFLAGS) -DVERBOSE tftp_client.c tftp.c file_utils.c -o tftp_client

tftp_server.o: tftp.h file_utils.h transfer.h

tftp_client.o: tftp.h file_utils.h

tftp.o: tftp.h

file_utils.o: file_utils.h

transfer.o: transfer.h

edit_all:
	gedit *.? Makefile & >/dev/null

clean:
	rm -f *.o
	rm -f tftp_server
	rm -f tftp_client
	rm -f downloads/*
