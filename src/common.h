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
 * @file common.h
 * @brief Common Functions
 *
*/

#pragma once

#define TEXT_MODE_FROM_C (0)
#define GRAPHICS_MODE_FROM_C (0)
#define INIT_RAMDISK (0)

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "terminal.h"
#include "printf.h"

struct regs {
  uint32_t gs, fs, es, ds;
  uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
  uint32_t int_no, err_code;
  uint32_t eip, cs, eflags, useresp, ss;
}__attribute__((packed));

/**
 * @brief implementation of _putchar function stub from printf.h
*/
inline void _putchar(char character) {
  terminal_putchar(character);
}

inline uint8_t *memset(uint8_t *dest, size_t size, uint8_t val) {
  for (size_t i = 0; i < size; i++) {
    dest[i] = val;
  }
  return dest;
}

inline void* memcpy(void* dest, const void* src, uint32_t len) {
  const uint8_t* sp = (const uint8_t*) src;
  uint8_t* dp = (uint8_t*) dest;
  for (; len != 0; len--) *dp++ = *sp++;
  return dest;
}

inline size_t strlen(const char *string) {
  const char *p;
  if(string == NULL)
    return 0;
  for (p = string; *p != '\0'; p++)
    continue;
  return p - string;
}

char *strcpy (char *dst, const char *src) {
	char *ret_val = dst;

	for (; (*dst = *src) != 0; ++src, ++dst);
	return ret_val;
}

inline void outportb(uint16_t port, uint8_t val) {
  asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

inline uint8_t inportb(uint16_t port) {
  uint8_t ret;
  asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}

char *strncpy(char *dst, const char *src, int count) {
  char *ret_val = dst;

  for ( ; (*src != '\0') && (count != 0); count--) *dst++ = *src++;
  *dst = '\0';
  return ret_val;
}

int strcmp(const char *str1, const char *str2) {
  while ((*str2 != '\0') && (*str1 == *str2)) {
    str1++;
    str2++;
  }
  return *str1 - *str2;
}

void outl(uint32_t addr, uint32_t l) {
  __asm__ __volatile__ ("outl  %%eax, %%dx" : : "d" (addr), "a" (l) );
}

uint32_t inl(uint32_t addr) {
  uint32_t l;
  __asm__ __volatile__ ("inl  %%dx, %%eax" : "=a" (l) : "d" (addr) );
  return l;
}

void insl(uint32_t addr, uint32_t buffer, uint32_t count) {
  __asm__ __volatile__ ("cld; rep; insl" :: "D" (buffer), "d" (addr), "c" (count));
}

void insw(uint32_t addr, uint32_t buffer, uint32_t count) {
  __asm__ __volatile__ ("cld; rep; insw" :: "D" (buffer), "d" (addr), "c" (count));
}
void outsw(uint32_t addr, uint32_t buffer, uint32_t count) {
  __asm__ __volatile__ ("cld; rep; outsw" :: "D" (buffer), "d" (addr), "c" (count));
}

inline void delay() {
  for(int i = 0; i < 1000000; i++) { // accurate time delay would require CPU ClockCycles/USec conversion
    __asm__ __volatile__ ("nop");
  }
}

inline int atoi(char *str) {
  int res = 0;
  for (int i = 0; str[i] != '\0'; ++i) {
    res = res * 10 + str[i] - '0';
  }
  return res;
}
