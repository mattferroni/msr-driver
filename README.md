# msr-driver
A simple kernel module to read MSRs on Intel machines.
Freely inspired by: http://www.mindfruit.co.uk/2012/11/fun-with-msrs-counting-performance.html

# How to

Make and install:

```
sudo make
sudo make install
```

Load kernel module and check if everything is ok:

```
sudo ./install
dmesg -e
lsmod | grep msrdrv
```

Compile and run the test code:

```
gcc -g -I. -O2 -o msrtest msrtest.c
./msrtest
```
