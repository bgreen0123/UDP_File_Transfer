/*Brendan Green and Jaden Smith
 * Programming project 2
 * UDPClient.c - This is acts as a client that is asking for a file and wants to get it back. It will send the user given file name to the server and take back in the data from that file. It will store the data in out.txt. If there is a loss in an ACK packet the server will resend the data packet after timeout and the client will see it is duplicate and resend the same ACK.*/

#include <stdio.h> /*for printf() and fprintf()*/
#include <sys/socket.h> /*for socket(), connect(), send(), and recv()*/
#include <arpa/inet.h> /*for sockaddr_in and inet_addr()*/
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "DieWithError.h"

#define PACKETBUFMAX 80 /*Longest string to echo*/
#define RCVBUFSIZE 32
#define PORT 4567 /*Hardcoded port that we will be sending to*/
#define SERVIP "127.0.0.1"/*Hardcoded local host*/

typedef struct{
	int count;
	int seq_num;
	char data[PACKETBUFMAX];
}Frame;

int simulateAckLoss(double ack_loss_ratio){
	double random = rand() / (double)RAND_MAX;/*Give random number between 0-1*/
	/*If the number is less than ack loss ration return 1*/
	if(random<ack_loss_ratio)
		return 1;
	/*Else return 0*/
	return 0;
}

int main(int argc, char *argv[])
{
	srand(time(0));/*Seed for the random number*/

	int sock; /*Socket descriptor*/
	struct sockaddr_in servAddr; /* Echo server address */
	struct sockaddr_in fromAddr; /* Source address of echo */
	unsigned int fromSize = sizeof(fromAddr);
	char fileName[RCVBUFSIZE];/*Hold the file name*/
	char fileBuffer[PACKETBUFMAX+1];/*Holds the file contents*/
	unsigned int fileNameLen;/*Length of file name*/
	int bytesReceived,n;/*Holds the bytes received by recvfrom()*/
	FILE *f;/*Will hold the file to write in*/
	double ack_loss_ratio;/*User input for loss ratio*/
	char *servIP;/*IP address used*/
	int lost;/*Used to hold if there was a loss*/
	int duplicate = 0;/*Make sure to keep track of last sequence numbers to check for duplicates*/
	int num_pkts = 0, num_acks_no_loss = 0, num_lost = 0, duplicates = 0, num_no_dups = 0,totalBytesReceived = 0,num_dropped = 0;

	Frame send;/*Struct to send packets*/
	Frame recv;/*Struct to receive packets*/
	int ack_pkt;/*Holds ACK number*/

	if (argc!=2)
	{
		fprintf(stderr, "Usage: ./UDPClient <ACK Loss Rate>\n"), argv[0];
		exit(1);
	}

	servIP = SERVIP;
	ack_loss_ratio = atof(argv[1]);

	/* Create a datagram/UDP socket */
	if((sock = socket(AF_INET, SOCK_DGRAM,0))<0)
		DieWithError("socket() failed");

	/*Construct the server address structure*/
	memset(&servAddr, 0, sizeof(servAddr)); /* Zero out structure */
	servAddr.sin_family = AF_INET;   /* Internet addr family */
	servAddr.sin_addr.s_addr = inet_addr(servIP);   /* Server IP address */
	servAddr.sin_port = htons(PORT);  /* Server port */

	/*Get the file the user wants*/
	printf("Enter the file you want: ");
	fgets(fileName,RCVBUFSIZE,stdin);
	send.count = strlen(fileName);
	send.seq_num = 0;/*The first seq num will be 0*/
	strcpy(send.data,fileName);
	
	/*send the file name to the server*/
	if (sendto(sock, &send, sizeof(Frame), MSG_CONFIRM, (struct sockaddr *)&servAddr, sizeof(servAddr))!=sizeof(Frame))
		DieWithError("send() sent a different number of bytes than expected");

	/*Receive a response*/
	totalBytesReceived=0;
	n=1;
	f=fopen("out.txt","w");/*Open the file to write into*/
	if(f==NULL){
		DieWithError("File does not exist");
	}

	/*While receiving don't stop looping*/
	/*Recieve data until all data is recieved or error*/
	bytesReceived = recvfrom(sock, &recv, sizeof(Frame), MSG_WAITALL, (struct sockaddr *) &fromAddr, &fromSize);
	recv.count = strlen(recv.data);/*How many bytes the data is*/
	while(bytesReceived > 0)
	{
		if(recv.count == 0){
			break;
		}

		printf("Packet %d recieved with %d data bytes\n",recv.seq_num,recv.count);
		num_pkts++;/*Increase the number of packets seen*/

		if(duplicate == recv.seq_num){
			duplicates++;/*Increase the number of duplicate packets found*/
			printf("Duplicate packet %d received with %d data bytes\n",recv.seq_num,recv.count);
		}
		else{
			totalBytesReceived += recv.count;/*Add amount of bytes received to total count*/
			/*Seq num should be 1 first time*/
			recv.data[recv.count] = '\0';/*Including new line*/
			fputs(recv.data,f);/*Put the data into the file*/
			printf("Packet %d delivered to user\n",recv.seq_num);
			bzero(recv.data,PACKETBUFMAX);/*Zero out buffer so that more data can be received*/
		}

		duplicate = recv.seq_num;/*Set duplicate the pkt_seq just seen*/
		ack_pkt = recv.seq_num;/*Ack the seq_num just seen*/
		printf("ACK %d generated for transmission\n",ack_pkt);
		lost = simulateAckLoss(ack_loss_ratio);/*Check if there will be a loss*/
		if(!lost){/*No loss*/	
			if(sendto(sock, &ack_pkt, sizeof(int), MSG_CONFIRM, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0){
				DieWithError("sendto() failed");
			}
			else{
				printf("ACK %d successfully transmitted\n\n",ack_pkt);
				num_acks_no_loss++;/*Increase the number of acks*/
			}
		}
		else{/*Loss occured*/
			printf("ACK %d lost\n",ack_pkt);
			num_lost++;/*Increase the number of acks lost*/
		}
		bytesReceived = recvfrom(sock, &recv, sizeof(Frame), MSG_WAITALL, (struct sockaddr *) &fromAddr, &fromSize);
		recv.count = strlen(recv.data);
	}

	printf("End of Transmission packet with sequence number %d received\n",recv.seq_num);
	fclose(f);
	close(sock);

	printf("Total number of data packets received successfully: %d\nNumber of duplicate data packets received: %d\nNumber of data packets received successfully, not including duplicates: %d\nTotal number of data bytes received which are delivered to user: %d\nNumber of ACKs transmitted without loss: %d\nNumber of ACKs generated but dropped due to loss: %d\nTotal number of ACKs generated: %d",num_pkts+duplicates,duplicates,num_pkts,totalBytesReceived,num_acks_no_loss,num_lost,num_acks_no_loss+num_lost);

	exit(0);

}
