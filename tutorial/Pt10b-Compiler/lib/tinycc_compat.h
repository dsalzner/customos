/*
CustomOS
Copyright (C) 2023 D.Salzner <mail@dennissalzner.de>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file tinycc_compat.h
 * @brief TinyCC Compatibility Layer
 *
 * All remaining symbols, defines and typedefs that the CustomOS needs to provide for TinyCC to function.
 *
 * disabled features in TinyCC by C-define "CUSTOMOS":
 * - disables all references to time_t from TinyCC
 * - disables memory protection handling as it's not required in the CustomOS
 * - features related to environment variables, getenv
 * - just like tccboot-0.1, tinycc will now only read and write to memory instead of to disk
 *   most fopen/fclose/read/write functions use the heap instead of files
 *
 * dependencies:
 * - for variadic string functions and fprintf I'm relying on vsprintf.c from tccboot-0.1
 * - for double to ascii functions on dtoa.c from tccboot-0.1
 * - and for malloc on malloc.c from tccboot-0.1
 * - qsort is taken from musl-1.2.5
 *
 * takeover
 * - this is a mix of adaptations from tccboot-0.1/libc.c, musl-1.2.5 and glue code
 *
*/

#pragma once

#define TCC_VERSION "0.9.27"

#include "libtcc.h"

#define O_RDONLY 00
#define O_WRONLY 01
#define O_CREAT 0x0100
#define O_TRUNC 0x0200

int printf_hidden(const char *format, ...);
#define COS_TINYCC_DEBUG printf_hidden

void putchar(const char chr);

#ifndef CUSTOMOS

  #include <stdio.h>
  #include <errno.h>
  #define cos_malloc malloc
  
#else

#include "stdarg.h"

typedef int jmp_buf[6];

/*
===== Additional glue code =====
*/

int errno;

#define TYPEDEF typedef
typedef unsigned int size_t;
typedef unsigned int ssize_t;
typedef unsigned long uintptr_t;

typedef signed char int8_t;
typedef short int int16_t;
typedef int int32_t;
typedef long long int int64_t;
typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long int uint64_t;

#define NULL 0

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define __isoc_va_list char *

#define PROT_READ 0x1
#define PROT_WRITE 0x2
#define PROT_EXEC 0x4
#define PROT_NONE 0x0

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef char *va_list;

typedef struct FILE {
    int fd;
} FILE;

FILE *stderr;
FILE *stdout;

int fprintf(FILE *f, const char *fmt, ...);
int printf(const char *fmt, ...);
FILE *fopen(const char *path, const char *mode);
int fclose(FILE *stream);

/*
===== taken from https://stackoverflow.com/questions/37897645/page-size-undeclared-c =====
*/

#define PAGE_SHIFT      12
#ifdef __ASSEMBLY__
#define PAGE_SIZE       (1 << PAGE_SHIFT)
#else
#define PAGE_SIZE       (1UL << PAGE_SHIFT)
#endif
#define PAGE_MASK       (~(PAGE_SIZE-1))

/*
===== taken from linux limits.h =====
*/

#define INT_MAX 2147483647
#define ERANGE 34
#define UCHAR_MAX 255

/*
===== taken from lib/musl-1.2.5/ =====
*/

// -- musl-1.2.5/include/ctype.h
static __inline int __isspace(int _c)
{
        return _c == ' ' || (unsigned)_c-'\t' < 5;
}

#define isalpha(a) (0 ? isalpha(a) : (((unsigned)(a)|32)-'a') < 26)
#define isdigit(a) (0 ? isdigit(a) : ((unsigned)(a)-'0') < 10)
#define islower(a) (0 ? islower(a) : ((unsigned)(a)-'a') < 26)
#define isupper(a) (0 ? isupper(a) : ((unsigned)(a)-'A') < 26)
#define isprint(a) (0 ? isprint(a) : ((unsigned)(a)-0x20) < 0x5f)
#define isgraph(a) (0 ? isgraph(a) : ((unsigned)(a)-0x21) < 0x5e)
#define isspace(a) __isspace(a)

char *strchr(const char *s, int c);

// -- musl-1.2.5/arch/generic/bits/errno.h
#define EPERM            1
#define ENOENT           2
#define EIO              5
#define ENOMEM          12
#define EINVAL          22
#define ENOSPC          28

// -- musl-1.2.5/src/internal/atomic.h - a_ctz_l for qsort
#ifndef a_ctz_32
#define a_ctz_32 a_ctz_32
static inline int a_ctz_32(uint32_t x)
{
#ifdef a_clz_32
        return 31-a_clz_32(x&-x);
#else
        static const char debruijn32[32] = {
                0, 1, 23, 2, 29, 24, 19, 3, 30, 27, 25, 11, 20, 8, 4, 13,
                31, 22, 28, 18, 26, 10, 7, 12, 21, 17, 9, 6, 16, 5, 15, 14
        };
        return debruijn32[(x&-x)*0x076be629 >> 27];
#endif
}
#endif

static inline int a_ctz_l(unsigned long x)
{
        return a_ctz_32(x);
}

#endif

// ===== =====
