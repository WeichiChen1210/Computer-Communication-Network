/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <arpa/inet.h>
#include <sys/stat.h>

#define BUFFERSIZE 1024

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void TCP_server(char ip[20], int portno, char path[100]){
    int sockfd, newsockfd;
    socklen_t clilen;
    char buffer[BUFFERSIZE];
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    char message[BUFFERSIZE];
    struct stat filestat;

    // if (argc < 2) {
    //     fprintf(stderr,"ERROR, no port provided\n");
    //     exit(1);
    // }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
       error("ERROR opening socket");
     
    // bzero((char *) &serv_addr, sizeof(serv_addr));
    // portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    inet_aton(ip, &serv_addr.sin_addr);
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    listen(sockfd,5);
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0)
        error("ERROR on accept");
    bzero(buffer,BUFFERSIZE);

    if (lstat(path, &filestat) < 0){
		exit(1);
	}
    FILE *fp = fopen(path, "rb");
	printf("The file size is %lu bytes\n", filestat.st_size);
    if(fp == NULL){
        printf("Cannot open this file.\n");
        return;
    }

    int numbytes = 0;
    while(!feof(fp)){
        numbytes = fread(buffer, sizeof(char), sizeof(buffer), fp);
        printf("fread %d bytes, ", numbytes);
		numbytes = write(newsockfd, buffer, numbytes);
		printf("Sending %d bytesn\n",numbytes);
    }

    // n = read(newsockfd,buffer,BUFFERSIZE-1);
    // if (n < 0) error("ERROR reading from socket");
    // printf("Here is the message: %s\n",buffer);
    
    fclose(fp);
    close(newsockfd);
    close(sockfd);

    return;
}

void TCP_client(char ip[20], int portno, char path[100]){

    int sockfd, n, numbytes;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[BUFFERSIZE];
    char message[BUFFERSIZE];
    FILE *fp;
    // if (argc < 3) {
    //    fprintf(stderr,"usage %s hostname port\n", argv[0]);
    //    exit(0);
    // }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    // server = gethostbyname(argv[1]);
    // if (server == NULL) {
    //     fprintf(stderr,"ERROR, no such host\n");
    //     exit(0);
    // }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    // bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    inet_aton(ip, &serv_addr.sin_addr);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

    
    if((fp = fopen("../gsm-gprs.pptx", "wb")) == NULL){
        perror("connect");
		exit(1);
    }
    while(1){
        numbytes = read(sockfd, buffer, sizeof(buffer));
        printf("read %d bytes, ", numbytes);
		if(numbytes == 0){
			break;
		}
		numbytes = fwrite(buffer, sizeof(char), numbytes, fp);
		printf("fwrite %d bytesn\n", numbytes);
    }
    printf("\n");
    // printf("Please enter the message: ");
    // bzero(buffer,BUFFERSIZE);
    // fgets(buffer,BUFFERSIZE,stdin);
    
    // n = write(sockfd,buffer,strlen(buffer));
    // if (n < 0) 
    //      error("ERROR writing to socket");
    
    
    fclose(fp);
    close(sockfd);
    return;
}

int main(int argc, char *argv[])
{
    char ip[20], path[100];
    int protocol, action, portno;

    if(!strcmp(argv[1], "tcp"))
        protocol = 0;
    else if(!strcmp(argv[1], "udp"))
        protocol = 1;
    if(!strcmp(argv[2], "send"))
        action = 0;
    else if(!strcmp(argv[2], "recv"))
        action = 1;
    strcpy(ip, argv[3]);
    sscanf(argv[4], "%d", &portno);
    if(argc == 6)
        strcpy(path, argv[5]);

    if(protocol == 0 && action == 0)
        TCP_server(ip, portno, path);
    if(protocol == 0 && action == 1)
        TCP_client(ip, portno, path);
    
     return 0;
}
