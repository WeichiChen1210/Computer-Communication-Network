#!bin/bash
gcc multicast_server.c -o server -lm -lc -lliquid
gcc multicast_client.c -o client -lm -lc -lliquid
cp ./client ../