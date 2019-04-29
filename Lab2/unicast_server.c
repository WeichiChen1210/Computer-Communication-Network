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
    char ip[20], path[100];
    int portno;

    /* getting ip, port, file path */
    strcpy(ip, argv[1]);
    sscanf(argv[2], "%d", &portno);
    strcpy(path, argv[3]);

    int sock;
    struct sockaddr_in servaddr;
    char recvbuf[BUFFERSIZE] = {0};
    char buffer[BUFFERSIZE];
    struct sockaddr_in client_addr;
    socklen_t peerlen;
    long int numbytes;
    struct stat filestat;

    /* create udp socket(ipv4, udp, protocol) */
    if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)    ERR_EXIT("socket error");

    /* setting server address */
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(portno);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    /* bind to the socket */
    if (bind(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) ERR_EXIT("bind error");
    

    peerlen = sizeof(client_addr);
    memset(recvbuf, 0, sizeof(recvbuf));
    memset(buffer, 0, sizeof(buffer));

    /* receive request from client */    
    numbytes = recvfrom(sock, recvbuf, sizeof(recvbuf), 0, (struct sockaddr *)&client_addr, &peerlen);
    if (numbytes == -1){
        if (errno == EINTR) /*continue*/;
        ERR_EXIT("recvfrom error");
    }
    else if(!strcmp(recvbuf, "Request"))
    {
        printf("Received: %s\n", recvbuf);
        printf("Send: %s\nStart sending file...\n", path);      
    }

    memset(recvbuf, 0, sizeof(recvbuf));

    /* prepare file to read and get info */
    if (lstat(path, &filestat) < 0){
		exit(1);
	}
	printf("The file size is %lu bytes\n", filestat.st_size);
    FILE *fp = fopen(path, "rb");
    if(fp == NULL){
        printf("Cannot open this file.\n");
        exit(1);
    }
    
    int seqnum = 0;
    /* send file to client */
    while(!feof(fp)){
        /* set sequence number to the first 4 bytes in buffer 
		 * each has 1 byte of seqnum, 8 bits
		 */
		buffer[0] = seqnum >> 24;
		buffer[1] = seqnum >> 16;
		buffer[2] = seqnum >> 8;
		buffer[3] = seqnum;
        numbytes = fread(buffer+4, sizeof(char), sizeof(buffer)-4, fp);
        
        if(numbytes == 0)   break;
		
        numbytes = sendto(sock, buffer, numbytes+4, 0, (struct sockaddr *)&client_addr, peerlen);     // send data
        /* increase seq number */
        seqnum++;
    }
    printf("Send %d packets\n", seqnum);
    /* send finish message to client */
    sprintf(recvbuf, "finish");
    sendto(sock, recvbuf, sizeof(recvbuf), 0, (struct sockaddr *)&client_addr, peerlen);
    printf("Sending completed.\n");
    
    fclose(fp);
    close(sock);
}