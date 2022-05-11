#include <stdio.h> /*for printf() and fprintf()*/
#include <sys/socket.h> /*for socket(), connect(), send(), and recv()*/
#include <arpa/inet.h> /*for sockaddr_in and inet_addr()*/
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "DieWithError.h"

#define PACKETBUFMAX 80 /*Longest string to echo*/
#define RCVBUFSIZE 32
#define PORT 4567 /*Hardcoded port that we will be sending to*/


int main(int argc, char *argv[])
{
	int sock; /*Socket descriptor*/
	struct sockaddr_in echoServAddr; /* Echo server address */
	struct sockaddr_in fromAddr; /* Source address of echo */
	unsigned short echoServPort;
	unsigned int fromSize;
	char *servIP;
	char echoString[RCVBUFSIZE];
	char fileBuffer[PACKETBUFMAX+1];
	unsigned int echoStringLen;
	int bytesReceived,totalBytesReceived,n;
	FILE *f;

	if (argc<=1 || argc>2)
	{
		fprintf(stderr, "Usage: ./UDPClient <Server IP>\n"), argv[0];
		exit(1);
	}

	servIP = argv[1];

	/* Create a datagram/UDP socket */
	if((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP))<0)
		DieWithError("socket() failed");

	/*Construct the server address structure*/
	memset(&echoServAddr, 0, sizeof(echoServAddr)); /* Zero out structure */
	echoServAddr.sin_family = AF_INET;   /* Internet addr family */
	echoServAddr.sin_addr.s_addr = inet_addr(servIP);   /* Server IP address */
	echoServAddr.sin_port = htons(PORT);  /* Server port */

	/*Get the file the user wants*/
	printf("Enter the file you want: ");
	fgets(echoString,RCVBUFSIZE,stdin);
	echoStringLen = strlen(echoString);
	printf("File name reveived");
	
	/*send the file name to the server*/
	if (sendto(sock, echoString, echoStringLen, 0, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr))!=echoStringLen)
		DieWithError("send() sent a different number of bytes than expected");

	/*Receive a response*/	
	totalBytesReceived=0;
	n=1;
	f=fopen("out.txt","w");
	printf("File opened\n");

	/*While receiving don't stop looping*/
	while(1)
	{
		/*Recieve data until all data is recieved or error*/
		if ((bytesReceived = recvfrom(sock, fileBuffer, PACKETBUFMAX, 0, (struct sockaddr *) &echoServAddr, &fromSize)) <= 0)
		{
			break;
		}
		
		totalBytesReceived += bytesReceived;
		fprintf(f,"%s",fileBuffer);
		printf("Packet %d recieved with %d data bytes\n",n,bytesReceived);
		/*Zero out the buffer so more data can be sent*/
		bzero(fileBuffer,PACKETBUFMAX);
		n++;
	}

	printf("\n");
	close(sock);
	exit(0);
}
