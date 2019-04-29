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
#define BUFSIZE 256

struct sockaddr_in localSock;
struct ip_mreq group;
int sd;
char group_ip[20] = "226.1.1.1";
char local_ip[20] = "192.168.208.193";
int group_port = 4321;
int datalen;
char databuf[BUFSIZE];
struct stat filestat;
char filename[200] = "output.jpg";
char org_filename[20] = "picture.jpg";
int numbytes;
int seqnum = 0;
int arr[10];
 
int main(int argc, char *argv[])
{
	if(argc > 1) strcpy(filename, argv[1]);
	/////////////// fec setup ///////////////
	// simulation parameters
    unsigned int n = BUFSIZE;                     // original data length (bytes)
    fec_scheme fs = LIQUID_FEC_HAMMING74;   // error-correcting scheme

    // compute size of encoded message
    unsigned int k = fec_get_enc_msg_length(fs,n);

    // create arrays
    // unsigned char msg_org[n];   // original data message
    unsigned char msg_enc[k];   // encoded/received data message
    unsigned char msg_dec[n];   // decoded data message

    // CREATE the fec object
    fec q = fec_create(fs,NULL);
    fec_print(q);

    unsigned int i;

	/////////////// socket setup ///////////////
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
	
	/////////////// file operation ///////////////
	FILE *fp;
	if((fp = fopen(filename, "wb")) == NULL){
		printf("Cannot write this file.\n");
        exit(1);
	}

	printf("Reading datagram message...");
	int recvcount = 0, count = 0, lost = 0;
	seqnum = 0;
	/* Read from the socket. */
	while((numbytes = read(sd, msg_enc, sizeof(msg_enc))) > 0){
		fec_decode(q, n, msg_enc, msg_dec);
		numbytes = sizeof(msg_dec);
		if(!strncmp(msg_dec, "finish", numbytes)){
			fclose(fp);
			break;  // if finish, break
		}

		if(numbytes > 0) {
			numbytes = fwrite(msg_dec+4, sizeof(unsigned char), sizeof(msg_dec)-4, fp);
		}
		seqnum = msg_dec[0] << 24;
		seqnum += msg_dec[1] << 16;
		seqnum += msg_dec[2] << 8;
		seqnum += msg_dec[3];
		
		if(seqnum > count){
			lost += abs(seqnum-count);
			count = seqnum+1;
		}
		else count++;
		recvcount++;
	}
	
	printf("OK.\nReceived %d packets, lost %d.\n", recvcount, lost);

	if (lstat(filename, &filestat) < 0){
		exit(1);
	}
	printf("The file size is %ld bytes\n", filestat.st_size);

	/////////////// file error check ///////////////
	// FILE *fp1, *fp2;
	// if((fp1 = fopen(org_filename, "rb")) == NULL){
	// 	printf("Cannot write this file.\n");
    //     exit(1);
	// }
	// if((fp2 = fopen(filename, "rb")) == NULL){
	// 	printf("Cannot write this file.\n");
    //     exit(1);
	// }
	
	// unsigned char file1[n];
	// unsigned char file2[n];
	// int error_count = 0;
	// while(!feof(fp1) && !feof(fp2)){
	// 	fread(file1, sizeof(unsigned char), sizeof(file1), fp1);
	// 	fread(file2, sizeof(unsigned char), sizeof(file2), fp2);
	// 	unsigned int num_bit_errors = count_bit_errors_array(file1, file2, n);
    // 	if(num_bit_errors != 0) error_count++;
	// }
	// fclose(fp1);
	// fclose(fp2);
	// printf("errors: %d\n", error_count);


	return 0;
}
