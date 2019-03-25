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
    struct sockaddr_in serv_addr, cli_addr;
    
    int n;
    char buffer[BUFFERSIZE];
    char message[BUFFERSIZE];
    
    struct stat filestat;
    
    long int total = 0;
    int percent = 0, count = 0;
    int numbytes = 0;
    
    time_t rawtime;
    struct tm * timeinfo;

    /* Create a socket(ipv4, TCP, protocol), return sockfd */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");
    
    /* Setting server address */
    serv_addr.sin_family = AF_INET;     // ipv4
    inet_aton(ip, &serv_addr.sin_addr); // convert and store ip 
    serv_addr.sin_port = htons(portno); // convert and store port

    /* bind the address to server socket(socketfd, address info, length of address) */
    if(bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    
    /* listen to the socket(sockfd, backlog(the max num waiting in the queue)) */
    listen(sockfd,5);
    
    /* accept connections(sockfd, client address, length), returns client's socket */
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0)
        error("ERROR on accept");
    bzero(buffer,BUFFERSIZE);

    /* received request from client */
    n = read(newsockfd, message, sizeof(message));
    if (n < 0 || strcmp(message, "Request")) error("ERROR reading from socket");
    strcpy(message, "");

    /* inform for sending files and file name to client */
    sprintf(message, "Sending file: %s", path);
    printf("%s\n", message);
    n = write(newsockfd, message, sizeof(message));
    if (n < 0) error("ERROR writing to socket");
    strcpy(message, "");

    /* read the feedback from client */
    n = read(newsockfd, message, sizeof(message));
    if (n < 0 || strcmp(message, "OK")) error("ERROR reading from socket");
    // printf("Here is the message: %s\n", message);
    strcpy(message, "");    

    /* open the file to read info(file size) */
    if (lstat(path, &filestat) < 0){
		exit(1);
	}
	printf("The file size is %lu bytes.\n", filestat.st_size);
    
    /* open the file for reading */
    FILE *fp = fopen(path, "rb");
    if(fp == NULL){
        printf("Cannot open this file.\n");
        return;
    }

    /* split the file into packets and send to the client */
    while(!feof(fp)){       //while the file is not at the end
        numbytes = fread(buffer, sizeof(char), sizeof(buffer), fp);     // read file and store into buffer, return bytes actually read
		numbytes = write(newsockfd, buffer, numbytes);                  // send buffer to the client
        total += numbytes;                                              // sum the bytes currently sent
        percent =(int)((float)total / (float)filestat.st_size * 100);   // calculate the percentage

        /* if the percent meets 5% or more, print out the progress and the time */
        if(percent == count){
            count += 5;
            time (&rawtime);
            timeinfo = localtime (&rawtime);
            printf("%d%%\t%d/%d/%d %02d:%02d:%02d\n", percent, timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
        }
        if((percent / 5) * 5 > count){            
            int year, month, day, hour, min, sec, temp = count;
            count = (percent / 5) * 5 + 5;
            time (&rawtime);
            timeinfo = localtime (&rawtime);
            year = timeinfo->tm_year + 1900;
            month = timeinfo->tm_mon + 1;
            day = timeinfo->tm_mday;
            hour = timeinfo->tm_hour;
            min = timeinfo->tm_min;
            sec = timeinfo->tm_sec;
            while(temp < count){
                printf("%d%%\t%d/%d/%d %02d:%02d:%02d\n", temp, year, month, day, hour, min, sec);
                temp += 5;
            } 
        }
    }
    printf("Complete transfering.\n");

    sprintf(message, "finish");
    n = write(newsockfd, message, sizeof(message));
    if (n < 0) error("ERROR writing to socket");
    strcpy(message, "");
    
    /* close file and sockets */
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

    /* create socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");

    /* setting server address info */
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    inet_aton(ip, &serv_addr.sin_addr);
    serv_addr.sin_port = htons(portno);

    /* connect to the server(sockfd, server adress, size of address), return 1 when success */
    if (connect(sockfd,(struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)   error("ERROR connecting");

    /* send request to server */
    sprintf(message, "Request");
    n = write(sockfd, message, sizeof(message));
    if (n < 0)  error("ERROR writing to socket");
    memset(message, 0, sizeof(message));

    /* receive respond from server, check the file name */
    n = read(sockfd, message, sizeof(message));
    if (n < 0) error("ERROR reading from socket");
    sscanf(message, "Sending file: %s", filename);
    printf("Receiving file: %s\n", filename);
    
    /*respond ok to server */
    strcpy(message, "OK");
    n = write(sockfd, message, sizeof(message));
    if (n < 0)  error("ERROR writing to socket");
    strcpy(message, "");

    /* open the file to be saved */
    if((fp = fopen(filename, "wb")) == NULL){
        perror("connect");
		exit(1);
    }

    /* start to receive packages and save them */
    while(1){
        numbytes = read(sockfd, buffer, sizeof(buffer));
		if(numbytes == 0 || !strcmp(buffer, "finish"))   break; // if receive 0 byte or message "finish", break
		numbytes = fwrite(buffer, sizeof(char), numbytes, fp);
    }
    printf("Complete transfering. ");

    /* confirm file info */
    fclose(fp);
    if (lstat(filename, &filestat) < 0){
		exit(1);
	}
	printf("File size is %lu bytes\n", filestat.st_size);
    
    close(sockfd);
    return;
}

void UDP_server(char ip[20], int portno, char path[100]){
    int sock;
    struct sockaddr_in servaddr;
    char recvbuf[BUFFERSIZE] = {0};
    char sendbuf[BUFFERSIZE] = {0};
    char buffer[BUFFERSIZE];
    struct sockaddr_in client_addr;
    socklen_t peerlen;
    long int numbytes;
    struct stat filestat;
    long int total = 0;
    int percent = 0, n, count = 0;
    time_t rawtime;
    struct tm * timeinfo;
    struct timeval timeout = {3, 0};    

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
    memset(sendbuf, 0, sizeof(sendbuf));
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
        sprintf(sendbuf, "OK %s", path);
        printf("Send: %s\nStart sending file...\n", sendbuf);
        sendto(sock, sendbuf, sizeof(sendbuf), 0, (struct sockaddr *)&client_addr, peerlen);    // send file name back        
    }

    memset(sendbuf, 0, sizeof(sendbuf));
    memset(recvbuf, 0, sizeof(recvbuf));

    /* prepare file to read and get info */
    if (lstat(path, &filestat) < 0){
		exit(1);
	}
	printf("The file size is %lu bytes\n", filestat.st_size);
    FILE *fp = fopen(path, "rb");
    if(fp == NULL){
        printf("Cannot open this file.\n");
        return;
    }

    /* set timeout if the client didn't send ACK back */
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval));
    
    /* send file to client */
    while(!feof(fp)){
        numbytes = fread(buffer, sizeof(char), sizeof(buffer), fp);
        if(numbytes == 0)   break;
		numbytes = sendto(sock, buffer, numbytes, 0, (struct sockaddr *)&client_addr, peerlen);     // send data
        n = recvfrom(sock, recvbuf, sizeof(recvbuf), 0, (struct sockaddr *)&client_addr, &peerlen); // wait for ACK to continue
        while(strcmp(recvbuf, "ACK") || n == -1){
            printf("Resending...\n");
            numbytes = sendto(sock, buffer, numbytes, 0, (struct sockaddr *)&client_addr, peerlen);     // send data
            n = recvfrom(sock, recvbuf, sizeof(recvbuf), 0, (struct sockaddr *)&client_addr, &peerlen); // wait for ACK to continue
        }

        /* calculate the progress */
        total += numbytes;
        percent =(int)((float)total / (float)filestat.st_size * 100);
        if(percent == count){
            count += 5;
            time(&rawtime);
            timeinfo = localtime (&rawtime);
            printf("%d%%\t%d/%d/%d %02d:%02d:%02d\n", percent, timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
        }
        if((percent / 5) * 5 > count){            
            int year, month, day, hour, min, sec, temp = count;
            count = (percent / 5) * 5 + 5;
            time(&rawtime);
            timeinfo = localtime (&rawtime);
            year = timeinfo->tm_year + 1900;
            month = timeinfo->tm_mon + 1;
            day = timeinfo->tm_mday;
            hour = timeinfo->tm_hour;
            min = timeinfo->tm_min;
            sec = timeinfo->tm_sec;
            while(temp < count){
                printf("%d%%\t%d/%d/%d %02d:%02d:%02d\n", temp, year, month, day, hour, min, sec);
                temp += 5;
            } 
        }
    }

    /* send finish message to client */
    sprintf(sendbuf, "finish");
    sendto(sock, sendbuf, sizeof(sendbuf), 0, (struct sockaddr *)&client_addr, peerlen);
    printf("Sending completed.\n");
    fclose(fp);
    close(sock);
}

void UDP_client(char ip[20], int portno, char path[100]){
    int sock, numbytes;
    struct sockaddr_in servaddr;
    struct sockaddr_in src;
    socklen_t len;
    long int ret;
    char sendbuf[BUFFERSIZE] = {0};
    char recvbuf[BUFFERSIZE] = {0};
    char filename[BUFFERSIZE];
    char buffer[BUFFERSIZE];
    FILE *fp;
    struct stat filestat;
    struct timeval timeout = {5, 0}; 

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
    
    /* receive respond */
    ret = recvfrom(sock, recvbuf, sizeof(recvbuf), 0, (struct sockaddr *)&src, &len);
    if (ret == -1){
        if (errno == EINTR);
        ERR_EXIT("recvfrom");
    }
    else if(ret > 0){
        sscanf(recvbuf, "OK %s", filename);     // get file name
        printf("Received: %s\nStart receiving file...\n", recvbuf);
        //printf("%s\n", filename);
    }

    // prepare file to write
    if((fp = fopen(filename, "wb")) == NULL){
        perror("connect");
		exit(1);
    }
    
    memset(sendbuf, 0, sizeof(sendbuf));
    memset(recvbuf, 0, sizeof(recvbuf));
    int count = 0;

    /* set timeout if the packet is lost */
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval));
    /* receive data and wite into file */
    while(1){
        numbytes = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&src, &len);
        if(!strcmp(buffer, "finish"))   break;  // if finish, break
        if(numbytes == 0){  // receive 0 byte, send ACK to break
            sprintf(sendbuf, "ACK");
            sendto(sock, sendbuf, sizeof(sendbuf), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
            // printf("Send ACK\n");
            break;
        }
		numbytes = fwrite(buffer, sizeof(char), numbytes, fp);  // write file

        /* send ACK to confirm receive */
        sprintf(sendbuf, "ACK");
        sendto(sock, sendbuf, sizeof(sendbuf), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
        count++;
    }

    /* get file info to confirm */    
    fclose(fp);
    if (lstat(filename, &filestat) < 0){
		exit(1);
	}
	printf("Receiving completed.\nReceived file size is %lu bytes\n", filestat.st_size);

    close(sock);
}

int main(int argc, char *argv[])
{
    char ip[20], path[100];
    int protocol, action, portno;

    /* deciding protocol and actions */
    if(!strcmp(argv[1], "tcp"))
        protocol = 0;
    else if(!strcmp(argv[1], "udp"))
        protocol = 1;
    if(!strcmp(argv[2], "send"))
        action = 0;
    else if(!strcmp(argv[2], "recv"))
        action = 1;
    
    /* getting ip, port, file path */
    strcpy(ip, argv[3]);
    sscanf(argv[4], "%d", &portno);
    if(argc == 6)   strcpy(path, argv[5]);

    /* choose protocol and action */
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