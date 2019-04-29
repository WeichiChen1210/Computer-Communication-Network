# Computer-Communication-Network
## Lab2
This lab is to implement unicast and multicast with no FEC and with FEC.
Files:
1. unicast and multicast with no FEC: nofec_server.c and nofec_client.c
2. unicast and multicast with FEC: fec_server.c and fec_client.c

### Install "liquid" library
I use the FEC functions in "liquid library", so need to install the library first.
Steps:
1. git clone the repository: https://github.com/jgaeddert/liquid-dsp.git
2. sudo apt-get install automake autoconf
3. ./bootstrap.sh
   ./configure
   make
   sudo make install
4. To unistall: move to the library folder and 
   sudo make uninstall

### Compile
1. Compile with arguments " -lm -lc -lliquid". For example: gcc fec_server -o fec_server.c -lm -lc -lliquid.
2. While executing, if you have the error with "error while loading shared libraries: libliquid.so: cannot open shared object file: No such file or directory", try use the command: sudo ldconfig.

### Execution
File will be transfer from server to client. Execute the client first, then execute server, it will transfer file right away.
Command arguments:
1. uni: unicast
   multi: multicast
2. "filename": name of the input or output file

Command:
+ server:
./nofec_server uni picture.jpg
which means to run no FEC server in unicast mode and will transfer "picture.jpg" file.
+ client:
./fec_client multi output.mp4
which means to run FEC client in multicast mode and will receive file and output as the name "output.mp4".

Note:
1. Please run client first and then server
2. Please run client and server of the same version(FEC or no FEC)

sequence number
https://stackoverflow.com/questions/1577161/passing-a-structure-through-sockets-in-c
https://stackoverflow.com/questions/26975224/c-sending-structure-over-udp-socket