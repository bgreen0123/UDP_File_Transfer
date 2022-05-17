/*Brendan Green and Jaden Smith
 * Programming project 2
 * UDPServer.c - This acts as the server that will be giving the file back the data from the file requested. In the case of a lost data packet it will resend it adding reliability. At the end it will print a seris of values the user may be interested in.
 * */

#include <stdio.h> /*for printf() and fprintf()*/
#include <sys/socket.h> /*for socket(), connect(), send(), and recv()*/
#include <arpa/inet.h> /*for sockaddr_in and inet_addr()*/
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include "DieWithError.h"

#define RCVBUFSIZE 32
#define SENDBUFSIZE 80
#define PORT 4567

typedef struct{
	int count;
	int seq_num;
	char data[SENDBUFSIZE];
}Frame;

int simulateLoss(double loss_ratio){
	double random = rand() / ((double)RAND_MAX); /*Give a number between 0 and 1*/
	/*If the random value is less than the packet loss ratio return 1*/
	if(loss_ratio>random)
		return 1;
	/*Else return 0*/
	return 0;
}

int main(int argc, char *argv[])
{
	srand(time(0)); /*Seeding for randomness*/

	int sock; /* Socket */
	struct sockaddr_in servAddr; /* Local address */
	struct sockaddr_in clntAddr; /* Client address */
	unsigned int clntAddrLen = sizeof(clntAddr); /* Length of incoming message */
	char fileBuffer[RCVBUFSIZE];
	char data[SENDBUFSIZE];
	int recvFileNameSize, bytesSent, n, lost;/* Size of received message */
	int seq = 1, ack = 0;
	double packet_loss;
	int timeout;
	FILE *f;
	int totalBytesSent = 0, num_pkts = 0, num_generated = 0, num_success = 0, num_timeouts = 0, num_acks = 0, num_timeout = 0, num_dropped = 0, num_retransmit = 0;

	Frame send;
	Frame recv;
	int ack_pkt;


	if (argc != 3) /* Test for correct number of arguments */
	{
		fprintf(stderr, "Usage: %s <Timeout> <Packet loss ratio>\n", argv[0]) ;
		exit(1);
	}

	/*User should have given timeout value between 1-10 and packet loss ratio between 0-1*/
	timeout = atoi(argv[1]);
	packet_loss = atof(argv[2]);


	/* Create socket for incoming connections */
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		DieWithError( "socket () failed") ;

	/* Construct local address structure */
	memset(&servAddr, 0, sizeof(servAddr)); /* Zero out structure */
	servAddr.sin_family = AF_INET; /* Internet address family */
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
	servAddr.sin_port = htons(PORT); /* Local port */

	/* Bind to the local address */
	if (bind(sock, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
		DieWithError ( "bind () failed");


	/* Block until receive file name from a client */
	if ((recvFileNameSize = recvfrom(sock, &recv, sizeof(Frame), MSG_WAITALL, (struct sockaddr *) &clntAddr, &clntAddrLen)) < 0)
		DieWithError("recvfrom() failed");
			
	printf("Handling client %s\n", inet_ntoa(clntAddr.sin_addr));
	recv.data[recv.count-1]='\0';/*Not including new line*/
	printf("File name: %s\n",recv.data);
	f = fopen(recv.data,"r");/*Open file for reading*/
	if (f == NULL)
	{
		DieWithError("File doesn't exist");
	}

	/*Set timeout for incoming packets*/
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = pow(10.0,(double) timeout);
	if(setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO, &tv, sizeof(tv))<0){
		DieWithError("setsockopt() failed");
	}

	/*While there is still data to read keep going*/
	while(fgets(send.data,SENDBUFSIZE,f) != NULL)
	{
		lost = simulateLoss(packet_loss);/*Find if the packet will be lost or not*/

		send.seq_num = seq;/*First time will be 1*/
		send.count = strlen(send.data);/*Get the amount of data*/
		
		num_generated++;/*Increase the number of packets generated*/
		totalBytesSent+=send.count;/*Increase the total intial bytes*/
		
		printf("Packet %d generated for transmission with %d data bytes\n",seq,send.count);
			
		/*If 0 is returned transmit normally*/
		if(!lost){
			ack = 0;
			while(ack == 0){

				num_pkts++;/*Increase the total number of packets*/
				/*Send the client data*/
				if((bytesSent = sendto(sock, &send, sizeof(Frame), MSG_CONFIRM, (struct sockaddr *) &clntAddr, sizeof(clntAddr))) == -1){
					DieWithError("sendto() failed");
				}

				/*Receive the ack*/
				n = 0;
				n = recvfrom(sock, &ack_pkt, sizeof(int), MSG_WAITALL, (struct sockaddr *) &clntAddr, &clntAddrLen);/*This is waiting for ACK for 10^timeout microseconds*/

				if(n>=0){/*An ACK was received*/
					printf("ACK %d received\n",ack_pkt);
					num_acks++;/*Increase the number of ACKs received*/
				}
				if(n==-1 || ack_pkt != seq){/*Not correct ACK was received*/
					if(n == -1){
						printf("Timeout expired for packet numbered %d\n",seq);
						num_timeout++;/*Increase the number of times timeout occured*/
					}
					printf("Packet %d generated for re-transmission with %d data bytes\n",seq,send.count);
					num_retransmit++;/*Increase the number of re-transmitted packets*/
				}
				else if(ack_pkt==seq){/*The ACK we recieved is the correct one*/
					printf("Packet %d successfully transmitted with %d data bytes\n\n",seq,send.count);
					num_success++;
					ack = 1;/*If the correct ACK was found we don't want to keep going, break the loop*/
				}
			}
		}
		/*If 1 is returned packet is lost*/
		else{
			printf("Packet %d lost\n",seq);
			num_dropped++;/*Increase the number of packets dropped due to loss*/
		}
		seq = (seq+1)%2;	
		bzero(data,SENDBUFSIZE);/*Zero out the buffer to make room for more data*/
	}
		send.count = 0;
		send.seq_num = seq;
		bzero(send.data,SENDBUFSIZE);
		sendto(sock,&send,sizeof(Frame),MSG_CONFIRM,(struct sockaddr *) &clntAddr, sizeof(clntAddr)); /*Send last packet with no bytes to end transmission*/
		printf("End of transmission packet with sequence number %d transmitted with %d data bytes\n",send.seq_num,send.count);
		fclose(f);

		printf("Number of data packets generated for transmission: %d\nTotal number of data bytes generated for transmission, initial transmission only: %d\nTotal number of data packets generated for retransmission: %d\nNumber of data packets dropped due to loss: %d\nNumber of data packets transmitted successfully: %d\nNumber of ACKs received: %d\nCount of how many times timeout expired: %d",num_pkts,totalBytesSent,num_retransmit,num_dropped,num_pkts+num_retransmit,num_acks,num_timeouts);

		exit(0);
}
