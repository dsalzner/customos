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
  return "Example Application";
}

// --

void init() {
  char c[] = "Hello from example application";
  for(int i = 0; i < 30; i++) {
    putchar(c[i]);
  }
}
