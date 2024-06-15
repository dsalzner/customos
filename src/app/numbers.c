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
    return "Numbers";
}

void putstr(const char *str) {
  while(*str != '\0') {
    putchar(*str);
    str++;
  }
}

int log10_(unsigned int number) {
  unsigned int i = 0;
  unsigned int j = 1;
  for(i = 0; i < 5; i++) {
    j *= 10;
    if (number < j) {
      return i;
    }
  }
  return 0;
}

void itoa(unsigned int number, char * str) {
  if(number == 0) {
    str[0] = '0';
    str[1] = '\0';
    return;
  }
  int i = 0;
  int digits = log10_(number);
  while(number > 0) {
    str[digits - i] = number % 10 + '0';
    number = number / 10;
    i++;
  }
  str[i] = '\0';
}

void init() {
  int i = 0;
  char str1[20];
  char str2[] = "Numbers: ";
  putstr(str2);
  for(i = 0; i < 20; i++) {
    itoa(i, str1);
    putstr(str1);
    putstr(", ");
  }
}

