#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#define BUFSIZE 256

int mode;	// 0 uni 1 multi
struct sockaddr_in localSock;
struct ip_mreq group;
int sd;
char group_ip[20] = "226.1.1.1";
char local_ip[20] = "192.168.1.100";
int group_port = 5000;
unsigned char databuf[BUFSIZE];
struct stat filestat;
char filename[200] = "output.jpg";
int numbytes;

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

	printf("[Multicast client without FEC: Prepare to receive file]\n");
	/////////////// socket setup ///////////////
	/* Create a datagram socket on which to receive. */
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sd < 0)
	{
		perror("Opening datagram socket error");
		exit(1);
	}
	// else	printf("Opening datagram socket....OK.\n");
		 
	/* Enable SO_REUSEADDR to allow multiple instances of this */
	/* application to receive copies of the multicast datagrams. */
	int reuse = 1;
	if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0)
	{
		perror("Setting SO_REUSEADDR error");
		close(sd);
		exit(1);
	}
	// else	printf("Setting SO_REUSEADDR...OK.\n");
		 
	/* Bind to the proper port number with the IP address */
	/* specified as INADDR_ANY. */
	memset((char *) &localSock, 0, sizeof(localSock));
	localSock.sin_family = AF_INET;
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
	FILE *fp;
	if((fp = fopen(filename, "wb")) == NULL){
		printf("Cannot write this file.\n");
        exit(1);
	}

	printf("Reading datagram file...");

	int count = 0, recvcount = 0, seqnum = 0, lost = 0;
	/* Read from the socket */
	while((numbytes = read(sd, databuf, sizeof(databuf))) > 0){
		/* if the message is "finish", end receiving */
		if(!strncmp(databuf, "finish", numbytes)){
			fclose(fp);
			break;
		}
		/* comppute sequence number from the first 4 bytes */
		seqnum = databuf[0] << 24;
		seqnum += databuf[1] << 16;
		seqnum += databuf[2] << 8;
		seqnum += databuf[3];
		/* write files from the 5th byte for size-4*/
		if(numbytes > 0) {
			numbytes = fwrite(databuf+4, sizeof(unsigned char), numbytes-4, fp);
		}

		/* compare seqnum to count, if not the same, then some packets are lost */
		if(seqnum > count){
			/* number of lost packets */
			lost += abs(seqnum-count);
			/* update counter */
			count = seqnum+1;
		}
		/* else update counter */
		else count++;
		/* actual packet received */
		recvcount++;
	}
	
	printf("OK.\nReceived %d packets, lost %d.\n", recvcount, lost);

	/* get received file size */
	if (lstat(filename, &filestat) < 0){
		exit(1);
	}
	printf("Received file size is %ld bytes\n", filestat.st_size);
	close(sd);

	/* compute lost rate = number of lost packets/(number of lost packets+received packets) */
	printf("Lost Rate: %.2f%%\n", 100*(float)lost/(float)(lost+recvcount));
	return 0;
}
