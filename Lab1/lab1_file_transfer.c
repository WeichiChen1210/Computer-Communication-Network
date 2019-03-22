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

    long int total = 0;
    int percent = 0;
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

    int numbytes = 0;
    int five = 0;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    printf("Progress: %d%%\t%d/%d/%d %02d:%02d:%02d\n", (int)percent, timeinfo->tm_year+1900, timeinfo->tm_mon, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
    five += 5;
    while(!feof(fp)){
        numbytes = fread(buffer, sizeof(char), sizeof(buffer), fp);
		numbytes = write(newsockfd, buffer, numbytes);
        total += numbytes;
        percent =(int)((float)total / (float)filestat.st_size * 100);
        //percent =100 * total / filestat.st_size;
        if(percent == five){
            time ( &rawtime );
            timeinfo = localtime ( &rawtime );
            // printf ( "The current date/time is: %s", asctime (timeinfo) );
            printf("Progress: %d%%\t%d/%d/%d %02d:%02d:%02d\n", (int)percent, timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
            five += 5;
        }
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
        //printf("read %d bytes, ", numbytes);
		if(numbytes == 0)   break;
		numbytes = fwrite(buffer, sizeof(char), numbytes, fp);
		//printf("fwrite %d bytesn\n", numbytes);
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
    servaddr.sin_port = htons(5188);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        ERR_EXIT("bind error");

    char recvbuf[1024] = {0};
    struct sockaddr_in peeraddr;
    socklen_t peerlen;
    int n;

    while (1)
    {
        peerlen = sizeof(peeraddr);
        memset(recvbuf, 0, sizeof(recvbuf));
        n = recvfrom(sock, recvbuf, sizeof(recvbuf), 0, (struct sockaddr *)&peeraddr, &peerlen);
        if (n == -1)
        {
            if (errno == EINTR)
                continue;

            ERR_EXIT("recvfrom error");
        }
        else if(n > 0)
        {
            fputs(recvbuf, stdout);
            sendto(sock, recvbuf, n, 0, (struct sockaddr *)&peeraddr, peerlen);
        }
    }
    close(sock);
}

void UDP_client(char ip[20], int portno, char path[100]){
    int sock;
    if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
        ERR_EXIT("socket");
        struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(5188);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    int ret;
    char sendbuf[1024] = {0};
    char recvbuf[1024] = {0};
    while (fgets(sendbuf, sizeof(sendbuf), stdin) != NULL)
    {
        sendto(sock, sendbuf, strlen(sendbuf), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));

        ret = recvfrom(sock, recvbuf, sizeof(recvbuf), 0, NULL, NULL);
        if (ret == -1)
        {
            if (errno == EINTR)
                continue;
            ERR_EXIT("recvfrom");
        }

        fputs(recvbuf, stdout);
        memset(sendbuf, 0, sizeof(sendbuf));
        memset(recvbuf, 0, sizeof(recvbuf));
    }

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