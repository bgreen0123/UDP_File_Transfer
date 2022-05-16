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

typedef struct{
	int count;
	int seq_num;
	char data[PACKETBUFMAX];
}Frame;

int main(int argc, char *argv[])
{
	int sock; /*Socket descriptor*/
	struct sockaddr_in servAddr; /* Echo server address */
	struct sockaddr_in fromAddr; /* Source address of echo */
	unsigned short echoServPort;
	unsigned int fromSize = sizeof(fromAddr);
	char *servIP;
	char fileName[RCVBUFSIZE];
	char fileBuffer[PACKETBUFMAX+1];
	unsigned int fileNameLen;
	int bytesReceived,totalBytesReceived,n;
	FILE *f;

	Frame send;
	Frame recv;
	int ack = 0;

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
	memset(&servAddr, 0, sizeof(servAddr)); /* Zero out structure */
	servAddr.sin_family = AF_INET;   /* Internet addr family */
	servAddr.sin_addr.s_addr = inet_addr(servIP);   /* Server IP address */
	servAddr.sin_port = htons(PORT);  /* Server port */

	/*Get the file the user wants*/
	printf("Enter the file you want: ");
	fgets(fileName,RCVBUFSIZE,stdin);
	fileNameLen = strlen(fileName);
	printf("File name reveived");
	send.count = fileNameLen;
	send.seq_num = 0;
	strcpy(send.data,fileName);
	
	/*send the file name to the server*/
	if (sendto(sock, &send, sizeof(Frame), 0, (struct sockaddr *)&servAddr, sizeof(servAddr))!=sizeof(Frame))
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
		bytesReceived = recvfrom(sock, &recv, sizeof(Frame), 0, (struct sockaddr *) &fromAddr, &fromSize);
		if(recv.count == 0 || bytesReceived == -1)
		{
			if(bytesReceived == -1)
			{
				DieWithError("recvfrom() failed");
			}

			printf("Tranfer finished");
			fclose(f);
			close(sock);
			exit(0);
		}
		else
		{
			totalBytesReceived += recv.count;
			recv.data[recv.count] = '\0';
			fputs(recv.data,f);
			printf("Packet %d recieved with %d data bytes\n",n,recv.count);
			bzero(recv.data,PACKETBUFMAX);/*Zero out buffer so that more data can be received*/
			n++;/*Increase the packet count*/
		}
	}
}
