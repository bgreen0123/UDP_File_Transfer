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

typedef struct{
	int count;
	int seq_num;
	char data[SENDBUFSIZE];
}Frame;

typedef struct{
	int ack_seq_num;
}Ack;

int transmit(Frame frame, Ack ack, int sock, struct sockaddr_in clntAddr){
	int bytesSent;

	if((bytesSent = sendto(sock, &frame, sizeof(Frame), 0, (struct sockaddr *) &clntAddr, sizeof(clntAddr))) == -1)
	{
		DieWithError("sendto() failed");
	}

	return bytesSent;
}

int main(int argc, char *argv[])
{

	int sock; /* Socket */
	struct sockaddr_in servAddr; /* Local address */
	struct sockaddr_in clntAddr; /* Client address */
	unsigned int clntAddrLen = sizeof(clntAddr); /* Length of incoming message */
	char echoBuffer[SENDBUFSIZE]; /* Buffer for echo string */
	char fileBuffer[RCVBUFSIZE];
	char data[SENDBUFSIZE];
	int recvFileNameSize,totalBytesSent,bytesSent,n,timeout,lost; /* Size of received message */
	FILE *f;

	Frame send;
	Frame recv;
	Ack ack;
	ack.ack_seq_num = 1;


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
		if ((recvFileNameSize = recvfrom(sock, &recv, sizeof(Frame), 0, (struct sockaddr *) &clntAddr, &clntAddrLen)) < 0)
			DieWithError("recvfrom() failed") ;
		
		
		printf("Handling client %s\n", inet_ntoa(clntAddr.sin_addr));
		recv.data[recv.count-1]='\0';
		printf("File name: %s\n",recv.data);
		f = fopen(recv.data,"r");
		if (f == NULL)
		{
			DieWithError("File doesn't exist");
		}
		printf("File opened for reading\n");
		n = 1;
		bytesSent = 0;
		totalBytesSent = 0;
		while(fgets(send.data,SENDBUFSIZE,f) != NULL)
		{
			send.seq_num = ack.ack_seq_num;
			send.count = strlen(send.data);

			printf("Packet %d with seq num %d generated for transmission with %d data bytes\n",n,send.seq_num,send.count);
			
			/* Send data bytes back to the client */
			bytesSent = transmit(send,ack,sock,clntAddr);
			printf("Packet %d successfully transmitted with %d data bytes\n\n",n,send.count);
			/*Wait for the ACK using a timeout period of 5 seconds*/
			while(timeout != 5)
			{
				printf("Waiting...\n");
				if(recvfrom(sock, &ack, sizeof(Ack), MSG_DONTWAIT, (struct sockaddr *) &clntAddr, &clntAddrLen))
				{
					break;
				}

				sleep(1);

				if((timeout++) == 5)
				{
					bytesSent = transmit(send,ack,sock,clntAddr); /*retransmit and set the timer back to 0*/
					timeout = 0;
				}

			}

			printf("ACK %d with seq num %d received\n\n",n,ack.ack_seq_num);
			bzero(data,SENDBUFSIZE);/*Zero out the buffer to make room for more data*/
			totalBytesSent+=bytesSent;
			n++;
		}
		send.count = 0;
		sendto(sock,&send,sizeof(Frame),0,(struct sockaddr *) &clntAddr, sizeof(clntAddr)); /*Send last packet with no bytes to end transmission*/
		printf("End of transmission packet with sequence number %d transmitted with %d data bytes\n",n,send.count);
	}
/* NOT REACHED BECAUSE THE SERVER IS ALWAYS LISTENING*/
}
