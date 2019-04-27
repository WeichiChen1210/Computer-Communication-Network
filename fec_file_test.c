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

char filename[20] = "Sodagreen.mp4";
char outfilename[20] = "output.mp4";
int numbytes;
struct stat filestat;

int main() {
    ////////// open file //////////
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
    unsigned int n = 256;                     // original data length (bytes)
    fec_scheme fs = LIQUID_FEC_HAMMING74;   // error-correcting scheme

    // compute size of encoded message
    unsigned int k = fec_get_enc_msg_length(fs,n);
    printf("k %d\n", k);
    // create arrays
    unsigned char msg_org[n];   // original data message
    unsigned char msg_enc[k];   // encoded/received data message
    unsigned char msg_dec[n];   // decoded data message

    // CREATE the fec object
    fec q = fec_create(fs,NULL);
    fec_print(q);

    unsigned int count = 0;
    while(!feof(fp)){
        numbytes = fread(msg_org, sizeof(unsigned char), sizeof(msg_org), fp);
        // generate message
        // for (i=0; i<n; i++)
        //     msg_org[i] = i & 0xff;

        // encode message
        fec_encode(q, n, msg_org, msg_enc);

        // corrupt encoded message (flip bit)
        msg_enc[0] ^= 0x01;
        msg_enc[2] ^= 0x01;
        msg_enc[1] ^= 0x01;
        msg_enc[10] ^= 0x01;
        msg_enc[100] ^= 0x01;

        // decode message
        fec_decode(q, n, msg_enc, msg_dec);
		numbytes = fwrite(msg_dec, sizeof(unsigned char), sizeof(msg_dec), fp1);
        unsigned int num_bit_errors = count_bit_errors_array(msg_org, msg_dec, n);
        if(num_bit_errors != 0) count++;
        // printf("number of bit errors received:    %3u / %3u\n", num_bit_errors, n*8);
    }
    

    // DESTROY the fec object
    fec_destroy(q);

    // usigned int i;
    // printf("original message:  [%3u] ",n);
    // for (i=0; i<n; i++)
    //     printf(" %.2X", msg_org[i]);
    // printf("\n");
    //     printf("%s\n", msg_org);
    // printf("decoded message:   [%3u] ",n);
    // for (i=0; i<n; i++)
    //     printf(" %.2X", msg_dec[i]);
    // printf("\n");
    //     printf("%s\n", msg_dec);
    // count bit errors

    printf("errors: %d\ndone.\n", count);
    fclose(fp);
    fclose(fp1);
    return 0;
}