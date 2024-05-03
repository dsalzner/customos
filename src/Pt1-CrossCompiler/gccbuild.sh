#!/bin/bash
BASEDIR=$(cd `dirname $0` && pwd)
export PREFIX="${BASEDIR}/out/path/"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

echo "[ ] Install dependencies"
sudo apt -y install libgmp3-dev libmpfr-dev libisl-dev  libmpc-dev texinfo -y
sudo apt -y install nasm qemu-system-i386 xorriso

echo "[ ] Create directories"
rm -rf ${BASEDIR}/out/
mkdir -p ${PREFIX}
mkdir -p ${BASEDIR}/out/src/
cd ${BASEDIR}/out/src/

echo "[ ] Download code archives"
cd ${BASEDIR}
wget --no-clobber ftp://ftp.gnu.org/gnu/binutils/binutils-2.26.tar.gz
wget --no-clobber ftp://ftp.gnu.org/gnu/gcc/gcc-6.1.0/gcc-6.1.0.tar.gz

echo "[ ] Build binutils"
mkdir ${BASEDIR}/out/src/build-binutils/
cd ${BASEDIR}/out/src/
tar -xzf ${BASEDIR}/binutils-2.26.tar.gz
cd ${BASEDIR}/out/src/build-binutils/
../binutils-2.26/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --disable-werror
make
make install

echo "[ ] Build gcc"
mkdir ${BASEDIR}/out/src/build-gcc/
cd ${BASEDIR}/out/src/
tar -xzf ${BASEDIR}/gcc-6.1.0.tar.gz
cd ${BASEDIR}/out/src/build-gcc/

# -- patch isl include header
sed -i 's/#include <isl\/ast_build.h>/#include <isl\/ast_build.h>\n#include <isl\/id.h>\n#include <isl\/space.h>\n/g' ../gcc-6.1.0/gcc/graphite.h
../gcc-6.1.0/configure CXXFLAGS="-fpermissive" --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers

make all-gcc
make all-target-libgcc

make install-gcc
make install-target-libgcc
