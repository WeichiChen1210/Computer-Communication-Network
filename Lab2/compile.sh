#!bin/bash
gcc multicast_server.c -o server
gcc multicast_client.c -o client
cp ./client ../