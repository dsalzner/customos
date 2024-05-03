#!/bin/bash
set -e

BASEDIR=$(cd `dirname $0` && pwd)
CC=${BASEDIR}/../Pt1-CrossCompiler/out/path/bin/i686-elf-g++

echo "[ ] Install dependencies"
sudo apt -y install nasm xorriso qemu-system-i386

mkdir -p ${BASEDIR}/build
cd ${BASEDIR}/build

echo "[ ] Build boot code"
nasm -felf32 ../boot.asm -o boot.o

echo "[ ] Compile Kernel"
${CC} -c ../kernel.cpp -o kernel.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti

echo "[ ] Link"
${CC} -T ../linker.ld -o myos.bin -ffreestanding -O2 -nostdlib boot.o kernel.o -lgcc

echo "[ ] Check is x86"
grub-file --is-x86-multiboot myos.bin
if [ $? -eq 0 ]; then
    echo "[ ] Output is x86 bootable"
else
    echo "[W] Output is not x86 bootable"
fi

echo "[ ] Creating bootable CD-Image with Grub-Bootloader"
mkdir -p isodir/boot/grub
cp myos.bin isodir/boot/myos.bin
cp ../grub.cfg isodir/boot/grub/grub.cfg
grub-mkrescue -o myos.iso isodir

echo "[ ] Booting in QEmu"
qemu-system-i386 -cdrom myos.iso
