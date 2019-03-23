/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
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

#define BUFFERSIZE 1024
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

void TCP_server(char ip[20], int portno, char path[100]){
    int sockfd, newsockfd;
    socklen_t clilen;
    char buffer[BUFFERSIZE];
    char message[BUFFERSIZE];
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    struct stat filestat;
    time_t rawtime;
    struct tm * timeinfo;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
       error("ERROR opening socket");
     
    // bzero((char *) &serv_addr, sizeof(serv_addr));
    // portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    // serv_addr.sin_addr.s_addr = INADDR_ANY;
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
	printf("The file size is %lu bytes\n", filestat.st_size);
    
    FILE *fp = fopen(path, "rb");
    if(fp == NULL){
        printf("Cannot open this file.\n");
        return;
    }

    sprintf(message, "Sending file: %s", path);
    n = write(newsockfd, message, sizeof(message));
    if (n < 0) error("ERROR writing to socket");
    strcpy(message, "");
    n = read(newsockfd, message, sizeof(message));
    if (n < 0 || strcmp(message, "OK")) error("ERROR reading from socket");
    // printf("Here is the message: %s\n", message);
    strcpy(message, "");    

    long int total = 0;
    int percent = 0;
    int numbytes = 0;
    bool first = true;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    printf("Progress: %d%%\t%d/%d/%d %02d:%02d:%02d\n", percent, timeinfo->tm_year+1900, timeinfo->tm_mon, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
    
    while(!feof(fp)){
        numbytes = fread(buffer, sizeof(char), sizeof(buffer), fp);
		numbytes = write(newsockfd, buffer, numbytes);
        total += numbytes;
        percent =(int)((float)total / (float)filestat.st_size * 100);
        // printf("percent %d\n", percent);
        //percent =100 * total / filestat.st_size;
        if(percent/5 > 0 && percent%5 == 0){
            if(first){
                time ( &rawtime );
                timeinfo = localtime ( &rawtime );
                printf("Progress: %d%%\t%d/%d/%d %02d:%02d:%02d\n", (int)percent, timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
                first = false;
            }            
        }
        else first = true;
    }
    
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
    char filename[BUFFERSIZE];
    FILE *fp;
    struct stat filestat;

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

    n = read(sockfd, message, sizeof(message));
    if (n < 0) error("ERROR reading from socket");
    printf("%s\n", message);
    sscanf(message, "Sending file: %s", filename);
    // printf("%s\n", filename);
    strcpy(message, "OK");

    n = write(sockfd, message, sizeof(message));
    if (n < 0) 
         error("ERROR writing to socket");
    strcpy(message, "");

    if((fp = fopen(filename, "wb")) == NULL){
        perror("connect");
		exit(1);
    }
    while(1){
        numbytes = read(sockfd, buffer, sizeof(buffer));
        printf("read %d bytes, ", numbytes);
		if(numbytes == 0)   break;
		numbytes = fwrite(buffer, sizeof(char), numbytes, fp);
		printf("fwrite %d bytes\n", numbytes);
    }
        
    fclose(fp);
    if (lstat(filename, &filestat) < 0){
		exit(1);
	}
	printf("Received file size is %lu bytes\n", filestat.st_size);
    close(sockfd);
    return;
}

void UDP_server(char ip[20], int portno, char path[100]){
    int sock;
    if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
        ERR_EXIT("socket error");

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(portno);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        ERR_EXIT("bind error");

    char recvbuf[BUFFERSIZE] = {0};
    char sendbuf[BUFFERSIZE] = {0};
    char buffer[BUFFERSIZE];
    struct sockaddr_in client_addr;
    socklen_t peerlen;
    long int numbytes;
    struct stat filestat;

    peerlen = sizeof(client_addr);
    memset(recvbuf, 0, sizeof(recvbuf));
    memset(sendbuf, 0, sizeof(sendbuf));
    memset(buffer, 0, sizeof(buffer));
    numbytes = recvfrom(sock, recvbuf, sizeof(recvbuf), 0, (struct sockaddr *)&client_addr, &peerlen);
    if (numbytes == -1){
        if (errno == EINTR) /*continue*/;
        ERR_EXIT("recvfrom error");
    }
    else if(!strcmp(recvbuf, "Request"))
    {
        printf("Received: %s\n", recvbuf);
        sprintf(sendbuf, "OK %s", path);
        printf("Send: %s\nStart sending file...\n", sendbuf);
        sendto(sock, sendbuf, sizeof(sendbuf), 0, (struct sockaddr *)&client_addr, peerlen);
    }

    if (lstat(path, &filestat) < 0){
		exit(1);
	}
	printf("The file size is %lu bytes\n", filestat.st_size);
    
    FILE *fp = fopen(path, "rb");
    if(fp == NULL){
        printf("Cannot open this file.\n");
        return;
    }

    long int total = 0;
    int percent = 0;
    bool first = true;
    while(!feof(fp)){
        numbytes = fread(buffer, sizeof(char), sizeof(buffer), fp);
		numbytes = sendto(sock, buffer, numbytes, 0, (struct sockaddr *)&client_addr, peerlen);
        total += numbytes;
        percent =(int)((float)total / (float)filestat.st_size * 100);
        // printf("percent %d\n", percent);
        //percent =100 * total / filestat.st_size;
        if(percent/5 > 0 && percent%5 == 0){
            if(first){
                // time ( &rawtime );
                // timeinfo = localtime ( &rawtime );
                // printf("Progress: %d%%\t%d/%d/%d %02d:%02d:%02d\n", percent, timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
                printf("Progress: %d%%\n", percent);
                first = false;
            }            
        }
        else first = true;
    }
    strcpy(buffer, "fin");
    sendto(sock, buffer, numbytes, 0, (struct sockaddr *)&client_addr, peerlen);

    close(sock);
}

void UDP_client(char ip[20], int portno, char path[100]){
    int sock, numbytes;
    char filename[BUFFERSIZE];
    char buffer[BUFFERSIZE];
    FILE *fp;
    struct stat filestat;

    if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
        ERR_EXIT("socket");
    struct sockaddr_in servaddr;
    struct sockaddr_in src;
    socklen_t len;

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(portno);
    servaddr.sin_addr.s_addr = inet_addr(ip);
    len = sizeof(src);

    long int ret;
    char sendbuf[BUFFERSIZE] = {0};
    char recvbuf[BUFFERSIZE] = {0};
    sprintf(sendbuf, "Request");
    printf("Send: %s\n", sendbuf);
    sendto(sock, sendbuf, sizeof(sendbuf), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
    ret = recvfrom(sock, recvbuf, sizeof(recvbuf), 0, (struct sockaddr *)&src, &len);
    if (ret == -1){
        if (errno == EINTR);
        ERR_EXIT("recvfrom");
    }
    else if(ret > 0){
        sscanf(recvbuf, "OK %s", filename);
        printf("Received: %s\nStart receiving file...\n", recvbuf);
        //printf("%s\n", filename);
    }

    if((fp = fopen(filename, "wb")) == NULL){
        perror("connect");
		exit(1);
    }
    int count = 0;
    while(1){
        printf("count: %d\n", count);
        numbytes = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&src, &len);
        printf("read %d bytes, ", numbytes);
		if(numbytes == 0)   break;
		numbytes = fwrite(buffer, sizeof(char), numbytes, fp);
		printf("fwrite %d bytes\n", numbytes);
        count++;
    }
        
    fclose(fp);
    if (lstat(filename, &filestat) < 0){
		exit(1);
	}
	printf("Received file size is %lu bytes\n", filestat.st_size);

    close(sock);
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
    if(protocol == 1 && action == 0)
        UDP_server(ip, portno, path);
    if(protocol == 1 && action == 1)
        UDP_client(ip, portno, path);
    
     return 0;
}