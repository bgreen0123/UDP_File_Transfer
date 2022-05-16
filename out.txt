CC=gcc
CFLAGS=-Wall

all: UDPClient UDPServer

UDPClient: UDPClient.o

UDPServer: UDPServer.o

UDPClient.o: UDPClient.c
	$(CC) -c UDPClient.c

UDPServer.o: UDPServer.c 
	$(CC) -c UDPServer.c

clean:
	rm -f UDPClient.o UDPServer.o UDPClient UDPServer out.txt
