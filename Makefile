CFLAGS = -Wall

all: server client

install:
	sudo cp tftp_server /bin
	sudo cp tftp_client /bin

uninstall:
	sudo rm -f /bin/tftp_server
	sudo rm -f /bin/tftp_client

server: tftp_server.o tftp.o file_utils.o transfer.o
	gcc $(CFLAGS) -lpthread tftp_server.c tftp.c file_utils.c transfer.c -o tftp_server

client: tftp_client.o tftp.o file_utils.o
	gcc $(CFLAGS) tftp_client.c tftp.c file_utils.c -o tftp_client

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
