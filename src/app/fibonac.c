#ifndef __GNUC__
void* (*putchar)(char);

void setPutCharCallback(void* (*putchar_)(char)) {
  putchar = putchar_;
}
#else
#include <stdio.h>
void init();
int main() {
  init();
  return 0;
}
#endif

// --

const char * name() {
    return "Fibonacci";
}

int atoi(char *str) {
  int res = 0;
  for (int i = 0; str[i] != '\0'; ++i) {
    res = res * 10 + str[i] - '0';
  }
  return res;
}

unsigned int strlen_(const char *str) {
  for(unsigned int i = 0; i < 100; i++) { // -- capped at 100
    if(str[i] == '\0') {
      return i;
    }
  }
  return 0;
}

void reverse(char str[]) {
  unsigned int i, j;
  char ch;
  for (i = 0, j = strlen_(str) - 1; i < j; i++, j--) {
    ch = str[i];
    str[i] = str[j];
    str[j] = ch;
  }
}

// -- from "The C Programming Language"
void itoa(int number, char * str) {
  int i, sign;
  if ((sign = number) < 0) {
    number = -number;
  }
  i = 0;
  do {
     str[i++] = number % 10 + '0';
  } while ((number /= 10) > 0);
  if (sign < 0) {
    str[i++] = '-';
  }
  str[i] = '\0';
  reverse(str);
}

void putstr(const char *str) {
  int i = 0;
  for(i = 0; i < strlen_(str); i++) {
    putchar(str[i]);
  }
}

void init() {
  char str[8];

  int arr[20];
  arr[0] = 1;
  arr[1] = 1;  
  for(int i = 2; i < 20; i++) {
    arr[i] = arr[i-2] + arr[i-1];
  }

  for(int i = 0; i < 20; i++) {
    itoa(arr[i], str);
    putstr(str);
    putstr(", ");
  }
}
