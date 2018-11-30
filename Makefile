all: server client tftp link

all_verbose: server_verbose client_verbose tftp_verbose link	

link:
	gcc tftp_server.o tftp.o -o tftp_server
	gcc tftp_client.o tftp.o -o tftp_client

server:
	gcc -Wall tftp_server.c -lpthread -c tftp_server.c

server_verbose:
	gcc -DVERBOSE -Wall tftp_server.c -lpthread -c tftp_server.c

client:
	gcc -Wall tftp_client.c -c tftp_client.c

client_verbose:
	gcc -DVERBOSE -Wall tftp_client.c -c tftp_client.c

tftp_verbose:
	gcc -DVERBOSE -Wall tftp.c -c tftp.c

tftp:
	gcc -Wall tftp.c -c tftp.c

edit_all:
	gedit * & >/dev/null

clean:
	rm -f *.o
	rm -f tftp_server
	rm -f tftp_client
	clear

