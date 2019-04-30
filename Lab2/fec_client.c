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

struct sockaddr_in localSock;
struct ip_mreq group;
int sd;
char group_ip[20] = "226.1.1.1";
char local_ip[20] = "192.168.1.100";
int group_port = 4321;

char databuf[BUFSIZE];
struct stat filestat;
char filename[200] = "output.jpg";
int numbytes;
int recvcount = 0, count = 0, lost = 0, seqnum = 0;
 
int main(int argc, char *argv[])
{
	if(argc == 4){
		strcpy(local_ip, argv[1]);
		sscanf(argv[2], "%d", &group_port);
		strcpy(filename, argv[3]);
	}
	else{
		printf("wrong arguments\n");
		exit(1);
	}
	// printf("%s %d %s\n", local_ip, group_port, filename);

	printf("[Multicast client with FEC: Prepare to receive file]\n");
	/////////////// fec setup ///////////////
	// simulation parameters
    unsigned int n = BUFSIZE;               // original data length (bytes)
    fec_scheme fs = LIQUID_FEC_HAMMING74;   // error-correcting scheme

    // compute size of encoded message
    unsigned int k = fec_get_enc_msg_length(fs,n);

    // create arrays
    unsigned char msg_enc[k];   // encoded/received data message
    unsigned char msg_dec[n];   // decoded data message

    // CREATE the fec object
    fec q = fec_create(fs,NULL);
    // fec_print(q);

    unsigned int i;
	/////////////// socket setup ///////////////
	/* Create a datagram socket on which to receive. */
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sd < 0)
	{
		perror("Opening datagram socket error");
		exit(1);
	}
	// else	printf("Opening datagram socket....OK.\n");
		 
	/* Enable SO_REUSEADDR to allow multiple instances of this
	 * application to receive copies of the multicast datagrams. 
	 */
	int reuse = 1;
	if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0)
	{
		perror("Setting SO_REUSEADDR error");
		close(sd);
		exit(1);
	}
	// else	printf("Setting SO_REUSEADDR...OK.\n");
	 
	/* Bind to the proper port number with the IP address
	 * specified as INADDR_ANY. 
	 */
	memset((char *) &localSock, 0, sizeof(localSock));
	localSock.sin_family = AF_INET;				// IPV4
	localSock.sin_port = htons(group_port);
	localSock.sin_addr.s_addr = INADDR_ANY;
	if(bind(sd, (struct sockaddr*)&localSock, sizeof(localSock)))
	{
		perror("Binding datagram socket error");
		close(sd);
		exit(1);
	}
	// else	printf("Binding datagram socket...OK.\n");
	 
	/* Join the multicast group 226.1.1.1 on the local ip
	 * interface. Note that this IP_ADD_MEMBERSHIP option must be
	 * called for each local interface over which the multicast
	 * datagrams are to be received. 
	 */
	group.imr_multiaddr.s_addr = inet_addr(group_ip);
	group.imr_interface.s_addr = inet_addr(local_ip);
	if(setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(group)) < 0)
	{
		perror("Adding multicast group error");
		close(sd);
		exit(1);
	}
	// else	printf("Adding multicast group...OK.\n");
	
	/////////////// file operation ///////////////
	/* open file to be written */
	FILE *fp;
	if((fp = fopen(filename, "wb")) == NULL){
		printf("Cannot write this file.\n");
        exit(1);
	}

	/////////////// file receiving ///////////////
	printf("Reading datagram message...");
	seqnum = 0;
	int actual_byte;
	/* Read from the socket. */
	while((numbytes = read(sd, msg_enc, sizeof(msg_enc))) > 0){
		/* decoding, get decoded buffer msg_dec */
		fec_decode(q, n, msg_enc, msg_dec);

		numbytes = sizeof(msg_dec);
		/* compare the message, if is "finish" break the loop */
		if(!strncmp(msg_dec, "finish", numbytes)){
			fclose(fp);
			break;
		}
		/* else, write file with msg_dec from the 5th byte and for size-4 legnth */
		actual_byte = msg_dec[4];
		if(numbytes > 0) {
			numbytes = fwrite(msg_dec+5, sizeof(unsigned char), actual_byte, fp);
		}
		/* compute sequence number */
		seqnum = msg_dec[0] << 24;
		seqnum += msg_dec[1] << 16;
		seqnum += msg_dec[2] << 8;
		seqnum += msg_dec[3];
		/* compare the received seqnum and own counter,
		 * if not the same, there are packets lost,
		 * number of lost packets is seqnum-count, add to variable lost
		 * and the update the counter to seqnum+1
		 */
		if(seqnum > count){
			lost += abs(seqnum-count);
			count = seqnum+1;
		}
		/* else just counter++ */
		else count++;
		/* actual number of received packets */
		recvcount++;
	}
	// DESTROY the fec object
    fec_destroy(q);
	
	printf("OK.\nReceived %d packets, lost %d.\n", recvcount, lost);

	/* Recieved file size */
	if (lstat(filename, &filestat) < 0){
		exit(1);
	}
	printf("The file size is %ld bytes\n", filestat.st_size);

	/* compute lost rate = number of lost packets/(number of lost packets+received packets) */
	printf("Lost Rate: %.2f%%\n", 100*(float)lost/(float)(lost+recvcount));

	return 0;
}