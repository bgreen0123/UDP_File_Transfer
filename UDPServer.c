#include <stdio.h> /*for printf() and fprintf()*/
#include <sys/socket.h> /*for socket(), connect(), send(), and recv()*/
#include <arpa/inet.h> /*for sockaddr_in and inet_addr()*/
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "DieWithError.h"

#define RCVBUFSIZE 32
#define SENDBUFSIZE 80
#define PORT 4567

void DieWithError(char *errorMessage); /* Error handling function */

int main(int argc, char *argv[])
{

	int sock; /* Socket */
	struct sockaddr_in servAddr; /* Local address */
	struct sockaddr_in clntAddr; /* Client address */
	unsigned int clntAddrLen = sizeof(clntAddr); /* Length of incoming message */
	char echoBuffer[SENDBUFSIZE]; /* Buffer for echo string */
	char fileBuffer[RCVBUFSIZE];
	char data[SENDBUFSIZE];
	int recvFileNameSize,totalBytesSent,bytesSent,n; /* Size of received message */
	FILE *f;

	if (argc > 1) /* Test for correct number of arguments */
	{
		fprintf(stderr, "Usage: %s\n", argv[0]) ;
		exit(1);
	}

	/* Create socket for incoming connections */
	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		DieWithError( "socket () failed") ;

	/* Construct local address structure */
	memset(&servAddr, 0, sizeof(servAddr)); /* Zero out structure */
	servAddr.sin_family = AF_INET; /* Internet address family */
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
	servAddr.sin_port = htons(PORT); /* Local port */

	/* Bind to the local address */
	if (bind(sock, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
		DieWithError ( "bind () failed");


	for (;;) /* Run forever */
	{
		
		/* Block until receive file name from a client */
		if ((recvFileNameSize = recvfrom(sock, fileBuffer, RCVBUFSIZE, 0, (struct sockaddr *) &clntAddr, &clntAddrLen)) < 0)
			DieWithError("recvfrom() failed") ;
		
		
		printf("Handling client %s\n", inet_ntoa(clntAddr.sin_addr));
		fileBuffer[recvFileNameSize-1]='\0';
		printf("File name: %s\n",fileBuffer);
		f = fopen(fileBuffer,"r");
		if (f == NULL)
		{
			DieWithError("File doesn't exist");
		}
		printf("File opened for reading\n");
		n = 1;
		bytesSent = 0;
		totalBytesSent = 0;
		while(fgets(data,SENDBUFSIZE,f) != NULL)
		{
			/* Send received datagram back to the client */
			if((bytesSent = sendto(sock, data, SENDBUFSIZE, 0, (struct sockaddr *) &clntAddr, sizeof(clntAddr))) == -1)
			{
				DieWithError("sendto() failed");
				
			}
			else
			{
				totalBytesSent += bytesSent;
				printf("Packet %d transmitted with %d date bytes\n",n,bytesSent);
				bzero(data,SENDBUFSIZE);/*Zero out the buffer to make room for more data*/
				n++;
			}
		}
		bytesSent = 0;
		sendto(sock,data,bytesSent,0,(struct sockaddr *) &clntAddr, sizeof(clntAddr)); /*Send last packet with no bytes to end transmission*/
		printf("End of transmission packet with sequence number %d transmitted with %d data bytes\n",n,bytesSent);
	}
/* NOT REACHED BECAUSE THE SERVER IS ALWAYS LISTENING*/
}
