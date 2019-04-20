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

struct sockaddr_in localSock;
struct ip_mreq group;
int sd;
char group_ip[20] = "226.1.1.1";
char local_ip[20] = "192.168.1.100";
int group_port = 4321;
int datalen;
char databuf[1024];
struct stat filestat;
char filename[20] = "output.bin";
char filebuf[1024];
int numbytes;
 
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

	printf("Reading datagram message...OK.\n");
	int count = 0;
	/* Read from the socket. */
	datalen = sizeof(databuf);
	while(1){
		numbytes = read(sd, databuf, datalen);
        if(!strncmp(databuf, "finish", numbytes) || numbytes == 0)   break;  // if finish, break
		if(numbytes > 0) {
			numbytes = fwrite(databuf, sizeof(char), numbytes, fp);
			count++;
		}
	}
	printf("Receive %d times.\n", count);
	fclose(fp);
	if (lstat(filename, &filestat) < 0){
		exit(1);
	}
	printf("file size %ld bytes\n", filestat.st_size);
	return 0;
}
