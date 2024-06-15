#ifndef __linux__
void* (*putchar)(char);
const char * name() {
    return "Fibonacci";
}
void setPutCharCallback(void* (*putchar_)(char)) {
  putchar = putchar_;
}
#else
#include <cstdio>
void init();
void putchar(char ch) {
  printf("%c", ch);
}
int main() {
  init();
  return 0;
}
#endif

// --

int atoi(char *str) {
  int res = 0;
  for (int i = 0; str[i] != '\0'; ++i) {
    res = res * 10 + str[i] - '0';
  }
  return res;
}

unsigned int strlen(const char *str) {
  const char *p;
  if(str == NULL) {
    return 0;
  }
  for (p = str; *p != '\0'; p++) {
    continue;
  }
  return p - str;
}

void reverse(char str[]) {
  int i, j;
  char ch;
  for (i = 0, j = strlen(str) - 1; i < j; i++, j--) {
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
  for(i = 0; i < strlen(str); i++) {
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
