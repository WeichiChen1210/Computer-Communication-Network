#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <liquid/liquid.h>
#define BUFSIZE 256

struct in_addr localInterface;
struct sockaddr_in groupSock;
int sd;
char group_ip[20] = "226.1.1.1";
char local_ip[20] = "192.168.208.193";
int group_port = 4321;
struct stat filestat;
char filename[200] = "picture.jpg";
int numbytes;
int seqnum = 0, count = 0;

int main (int argc, char *argv[])
{
	if(argc > 1)	strcpy(filename, argv[1]);
	/////////////// fec setup ///////////////
	// simulation parameters
    unsigned int n = BUFSIZE;               // original data length (bytes)
    fec_scheme fs = LIQUID_FEC_HAMMING74;   // error-correcting scheme

    // compute size of encoded message
    unsigned int k = fec_get_enc_msg_length(fs,n);

    // create arrays
    unsigned char msg_org[n];   // original data message
    unsigned char msg_enc[k];   // encoded/received data message

    // CREATE the fec object
    fec q = fec_create(fs,NULL);
    fec_print(q);

    unsigned int i;

	/////////////// socket ///////////////
	/* Create a datagram socket on which to send. */
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sd < 0)
	{
		perror("Opening datagram socket error");
		exit(1);
	}
	else	printf("Opening the datagram socket...OK.\n");
	
	/* setting group
	 * Initialize the group sockaddr structure with a
	 * group address of 226.1.1.1 and port 4321.
	 */
	memset((char *) &groupSock, 0, sizeof(groupSock));
	groupSock.sin_family = AF_INET;						/* AF_INET: IPV4 */
	groupSock.sin_addr.s_addr = inet_addr(group_ip);	/* inet_addr(): convert ip address to type in_addr_t */
	groupSock.sin_port = htons(group_port);				/* htons(): convert port to in_port_t */
			
	/* Set local interface for outbound multicast datagrams.
	 * The IP address specified must be associated with a local, 
	 * multicast capable interface.
	 */
	localInterface.s_addr = inet_addr(local_ip);

	/* Definition: int setsockopt(int socket, int level, int optname, void *optval, int optlen);
	 * IPPROTO_IP: multicast is ip level and UDP, so use IPPROTO
	 * IP_MULTICAST_IF: set to send message to specific socket (localInterface)
	 */
	if(setsockopt(sd, IPPROTO_IP, IP_MULTICAST_IF, (char *)&localInterface, sizeof(localInterface)) < 0)
	{
		perror("Setting local interface error");
		exit(1);
	}
	else	printf("Setting the local interface...OK\n");

	/////////////// file operation ///////////////
	/* get file information(size) */
	if (lstat(filename, &filestat) < 0){
		printf("Cannot open this file.\n");
		exit(1);
	}
	printf("The file size is %ld bytes\n", filestat.st_size);
	printf("Sending file...");

	/* open file to be read */
	FILE *fp = fopen(filename, "rb");
	if(fp == NULL){
        printf("Cannot open this file.\n");
        exit(1);
    }

	/////////////// file transfering ///////////////
	seqnum = 0;
	while(!feof(fp)){
		/* set sequence number to the first 4 bytes */
		msg_org[0] = seqnum >> 24;
		msg_org[1] = seqnum >> 16;
		msg_org[2] = seqnum >> 8;
		msg_org[3] = seqnum;
		/* read the file and store in the buffer from the 5th byte and for size-4 */
		numbytes = fread(msg_org+4, sizeof(unsigned char), sizeof(msg_org)-4, fp);

		/* fec encoding, get encoded buffer msg_enc */
		fec_encode(q, n, msg_org, msg_enc);
		
		/* send data */
		numbytes = sendto(sd, msg_enc, sizeof(msg_enc), 0, (struct sockaddr*)&groupSock, sizeof(groupSock));
		/* increase counters */
		seqnum++;
		count++;
		if(numbytes < 0)	perror("Sending datagram message error");
	}
	fclose(fp);

	/////////////// send "finish" message ///////////////
	sprintf(msg_org, "finish");
	fec_encode(q, n, msg_org, msg_enc);
	numbytes = sendto(sd, msg_enc, sizeof(msg_enc), 0, (struct sockaddr*)&groupSock, sizeof(groupSock));

	printf("OK\nSend %d pakcets.\n", count);

	return 0;
}