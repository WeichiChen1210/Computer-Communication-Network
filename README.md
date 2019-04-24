# Computer-Communication-Network

## Install "liquid" library
1. git clone the repository: https://github.com/jgaeddert/liquid-dsp.git
2. sudo apt-get install automake autoconf
3. ./bootstrap.sh
    ./configure
    make
    sudo make install
    Successfully installed.
4. To unistall: move to the library folder and 
    sudo make uninstall
5. While executing, if you have the error with "error while loading shared libraries: libliquid.so: cannot open shared object file: No such file or directory", try use the command: sudo ldconfig.