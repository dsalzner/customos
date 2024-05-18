CC=../Pt1-CrossCompiler/out/path/bin/i686-elf-gcc
LD=${CC}
AS=nasm
CFLAGS= -ffreestanding -O2 -Wall -Wextra -fno-exceptions
LDFLAGS= -ffreestanding -O2 -nostdlib -lgcc

SRC := boot.asm kernel.c printf.c fatfs/fatfs_ff.c fatfs/fatfs_ffunicode.c \
ata.h common.h gdt.h idt.h isrs.h keyboard.h multiboot.h printf.h ramdisk.h terminal.h

OBJ := boot.o printf.o kernel.o fatfs/fatfs_ff.o fatfs/fatfs_ffunicode.o graphics/graphics.o shell/shell.o

SHELL := /bin/bash

all: run

run: image
	qemu-system-i386 \
   -drive file=disk.fat,format=raw \
	 -drive file=myos.iso,index=2,media=cdrom -boot d

image: disk
	mkdir -p isodir/boot/grub
	cp myos.bin isodir/boot/myos.bin
	cp disk.fat isodir/boot/disk.fat
	cp grub.cfg isodir/boot/grub/grub.cfg
	grub-mkrescue -o myos.iso isodir

disk: myos.bin
	./make-disk.sh

myos.bin: ${SRC} ${OBJ}
	$(LD) -T linker.ld -o myos.bin -ffreestanding -O2 -nostdlib ${OBJ} -lgcc

boot.o: boot.asm
	${AS} -felf32 -i . boot.asm -o boot.o

kernel.o: kernel.c
	${CC} -c kernel.c -o kernel.o ${CFLAGS}

graphics/graphics.o: graphics/graphics.asm
	${AS} -felf32 -i graphics/ graphics/graphics.asm -o graphics/graphics.o

printf.o: printf.c printf.h
	${CC} -c printf.c -o printf.o ${CFLAGS}

ramdisk.o: ramdisk.c disk/ramdisk.h
	${CC} -c ramdisk.c -o ramdisk.o ${CFLAGS}

fatfs_ff.o: fatfs/fatfs_ff.c fatfs/fatfs_ff.h
	${CC} -c fatfs/fatfs_ff.c -o fatfs_ff.o ${CFLAGS}

fatfs_ffunicode.o: fatfs/fatfs_ffunicode.c fatfs/fatfs_ffunicode.h
	${CC} -c fatfs/fatfs_ffunicode.c -o fatfs_ffunicode.o ${CFLAGS}

shell/shell.o: shell/shell.rs
	./rust-deps.sh
	source "${HOME}/.cargo/env" && rustup override set nightly
	source "${HOME}/.cargo/env" && rustup component add rust-src --toolchain nightly-x86_64-unknown-linux-gnu
	source "${HOME}/.cargo/env" && cd shell/ && cargo rustc --target i686-unknown-none-gnu.json -Z build-std --release -- --emit=obj && cp target/i686-unknown-none-gnu/release/deps/shell-*.o shell.o

clean:
	rm -f myos.bin
	rm -f *.o
	rm -f fatfs/*.o
	rm -f graphics/*.o
	rm -f myos.iso
	rm -rf isodir/
