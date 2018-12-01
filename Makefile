all: server client tftp user link

all_verbose: server_verbose client_verbose tftp_verbose user_verbose link

link:
	gcc tftp_server.o tftp.o user.o -o tftp_server
	gcc tftp_client.o tftp.o -o tftp_client
	rm -f *.o

server:
	gcc -Wall -lpthread -c tftp_server.c

server_verbose:
	gcc -DVERBOSE -Wall -lpthread -c tftp_server.c

client:
	gcc -Wall -c tftp_client.c

client_verbose:
	gcc -DVERBOSE -Wall -c tftp_client.c

tftp:
	gcc -Wall -c tftp.c

tftp_verbose:
	gcc -DVERBOSE -Wall -c tftp.c

user:
	gcc -Wall -lpthread -c user.c

user_verbose:
	gcc -Wall -lpthread -DVERBOSE -c user.c

edit_all:
	gedit * & >/dev/null

clean:
	rm -f *.o
	rm -f tftp_server
	rm -f tftp_client
	clear

