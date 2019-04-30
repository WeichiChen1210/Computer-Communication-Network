# Computer-Communication-Network
## Lab2
This lab is to implement unicast and multicast with no FEC and with FEC.
Files:
1. unicast: unicast_client.c and unicast_server.c
2. multicast without FEC: nofec_server.c and nofec_client.c
3. multicast with FEC: fec_server.c and fec_client.c

### Install "liquid" library
To use the FEC functions in "liquid library", you need to install the library first.
Steps:
1. git clone the repository: https://github.com/jgaeddert/liquid-dsp.git
2. sudo apt-get install automake autoconf
3. ./bootstrap.sh
   ./configure
   make
   sudo make install
   sudo ldconfig
4. To unistall: move to the library folder and 
   sudo make uninstall

Compile:

1. With fec files: Compile with arguments " -lm -lc -lliquid". For example: gcc fec_server.c -o fec_server -lm -lc -lliquid.
2. Others: gcc "src code" -o "output file"

### Execution
These files have common kinds of arguements: "IP" "PORT" "filename".
For example:
./fec_client 127.0.0.1 8080 picture.jpg

1. Unicast: First execute server, it will wait for client's request. The filename of server's arg is the filename to be transfered, while client's is the filename to be named and created.
./unicast_client "IP" "PORT" "input filename"
./unicast_server "IP" "PORT" "output filename"
    
2. Multicast: First execute clients, they would wait for nulticast, then execute server, it will start to transfe right away. The filenames are like unicast.
./nofec_client "IP" "PORT" "input filename"
./nofec_server "IP" "PORT" "output filename"

### Input
Input can be many types of files. However, please makesure the input file exists and in the same folder.

### Output
The output files' names are the same as the names in the command line.
UDP is unreliable, losing packets are possible. So the size of the output files may be smaller than the original ones.
They may be recognized by computer but sometimes cause problems to be read when the file is not complete.
FEC can increase the correctness of each packet, but cannot decrease the lost rate(sometimes increase).

sequence number
https://stackoverflow.com/questions/1577161/passing-a-structure-through-sockets-in-c
https://stackoverflow.com/questions/26975224/c-sending-structure-over-udp-socket