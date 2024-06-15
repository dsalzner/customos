#ifndef __linux__
void* (*putchar)(char);
const char * name() {
    return "Example Application";
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

void init() {
  char c[] = "Hello from example application";
  for(int i = 0; i < 30; i++) {
    putchar(c[i]);
  }
}
