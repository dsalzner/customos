ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

CC=${ROOT_DIR}/../tutorial/Pt1-CrossCompiler/out/path/bin/i686-elf-gcc
CXX=${ROOT_DIR}/../tutorial/Pt1-CrossCompiler/out/path/bin/i686-elf-g++
LD=${CC}
AS=nasm
CFLAGS= -ffreestanding -O2 -Wall -Wextra -fno-exceptions
LDFLAGS= -ffreestanding -O2 -nostdlib -lgcc

TINYCC_INC := -I./lib/tcc-0.9.27/ -I./lib/tcc-0.9.27/include/ -I./lib/ -I.
TINYCC_CFLAGS := -DCONFIG_TCC_STATIC -DCONFIG_TCC_SEMLOCK -DCUSTOMOS

SRC := boot.asm kernel.c printf.c fatfs/fatfs_ff.c fatfs/fatfs_ffunicode.c \
ata.h common.h gdt.h idt.h isrs.h keyboard.h multiboot.h printf.h ramdisk.h terminal.h
OBJ := boot.o printf.o kernel.o fatfs/fatfs_ff.o fatfs/fatfs_ffunicode.o graphics/graphics.o shell/shell.o lib/tinycc_interpreter.o lib/libtcc.o lib/vsprintf.o lib/dtoa.o lib/qsort_nr.o lib/qsort.o lib/tinycc_compat.o lib/malloc.o

SHELL := /bin/bash

all: run

run: image
	qemu-system-i386 \
	-drive file=disk.fat,format=raw \
	-drive file=myos.iso,index=2,media=cdrom -boot d \
	-display gtk,zoom-to-fit=on \
	-netdev user,id=net0,hostfwd=tcp::10122-:22,hostfwd=tcp::15901-:5900 \
	-device rtl8139,netdev=net0,id=net0,mac=52:54:00:09:49:20

image: disk
	mkdir -p isodir/boot/grub
	cp myos.bin isodir/boot/myos.bin
	cp disk.fat isodir/boot/disk.fat
	cp grub.cfg isodir/boot/grub/grub.cfg
	grub-mkrescue -o myos.iso isodir

disk: myos.bin
	./make-disk.sh

myos.bin: ${SRC} ${OBJ}
	$(LD) -T linker.ld -o myos.bin -ffreestanding -O2 -nostdlib ${OBJ}  ${TINYCC_OBJ} -lgcc

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

shell/shell.o: shell/shell.rs shell/fontsourcecodepro_light_10pt.rs
	./rust-deps.sh
	source "${HOME}/.cargo/env" && rustup override set nightly
	source "${HOME}/.cargo/env" && rustup component add rust-src --toolchain nightly-x86_64-unknown-linux-gnu
	source "${HOME}/.cargo/env" && cd shell/ && cargo rustc --target i686-unknown-none-gnu.json -Z build-std --release -- --emit=obj && cp target/i686-unknown-none-gnu/release/libshell.rlib shell.o

lib/tinycc_interpreter.o:
	${CC} -w ${TINYCC_INCLUDES} ${TINYCC_CFLAGS} ${TINYCC_INC} -c lib/tcc-0.9.27/libtcc.c -o lib/libtcc.o
	${CC} -w ${TINYCC_INCLUDES} ${TINYCC_CFLAGS} ${TINYCC_INC} -c lib/tinycc_interpreter.c -o lib/tinycc_interpreter.o
	${CC} -w ${TINYCC_INCLUDES} ${TINYCC_CFLAGS} ${TINYCC_INC} -c lib/tccboot-0.1/vsprintf.c -o lib/vsprintf.o
	${CC} -w ${TINYCC_INCLUDES} ${TINYCC_CFLAGS} ${TINYCC_INC} -c lib/tccboot-0.1/dtoa.c -o lib/dtoa.o
	${CC} -w ${TINYCC_INCLUDES} ${TINYCC_CFLAGS} ${TINYCC_INC} -c lib/musl-1.2.5/src/stdlib/qsort.c -o lib/qsort.o
	${CC} -w ${TINYCC_INCLUDES} ${TINYCC_CFLAGS} ${TINYCC_INC} -c lib/musl-1.2.5/src/stdlib/qsort_nr.c -o lib/qsort_nr.o
	${CC} -w ${TINYCC_INCLUDES} ${TINYCC_CFLAGS} ${TINYCC_INC} -c lib/tinycc_compat.c -o lib/tinycc_compat.o
	${CC} -w ${TINYCC_INCLUDES} ${TINYCC_CFLAGS} ${TINYCC_INC} -c lib/tccboot-0.1/malloc.c -o lib/malloc.o

clean:
	rm -f shell/shell.o
	rm -rf lib/tcc-0.9.27/*.o
	rm -rf lib/*.o
	rm -f myos.bin
	rm -f *.o
	rm -f fatfs/*.o
	rm -f graphics/*.o
	rm -f myos.iso
	rm -rf isodir/
	rm -rf disk-mount/
