#!/bin/bash

if [ ! -f "disk.fat" ]; then
    dd if=/dev/zero of=disk.fat bs=8M count=1
    LOOP=`sudo losetup -f`
    sudo losetup $LOOP disk.fat
    sudo mkfs.vfat $LOOP
    mkdir mount/
    sudo mount $LOOP mount/
    echo "Lorem ipsum dolor sit amet, consectetur adipisici elit"  | sudo tee mount/testfile.txt
    find mount/
    sudo umount mount
    sudo losetup -d $LOOP
    sudo chown $SUDO_USER:$SUDO_USER disk.fat
fi
