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

char filename[20] = "picture.jpg";
char outfilename[20] = "output.jpg";

struct in_addr localInterface;
struct sockaddr_in groupSock;
int sd;
char group_ip[20] = "226.1.1.1";
char local_ip[20] = "192.168.1.100";
int group_port = 4321;
// char databuf[BUFSIZE] = "Multicast test message.";
// int datalen = sizeof(databuf);
struct stat filestat;
// char filename[20] = "picture.jpg";
// char filebuf[BUFSIZE];
int numbytes;
int seqnum = 0;

int main() {
    ////////// open file //////////
    int numbytes = 0;
    FILE *fp = fopen(filename, "rb");
	if(fp == NULL){
        printf("Cannot open this file.\n");
        exit(1);
    }
    FILE *fp1 = fopen(outfilename, "wb");
    if(fp1 == NULL){
        printf("Cannot open this file2.\n");
        exit(1);
    }

    ////////// fec setup //////////
    // simulation parameters
    unsigned int n = 8;                     // original data length (bytes)
    fec_scheme fs = LIQUID_FEC_HAMMING74;   // error-correcting scheme

    // compute size of encoded message
    unsigned int k = fec_get_enc_msg_length(fs,n);

    // create arrays
    unsigned char msg_org[n];   // original data message
    unsigned char msg_enc[k];   // encoded/received data message
    unsigned char msg_dec[n];   // decoded data message

    // CREATE the fec object
    fec q = fec_create(fs,NULL);
    fec_print(q);

    unsigned int i;
    while(!feof(fp)){
        numbytes = fread(msg_org, sizeof(char), sizeof(msg_org), fp);
        // generate message
        // for (i=0; i<n; i++)
        //     msg_org[i] = i & 0xff;

        // encode message
        fec_encode(q, n, msg_org, msg_enc);

        // corrupt encoded message (flip bit)
        msg_enc[0] ^= 0x01;
        msg_enc[2] ^= 0x01;
        msg_enc[1] ^= 0x01;

        // decode message
        fec_decode(q, n, msg_enc, msg_dec);
		numbytes = fwrite(msg_dec, sizeof(char), numbytes, fp1);

    }
    

    // DESTROY the fec object
    fec_destroy(q);

    printf("original message:  [%3u] ",n);
    for (i=0; i<n; i++)
        printf(" %.2X", msg_org[i]);
    printf("\n");

    printf("decoded message:   [%3u] ",n);
    for (i=0; i<n; i++)
        printf(" %.2X", msg_dec[i]);
    printf("\n");

    // count bit errors
    unsigned int num_bit_errors = count_bit_errors_array(msg_org, msg_dec, n);
    printf("number of bit errors received:    %3u / %3u\n", num_bit_errors, n*8);

    printf("done.\n");
    fclose(fp);
    fclose(fp1);
    return 0;
}