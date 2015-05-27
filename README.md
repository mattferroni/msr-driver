# msr-driver
A simple kernel module to read MSRs on Intel machines.

# How to
sudo make
sudo make install
sudo ./install
dmesg -e
gcc -g -I. -O2 -o msrtest msrtest.c
./msrtest
