#!/bin/bash

if [ "$(whoami)" != "root" ] ; then
        echo -e "\n\tYou must be root to run this script.\n"
        exit 1
fi
mknod /dev/msrdrv c 223 0﻿
chmod 666 /dev/msrdrv
insmod -f msrdrv.ko
