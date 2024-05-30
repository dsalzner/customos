#ifndef __linux__
void* (*putchar)(char);
const char * name() {
    return "Sirpinski-Triangle";
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

int lineA[53];
int lineB[53];
bool swap = false;

void printLine(int * line, int row, int lines) {
  for(int i = 0; i < lines - row - 1; i++) {
    putchar(' ');
  }
  for(int i = 0; i < 53; i++) {
    if(line[i] > 0) {
      if(line[i] % 2 == 0) {
        putchar(' ');
      } else {
        putchar('x');
      }
    }
    putchar(' ');
  }
  putchar('\n');
}

void init() {
  int lines = 24;
  lineA[0] = '1';
  lineB[0] = '1';
  lineB[1] = '1';
  
  printLine(lineA, 0, lines + 2);
  printLine(lineB, 1, lines + 2);

  for(int j = 0; j < lines; j++) {
    int * prev = lineB;
    int * next = lineA;
    if(swap) {
      prev = lineA;
      next = lineB;
    }
    for(int i = 1; i < 53; i++) {
      next[i] = prev[i-1] + prev[i];
    }
    printLine(next, j, lines);
    swap = !swap;
  }
  for(int i = 0; i < 52; i++) {
    putchar('=');
  }
  
  return;
}
