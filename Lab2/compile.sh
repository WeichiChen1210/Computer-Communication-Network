#!bin/bash
gcc multicast_server.c -o server -lm -lc -lliquid
gcc multicast_client.c -o client -lm -lc -lliquid
gcc nofec_server.c -o nofec_server -lm -lc -lliquid
gcc nofec_client.c -o nofec_client -lm -lc -lliquid
cp ./client ../