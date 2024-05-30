#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

# -- copy applications onto disk
mkdir disk-mount
sudo mount ${DIR}/../disk.fat disk-mount
sudo cp ${DIR}/*.c disk-mount/
sudo umount disk-mount/
#rmdir disk-mount
