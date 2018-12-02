all: server client

all_verbose: server_verbose client_verbose

server:
	gcc -Wall -lpthread tftp_server.c tftp.c file_utils.c transfer.c -o tftp_server

server_verbose:
	gcc -Wall -lpthread -DVERBOSE tftp_server.c tftp.c file_utils.c transfer.c -o tftp_server

client:
	gcc -Wall tftp_client.c tftp.c -o tftp_client

client_verbose:
	gcc -Wall tftp_client.c tftp.c -o tftp_client

tftp_test:
	gcc -Wall -c tftp.c
	rm -f tftp.o

transfer_test:
	gcc -Wall -c transfer.c
	rm -f transfer.o

file_utils_test:
	gcc -Wall -c file_utils.c
	rm -f file_utils.o

edit_all:
	gedit * & >/dev/null

clean:
	rm -f *.o
	rm -f tftp_server
	rm -f tftp_client
	clear

