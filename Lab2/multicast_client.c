/* Receiver/client multicast Datagram example. */
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
#define BUFSIZE 128

struct sockaddr_in localSock;
struct ip_mreq group;
int sd;
char group_ip[20] = "226.1.1.1";
char local_ip[20] = "192.168.1.100";
int group_port = 4321;
int datalen;
char databuf[BUFSIZE];
struct stat filestat;
char filename[20] = "output.jpg";
char filebuf[BUFSIZE];
int numbytes;
int seqnum = 0;
int arr[10];
 
int main(int argc, char *argv[])
{
/* Create a datagram socket on which to receive. */
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sd < 0)
	{
		perror("Opening datagram socket error");
		exit(1);
	}
	else	printf("Opening datagram socket....OK.\n");
		 
	/* Enable SO_REUSEADDR to allow multiple instances of this */
	/* application to receive copies of the multicast datagrams. */
	{
		int reuse = 1;
	if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0)
	{
		perror("Setting SO_REUSEADDR error");
		close(sd);
		exit(1);
	}
	else	printf("Setting SO_REUSEADDR...OK.\n");
	}
	 
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
	else	printf("Binding datagram socket...OK.\n");
	 
	/* Join the multicast group 226.1.1.1 on the local 203.106.93.94 */
	/* interface. Note that this IP_ADD_MEMBERSHIP option must be */
	/* called for each local interface over which the multicast */
	/* datagrams are to be received. */
	group.imr_multiaddr.s_addr = inet_addr(group_ip);
	group.imr_interface.s_addr = inet_addr(local_ip);
	if(setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(group)) < 0)
	{
		perror("Adding multicast group error");
		close(sd);
		exit(1);
	}
	else	printf("Adding multicast group...OK.\n");
	
	FILE *fp;
	if((fp = fopen(filename, "wb")) == NULL){
		printf("Cannot write this file.\n");
        exit(1);
	}

	// printf("Reading datagram message...OK.\n");
	int i = 0, j = 0;
	for(i = 0; i < 10; i++)
			arr[i] = 0;

	int count = 0;
	seqnum = 0;
	/* Read from the socket. */
	datalen = sizeof(databuf);
	while(1){
		numbytes = read(sd, databuf, datalen);
        if(!strncmp(databuf, "finish", numbytes) || numbytes == 0)   break;  // if finish, break
		seqnum = databuf[numbytes-1] - '0';
		// printf("recv seqnum %d\n", seqnum);
		if(numbytes > 0) {
			numbytes = fwrite(databuf, sizeof(char), numbytes-1, fp);
			arr[seqnum]++;
			count++;
			// printf("count %d\n", count);
		}
	}
	printf("Last seqnum %d\nSeq array:\n", seqnum);
	for(i = 0; i < 10; i++)
		printf("%d ", arr[i]);
	printf("\nReceived %d packets.\n", count);
	fclose(fp);
	if (lstat(filename, &filestat) < 0){
		exit(1);
	}
	printf("The file size is %ld bytes\n", filestat.st_size);
	return 0;
}
