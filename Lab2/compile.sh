#!bin/bash
gcc fec_server.c -o fec_server -lm -lc -lliquid
gcc fec_client.c -o fec_client -lm -lc -lliquid
gcc nofec_server.c -o nofec_server -lm -lc -lliquid
gcc nofec_client.c -o nofec_client -lm -lc -lliquid