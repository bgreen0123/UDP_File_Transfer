#include <stdio.h> /*for printf() and fprintf()*/
#include <sys/socket.h> /*for socket(), connect(), send(), and recv()*/
#include <arpa/inet.h> /*for sockaddr_in and inet_addr()*/
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "DieWithError.h"

#define ECHOMAX 255   /* Longest string to echo */
#define RCVBUFSIZE 32
#define SENDBUFSIZE 80
#define SERVERPORT 4567

void DieWithError(char *errorMessage); /* Error handling function */

int main(int argc, char *argv[])
{

	int sock; /* Socket */
	struct sockaddr_in echoServAddr; /* Local address */
	struct sockaddr_in echoClntAddr; /* Client address */
	unsigned int cliAddrLen; /* Length of incoming message */
	char echoBuffer[SENDBUFSIZE]; /* Buffer for echo string */
	char fileBuffer[RCVBUFSIZE];
	char data[SENDBUFSIZE];
	int recvMsgSize,totalBytesRcvd,bytesRcvd,n; /* Size of received message */
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
	memset(&echoServAddr, 0, sizeof(echoServAddr)); /* Zero out structure */
	echoServAddr.sin_family = AF_INET; /* Internet address family */
	echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
	echoServAddr.sin_port = htons(SERVERPORT); /* Local port */

	/* Bind to the local address */
	if (bind(sock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) < 0)
		DieWithError ( "bind () failed");


	for (;;) /* Run forever */
	{
		/* Set the size of the in-out parameter */
		cliAddrLen = sizeof(echoClntAddr);
		
		/* Block until receive message from a client */
		if ((recvMsgSize = recvfrom(sock, fileBuffer, RCVBUFSIZE, 0, (struct sockaddr *) &echoClntAddr, &cliAddrLen)) < 0)
			DieWithError("recvfrom() failed") ;
		
		
		printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));
		fileBuffer[recvMsgSize-1]='\0';
		f = fopen(fileBuffer,"r");
		if (f==NULL)
		{
			DieWithError("File doesn't exist");
		}
		printf("File opened for reading");
		n=1;
		bytesRcvd=0;
		totalBytesRcvd = 0;
		while(fgets(data,SENDBUFSIZE,f)!=NULL)
		{
			/* Send received datagram back to the client */
			if ((bytesRcvd=sendto(sock, data, SENDBUFSIZE, 0, (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr))) == -1)
			{
				DieWithError("sendto() failed");
			}

			totalBytesRcvd += bytesRcvd;
			printf("Packet %d transmitted with %d date bytes\n",n,bytesRcvd);
			bzero(data,SENDBUFSIZE);/*Zero out the buffer to make room for more data*/
			n++;
		}
		bytesRcvd=0;
		printf("End of transmission packet with sequence number %d transmitted with %d data bytes\n",n,bytesRcvd);

		//close(sock);
}
/* NOT REACHED */
}
