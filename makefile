CC=gcc
CFLAGS=-Wall

all: UDPEchoClient UDPEchoServer

UDPEchoClient: UDPEchoClient.o

UDPEchoServer: UDPEchoServer.o

UDPEchoClient.o: UDPEchoClient.c
	$(CC) -c UDPEchoClient.c

UDPEchoServer.o: UDPEchoServer.c 
	$(CC) -c UDPEchoServer.c

clean:
	rm -f UDPEchoClient.o UDPEchoServer.o UDPEchoClient UDPEchoServer
