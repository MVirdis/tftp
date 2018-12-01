all: server client

all_verbose: server_verbose client_verbose

server:
	gcc -Wall -lpthread tftp_server.c tftp.c user.c -o tftp_server

server_verbose:
	gcc -Wall -lpthread -DVERBOSE tftp_server.c tftp.c user.c -o tftp_server

client:
	gcc -Wall tftp_client.c tftp.c -o tftp_client

client_verbose:
	gcc -Wall tftp_client.c tftp.c -o tftp_client

tftp_test:
	gcc -Wall -c tftp.c
	rm -f tftp.o

user_test:
	gcc -Wall -c user.c
	rm -f user.o

edit_all:
	gedit * & >/dev/null

clean:
	rm -f *.o
	rm -f tftp_server
	rm -f tftp_client
	clear

