#!/bin/bash

if [ "$(whoami)" != "root" ] ; then
        echo -e "\n\tYou must be root to run this script.\n"
        exit 1
fi

rmmod msrdrv
rm /dev/msrdrv