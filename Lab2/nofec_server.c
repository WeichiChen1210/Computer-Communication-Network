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

int mode;	//0 uni 1 multi
struct in_addr localInterface;
struct sockaddr_in groupSock;
int sd;
char group_ip[20] = "226.1.1.1";
char local_ip[20] = "192.168.1.100";
int group_port = 5000;
struct stat filestat;
char filename[200] = "picture.jpg";
unsigned char filebuf[BUFSIZE];
int numbytes;
int seqnum = 0, count = 0;

int main (int argc, char *argv[ ])
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
	
	printf("[Multicast server without FEC: Prepare to send file %s]\n", filename);
	/////////////// socket ///////////////
	/* Create a datagram socket on which to send.
	 *			IPV4,    UDP
	 */
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sd < 0)
	{
		perror("Opening datagram socket error");
		exit(1);
	}
	// else	printf("Opening the datagram socket...OK.\n");
	
	/* Initialize the group sockaddr structure with a */
	/* group address of 226.1.1.1 and port 4321. */
	memset((char *) &groupSock, 0, sizeof(groupSock));
	groupSock.sin_family = AF_INET;
	groupSock.sin_addr.s_addr = inet_addr(group_ip);
	groupSock.sin_port = htons(group_port);
			
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
	// else	printf("Setting the local interface...OK\n");

	/////////////// file operation ///////////////
	/* get file size */
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
	count = seqnum = 0;
	while(!feof(fp)){
		/* set sequence number to the first 4 bytes in buffer 
		 * each has 1 byte of seqnum, 8 bits
		*/
		filebuf[0] = seqnum >> 24;
		filebuf[1] = seqnum >> 16;
		filebuf[2] = seqnum >> 8;
		filebuf[3] = seqnum;
		/* read file data and store from the 5th byte for size-4 */
		numbytes = fread(filebuf+4, sizeof(unsigned char), sizeof(filebuf)-4, fp);
		
		/* send the actual bytes */
		numbytes = sendto(sd, filebuf, numbytes+4, 0, (struct sockaddr*)&groupSock, sizeof(groupSock));
		if(numbytes < 0)	perror("Sending datagram message error");

		/*update counter and seqnum*/
		count++;
		seqnum++;
	}
	fclose(fp);

	/* send "finish" message */
	sprintf(filebuf, "finish");
	sendto(sd, filebuf, numbytes, 0, (struct sockaddr*)&groupSock, sizeof(groupSock));

	printf("OK.\nSend %d packets.\n", count);
	close(sd);
	return 0;
}