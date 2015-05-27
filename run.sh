#!/bin/bash

if [ "$(whoami)" != "root" ] ; then
        echo -e "\n\tYou must be root to run this script.\n"
        exit 1
fi

echo "Make and install ---"
make
make install
echo "Load kernel module and check if everything is ok ---"
chmod +x *.sh
sudo ./uninstall.sh
sudo ./install
lsmod | grep msrdrv
echo "Compile and run the test code ---"
gcc -g -I. -O2 -o msrtest msrtest.c
./msrtest
echo "Fixing permissions and unload driver ---"
sudo ./uninstall.sh
chown matteo:matteo *