#ifndef __linux__
void* (*putchar)(char);
const char * name() {
    return "Example File Application";
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
  char c[] = "Hello from example file application";
  for(int i = 0; i < 36; i++) {
    putchar(c[i]);
  }
}
