#!/bin/bash
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# -- copy applications onto disk
mkdir disk-mount
sudo mount ${SCRIPT_DIR}/../disk.fat disk-mount
sudo cp ${SCRIPT_DIR}/*.c disk-mount/
sudo umount disk-mount/
#rmdir disk-mount
