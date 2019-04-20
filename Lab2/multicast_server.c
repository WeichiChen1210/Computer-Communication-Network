/* Send Multicast Datagram code example. */
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

struct in_addr localInterface;
struct sockaddr_in groupSock;
int sd;
char group_ip[20] = "226.1.1.1";
char local_ip[20] = "192.168.1.100";
int group_port = 4321;
char databuf[1024] = "Multicast test message.";
int datalen = sizeof(databuf);
struct stat filestat;
char filename[20] = "1GB.bin";
char filebuf[1024];
int numbytes;

int main (int argc, char *argv[ ])
{
	/* Create a datagram socket on which to send. */
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sd < 0)
	{
		perror("Opening datagram socket error");
		exit(1);
	}
	else	printf("Opening the datagram socket...OK.\n");
	
	/* Initialize the group sockaddr structure with a */
	/* group address of 226.1.1.1 and port 4321. */
	memset((char *) &groupSock, 0, sizeof(groupSock));
	groupSock.sin_family = AF_INET;
	groupSock.sin_addr.s_addr = inet_addr(group_ip);
	groupSock.sin_port = htons(group_port);
		
	// Disable loopback so you do not receive your own datagrams.
	// char loopch = 0;
	// if(setsockopt(sd, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&loopch, sizeof(loopch)) < 0)
	// {
	// perror("Setting IP_MULTICAST_LOOP error");
	// close(sd);
	// exit(1);
	// }
	// else
	// printf("Disabling the loopback...OK.\n");
			
	/* Set local interface for outbound multicast datagrams. */
	/* The IP address specified must be associated with a local, */
	/* multicast capable interface. */
	localInterface.s_addr = inet_addr(local_ip);

	// Definition: int setsockopt(int socket, int level, int optname, void *optval, int optlen);
	// IPPROTO_IP: multicast is ip level and UDP, so use IPPROTO
	// IP_MULTICAST_IF: set to send message to specific socket (localInterface)
	if(setsockopt(sd, IPPROTO_IP, IP_MULTICAST_IF, (char *)&localInterface, sizeof(localInterface)) < 0)
	{
		perror("Setting local interface error");
		exit(1);
	}
	else	printf("Setting the local interface...OK\n");

	if (lstat(filename, &filestat) < 0){
		printf("Cannot open this file.\n");
		exit(1);
	}
	
	FILE *fp = fopen(filename, "rb");
	if(fp == NULL){
        printf("Cannot open this file.\n");
        exit(1);
    }
	int count = 0;
	while(!feof(fp)){
		numbytes = fread(filebuf, sizeof(char), sizeof(filebuf), fp);
		if(numbytes == 0)	break;
		numbytes = sendto(sd, filebuf, numbytes, 0, (struct sockaddr*)&groupSock, sizeof(groupSock));
		count++;
		if(numbytes < 0)
		{
			perror("Sending datagram message error");
		}
	}
	printf("Send %d times.\n", count);
	/* Send a message to the multicast group specified by the*/
	/* groupSock sockaddr structure. */
	/*int datalen = 1024;*/
	sprintf(databuf, "finish");
	if(sendto(sd, databuf, datalen, 0, (struct sockaddr*)&groupSock, sizeof(groupSock)) < 0)
	{
		perror("Sending datagram message error");
	}
	else	printf("Sending finish message...OK\n");
	
	// Try the re-read from the socket if the loopback is not disable
	// if(read(sd, databuf, datalen) < 0)
	// {
	// 	perror("Reading datagram message error\n");
	// 	close(sd);
	// 	exit(1);
	// }
	// else
	// {
	// 	printf("Reading datagram message from client...OK\n");
	// 	printf("The message is: %s\n", databuf);
	// }
	fclose(fp);
	printf("File size: %ld bytes\n", filestat.st_size);
	return 0;
}