#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <arpa/inet.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#define BUFFERSIZE 256
#define ERR_EXIT(m) \
    do { \
        perror(m); \
        exit(EXIT_FAILURE); \
    } while (0)

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]){
    char ip[20], filename[100];
    int portno;
    /* getting ip, port, file path */
    strcpy(ip, argv[1]);
    sscanf(argv[2], "%d", &portno);
    strcpy(filename, argv[3]);

    int sock, numbytes;
    struct sockaddr_in servaddr;
    struct sockaddr_in src;
    socklen_t len;
    long int ret;
    char sendbuf[BUFFERSIZE] = {0};
    char recvbuf[BUFFERSIZE] = {0};
    char buffer[BUFFERSIZE];
    FILE *fp;
    struct stat filestat;

    /* create socket */
    if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
        ERR_EXIT("socket");

    /* setting server address */
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(portno);
    servaddr.sin_addr.s_addr = inet_addr(ip);
    len = sizeof(src);

    /* sending request */
    sprintf(sendbuf, "Request");
    printf("Send: %s\n", sendbuf);
    sendto(sock, sendbuf, sizeof(sendbuf), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));

    // prepare file to write
    if((fp = fopen(filename, "wb")) == NULL){
        perror("connect");
		exit(1);
    }
    
    memset(sendbuf, 0, sizeof(sendbuf));
    memset(recvbuf, 0, sizeof(recvbuf));
    int count = 0, seqnum = 0, recvcount = 0, lost = 0;

    /* receive data and wite into file */
    while(1){
        numbytes = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&src, &len);
        if(!strncmp(buffer, "finish", numbytes))   break;  // if finish, break
        /* comppute sequence number from the first 4 bytes */
		seqnum = buffer[0] << 24;
		seqnum += buffer[1] << 16;
		seqnum += buffer[2] << 8;
		seqnum += buffer[3];
        numbytes = fwrite(buffer+4, sizeof(char), numbytes-4, fp);  // write file
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

    fclose(fp);
    printf("OK.\nReceived %d packets, lost %d.\n", recvcount, lost);

	/* get received file size */
	if (lstat(filename, &filestat) < 0){
		exit(1);
	}
	printf("Received file size is %ld bytes\n", filestat.st_size);
	close(sock);

	/* compute lost rate = number of lost packets/(number of lost packets+received packets) */
	printf("Lost Rate: %.2f%%\n", 100*(float)lost/(float)(lost+recvcount));

    close(sock);
}