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

/*
===== Additional glue code =====
*/

#include "tinycc_compat.h"

// -- heap
#define NUM_PAGE_FRAMES (0x100000000 / 0x1000 / 8)
#define PAGE_FRAME_SIZE 0x1000

#define MALLOC_MAX_SIZE NUM_PAGE_FRAMES / 8
uint8_t _end[NUM_PAGE_FRAMES / 8];

int printf_hidden(const char *format, ...) { }

// --

int errno=0;
char msg_buf[1024];

// -- helper functions
void exit(int retval) {
  printf("[!] Exit %d\n", retval);
}

int fputs(const char *ptr, FILE *stream) {
  return fwrite(ptr, strlen(&ptr), sizeof(char), stream);
}

int unlink(const char *path) {
  printf("[W] TinyCC - unlink is not implemented\n");
  return 0;
}

int fflush(FILE *f)
{
  return 0;
}

int do_div(num, base) {
  return num/base;
}

void putchar(const char chr) {
  terminal_putchar(chr);
}

void putstr(const char *string) {
  const char *p;
  if(string == NULL)
    return 0;
  for (p = string; *p != '\0'; p++)
    putchar(p[0]);
}

/*
===== taken from lib/musl-1.2.5/ =====
 *   most notable takeover of individual functions:
 *
 *   - string/strchrnul.c
 *   - string/strchr.c
 *   - string/strnlen.c
 *   - string/memchr.c
 *   - string/memrchr.c
 *   - string/strrchr.c
 *
 *   - src/internal/atomic.h
 *   - arch/generic/bits/errno.h
 *
 *   - include/ctype.h
 *   - ctype/isxdigit.c
 *   - ctype/toupper.c
 *
 *   - network/htonl.c
 *   - network/ntohl.c
*/


// -- musl-1.2.5/src/string/strncmp.c
int strncmp(const char *_l, const char *_r, size_t n)
{
	const unsigned char *l=(void *)_l, *r=(void *)_r;
	if (!n--) return 0;
	for (; *l && *r && n && *l == *r ; l++, r++, n--);
	return *l - *r;
}

// -- musl-1.2.5/src/string/strncat.c
char *strncat(char *restrict d, const char *restrict s, size_t n)
{
	char *a = d;
	d += strlen(d);
	while (n && *s) n--, *d++ = *s++;
	*d++ = 0;
	return a;
}

// -- musl-1.2.5/src/string/memcmp.c
int memcmp(const void *vl, const void *vr, size_t n)
{
	const unsigned char *l=vl, *r=vr;
	for (; n && *l == *r; n--, l++, r++);
	return n ? *l-*r : 0;
}

// -- musl-1.2.5/src/string/memmove.c
void *memmove(void *dest, const void *src, size_t n)
{
	char *d = dest;
	const char *s = src;

	if (d==s) return d;
	if ((uintptr_t)s-(uintptr_t)d-n <= -2*n) return memcpy(d, s, n);

	if (d<s) {
		for (; n; n--) *d++ = *s++;
	} else {
		while (n) n--, d[n] = s[n];
	}

	return dest;
}

// -- vim ../tutorial/Pt10-TinyCC-Attempt2-Musl/lib/musl-1.2.5/src/string/strcat.c
char *strcat(char *restrict dest, const char *restrict src)
{
	strcpy(dest + strlen(dest), src);
	return dest;
}

// -- musl-1.2.5/src/ctype/isxdigit.c
int isxdigit(int c)
{
	return isdigit(c) || ((unsigned)c|32)-'a' < 6;
}

// -- musl-1.2.5/src/string/strchrnul.c
char *__strchrnul(const char *s, int c)
{
	c = (unsigned char)c;
	if (!c) return (char *)s + strlen(s);
	for (; *s && *(unsigned char *)s != c; s++);
	return (char *)s;
}


// -- musl-1.2.5/src/string/strchr.c
char *strchr(const char *s, int c)
{
	char *r = __strchrnul(s, c);
	return *(unsigned char *)r == (unsigned char)c ? r : 0;
}

// -- musl-1.2.5/src/string/strnlen.c
size_t strnlen(const char *s, size_t n)
{
	const char *p = memchr(s, 0, n);
	return p ? p-s : n;
}

// -- musl-1.2.5/src/string/memchr.c
#define SS (sizeof(size_t))
#define ALIGN (sizeof(size_t)-1)
#define ONES ((size_t)-1/UCHAR_MAX)
#define HIGHS (ONES * (UCHAR_MAX/2+1))
#define HASZERO(x) ((x)-ONES & ~(x) & HIGHS)

void *memchr(const void *src, int c, size_t n)
{
	const unsigned char *s = src;
	c = (unsigned char)c;
	for (; n && *s != c; s++, n--);
	return n ? (void *)s : 0;
}

// -- musl-1.2.5/src/ctype/toupper.c
int toupper(int c)
{
	if (islower(c)) return c & 0x5f;
	return c;
}

// -- musl-1.2.5/src/string/memrchr.c
void *__memrchr(const void *m, int c, size_t n)
{
	const unsigned char *s = m;
	c = (unsigned char)c;
	while (n--) if (s[n]==c) return (void *)(s+n);
	return 0;
}

// -- musl-1.2.5/src/string/strrchr.c
char *strrchr(const char *s, int c)
{
	return __memrchr(s, c, strlen(s) + 1);
}

// -- musl-1.2.5/include/byteswap.h
static __inline uint16_t __bswap_16(uint16_t __x)
{
	return __x<<8 | __x>>8;
}

static __inline uint32_t __bswap_32(uint32_t __x)
{
	return __x>>24 | __x>>8&0xff00 | __x<<8&0xff0000 | __x<<24;
}

#define bswap_16(x) __bswap_16(x)
#define bswap_32(x) __bswap_32(x)

// -- musl-1.2.5/src/network/htonl.c
uint32_t htonl(uint32_t n)
{
	union { int i; char c; } u = { 1 };
	return u.c ? bswap_32(n) : n;
}

// -- musl-1.2.5/src/network/ntohl.c
uint32_t ntohl(uint32_t n)
{
	union { int i; char c; } u = { 1 };
	return u.c ? bswap_32(n) : n;
}

/*
===== taken from lib/tccboot-0.1/lib.c =====
*/

#include "stdarg.h" // va_start

char msg_buf[1024];

void puts(const char *s)
{
    putstr(s);
    putstr("\n");
}

int printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    vsnprintf(msg_buf, sizeof(msg_buf), fmt, ap);
    putstr(msg_buf);
    va_end(ap);
    return 0;
}

FILE *stderr;

int fprintf(FILE *f, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    vsnprintf(msg_buf, sizeof(msg_buf), fmt, ap);
    putstr(msg_buf);
    va_end(ap);
    return 0;
}

void getcwd(char *buf, size_t buf_size)
{
    strcpy(buf, "/");
}

void fatal(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    putstr("tccboot: panic: ");
    vsnprintf(msg_buf, sizeof(msg_buf), fmt, ap);
    putstr(msg_buf);
    putstr("\n");
    va_end(ap);
    exit(1);
}

int errno;

long strtol(const char *nptr, char **endptr, int base)
{
    return simple_strtol(nptr, endptr, base);
}

long long strtoll(const char *nptr, char **endptr, int base)
{
    return simple_strtoll(nptr, endptr, base);
}

unsigned long strtoul(const char *nptr, char **endptr, int base)
{
    return simple_strtoul(nptr, endptr, base);
}

unsigned long long strtoull(const char *nptr, char **endptr, int base)
{
    return simple_strtoull(nptr, endptr, base);
}

int atoi(const char *s)
{
    return strtol(s, NULL, 10);
}

int setjmp(jmp_buf buf)
{
    return 0;
}

void longjmp(jmp_buf buf, int val)
{
    exit(1);
}

//#define MALLOC_MAX_SIZE (128 * 1024 * 1024)
//extern uint8_t _end;


uint8_t *malloc_ptr = &_end;

void *sbrk(int increment)
{
    COS_TINYCC_DEBUG("tinycc_compat::sbrk - start\n");

    //printf("heap ptr before %d \n", malloc_ptr);

    uint8_t *ptr, *new_ptr;

    if (increment == 0)
	return malloc_ptr;
    ptr = malloc_ptr;
    new_ptr = malloc_ptr + increment;
    if (new_ptr > (&_end + MALLOC_MAX_SIZE)) {
	errno = ENOMEM;
	COS_TINYCC_DEBUG("tinycc_compat::sbrk - no mem\n");
	return (void *)-1;
    }
    malloc_ptr = new_ptr;

    //printf("heap ptr after %d\n", malloc_ptr);

    COS_TINYCC_DEBUG("tinycc_compat::sbrk - end\n");

    return ptr;
}

uint8_t *romfs_base;

/* The basic structures of the romfs filesystem */

#define ROMBSIZE BLOCK_SIZE
#define ROMBSBITS BLOCK_SIZE_BITS
#define ROMBMASK (ROMBSIZE-1)
#define ROMFS_MAGIC 0x7275

#define ROMFS_MAXFN 128

#define __mkw(h,l) (((h)&0x00ff)<< 8|((l)&0x00ff))
#define __mkl(h,l) (((h)&0xffff)<<16|((l)&0xffff))
#define __mk4(a,b,c,d) htonl(__mkl(__mkw(a,b),__mkw(c,d)))
#define ROMSB_WORD0 __mk4('-','r','o','m')
#define ROMSB_WORD1 __mk4('1','f','s','-')

/* On-disk "super block" */

struct romfs_super_block {
	uint32_t word0;
	uint32_t word1;
	uint32_t size;
	uint32_t checksum;
	char name[0];		/* volume name */
};

/* On disk inode */

struct romfs_inode {
	uint32_t next;		/* low 4 bits see ROMFH_ */
	uint32_t spec;
	uint32_t size;
	uint32_t checksum;
	char name[0];
};

#define ROMFH_TYPE 7
#define ROMFH_HRD 0
#define ROMFH_DIR 1
#define ROMFH_REG 2
#define ROMFH_SYM 3
#define ROMFH_BLK 4
#define ROMFH_CHR 5
#define ROMFH_SCK 6
#define ROMFH_FIF 7
#define ROMFH_EXEC 8

/* Alignment */

#define ROMFH_ALIGN 16

#define MAX_FILE_HANDLES 256

typedef struct FileHandle {
    uint8_t *base;
    unsigned long size, max_size;
    unsigned long pos;
    int is_rw;
} FileHandle;

static FileHandle file_handles[MAX_FILE_HANDLES];

static uint8_t *output_base;
static size_t output_max_size, output_size;
static uint8_t output_filename[128];

void set_output_file(const char *filename,
		     uint8_t *base, size_t size)
{
    strcpy(output_filename, filename);
    output_base = base;
    output_max_size = size;
}

long get_output_file_size(void)
{
    return output_size;
}

static inline int get_file_handle(void)
{
    int i;

    for(i = 0; i < MAX_FILE_HANDLES; i++) {
	if (!file_handles[i].base)
	    return i;
    }
    errno = ENOMEM;
    return -1;
}

void show_filename(const char * filename) {
  printf("file: %s\n", filename);
}

int open(const char *filename, int access, ...)
{
    struct romfs_super_block *sb;
    unsigned long addr, next;
    struct romfs_inode *inode;
    int type, fd, len;
    char dir[1024];
    const char *p, *r;

    if (access & O_CREAT) {
	/* specific case for file creation */
	if (strcmp(filename, output_filename) != 0)
	    return -EPERM;
	fd = get_file_handle();
	if (fd < 0)
	    return fd;
	file_handles[fd].base = output_base;
	file_handles[fd].max_size = output_max_size;
	file_handles[fd].is_rw = 1;
	file_handles[fd].pos = 0;
	file_handles[fd].size = 0;
	return fd;
    }

    show_filename(filename);

    sb = (void *)romfs_base;
    if (sb->word0 != ROMSB_WORD0 ||
	sb->word1 != ROMSB_WORD1)
	goto fail;
    addr = ((unsigned long)sb->name + strlen(sb->name) + 1 + ROMFH_ALIGN - 1) &
	~(ROMFH_ALIGN - 1);
    inode = (void *)addr;

    /* search the directory */
    p = filename;
    while (*p == '/')
	p++;
    for(;;) {
	r = strchr(p, '/');
	if (!r)
	    break;
	len = r - p;
	if (len > sizeof(dir) - 1)
	    goto fail;
	memcpy(dir, p, len);
	dir[len] = '\0';
	p = r + 1;
#ifdef ROMFS_DEBUG
	printf("dir=%s\n", dir);
#endif

	for(;;) {
	    next = ntohl(inode->next);
	    type = next & 0xf;
	    next &= ~0xf;
	    if (!strcmp(dir, inode->name)) {
#ifdef ROMFS_DEBUG
		printf("dirname=%s type=0x%x\n", inode->name, type);
#endif
		if ((type & ROMFH_TYPE) == ROMFH_DIR) {
		chdir:
		    addr = ((unsigned long)inode->name + strlen(inode->name) +
			    1 + ROMFH_ALIGN - 1) &
			~(ROMFH_ALIGN - 1);
		    inode = (void *)addr;
		    break;
		} else if ((type & ROMFH_TYPE) == ROMFH_HRD) {
		    addr = ntohl(inode->spec);
		    inode = (void *)(romfs_base + addr);
		    next = ntohl(inode->next);
		    type = next & 0xf;
		    if ((type & ROMFH_TYPE) != ROMFH_DIR)
			goto fail;
		    goto chdir;
		}
	    }
	    if (next == 0)
		goto fail;
	    inode = (void *)(romfs_base + next);
	}
    }
    for(;;) {
	next = ntohl(inode->next);
	type = next & 0xf;
	next &= ~0xf;
#ifdef ROMFS_DEBUG
	printf("name=%s type=0x%x\n", inode->name, type);
#endif
	if ((type & ROMFH_TYPE) == ROMFH_REG) {
	    if (!strcmp(p, inode->name)) {
		fd = get_file_handle();
		if (fd < 0)
		    return fd;
		addr = ((unsigned long)inode->name + strlen(inode->name) +
			1 + ROMFH_ALIGN - 1) &
		    ~(ROMFH_ALIGN - 1);
		file_handles[fd].base = (void *)addr;
		file_handles[fd].is_rw = 0;
		file_handles[fd].pos = 0;
		file_handles[fd].size = ntohl(inode->size);
		return fd;
	    }
	}
	if (next == 0)
	    break;
	inode = (void *)(romfs_base + next);
    }
 fail:
    errno = ENOENT;
    return -1;
}

int read(int fd, void *buf, size_t size)
{
    FileHandle *fh = &file_handles[fd];
    int len;

    len = fh->size - fh->pos;
    if (len > (int)size)
	len = size;
    memcpy(buf, fh->base + fh->pos, len);
    fh->pos += len;
    return len;
}

int write(int fd, const void *buf, size_t size)
{
    FileHandle *fh = &file_handles[fd];
    int len;

    if (!fh->is_rw)
	return -EIO;
    len = fh->max_size - fh->pos;
    if ((int)size > len) {
	errno = ENOSPC;
	return -1;
    }
    memcpy(fh->base + fh->pos, buf, size);
    fh->pos += size;
    if (fh->pos > fh->size) {
	fh->size = fh->pos;
	output_size = fh->pos;
    }
    return size;
}

long lseek(int fd, long offset, int whence)
{
    FileHandle *fh = &file_handles[fd];

    switch(whence) {
    case SEEK_SET:
	break;
    case SEEK_END:
	offset += fh->size;
	break;
    case SEEK_CUR:
	offset += fh->pos;
	break;
    default:
	errno = EINVAL;
	return -1;
    }
    if (offset < 0) {
	errno = EINVAL;
	return -1;
    }
    fh->pos = offset;
    return offset;
}

int close(int fd)
{
    FileHandle *fh = &file_handles[fd];
    fh->base = NULL;
    return 0;
}

float strtof(const char *nptr, char **endptr)
{
    fatal("unimplemented: %s", __func__ );
}

long double strtold(const char *nptr, char **endptr)
{
    fatal("unimplemented: %s", __func__ );
}

double ldexp(double x, int exp)
{
    fatal("unimplemented: %s", __func__ );
}

FILE *fopen(const char *path, const char *mode)
{
    fatal("unimplemented: %s", __func__ );
}

FILE *fdopen(int fildes, const char *mode)
{
  	COS_TINYCC_DEBUG("tinycc_compat - fdopen\n");

    FILE *f;
    f = cos_malloc(sizeof(FILE));
    f->fd = fildes;
    return f;
}

int fclose(FILE *stream)
{
    close(stream->fd);
    free(stream);
}

size_t fwrite(const  void  *ptr,  size_t  size,  size_t  nmemb,  FILE *stream)
{
    int ret;
    if (nmemb == 1) {
	ret = write(stream->fd, ptr, size);
	if (ret < 0)
	    return ret;
    } else {
	ret = write(stream->fd, ptr, size * nmemb);
	if (ret < 0)
	    return ret;
	if (nmemb != 0)
	    ret /= nmemb;
    }
    return ret;
}

int fputc(int c, FILE *stream)
{
    uint8_t ch = c;
    write(stream->fd, &ch, 1);
    return 0;
}

// ===== =====
