void* (*putchar)(char);
const char * name() {
  return "Test";
}
void setPutCharCallback(void* putchar_) {
  putchar = putchar_;
}
int log10(unsigned int number) {
  int i = 0;
  int j = 0;
  for(i = 0; i < 5; i++) {
    j *= 10;
    if (number < j) {
      return i;
    }
  }
  return 0;
}
void itoa(unsigned int number, char * str) {
  int i = 0;
  int digits = 0;
  digits = log10(number);
  while(number > 0) {
    str[digits - i + 1] = number % 10 + '0';
    number = number / 10;
    i++;
  }
  str[i] = '\0';
}
void putstr(const char *str) {
  while(*str != '\0') {
    putchar(*str);
    str++;
  }
}
void init() {
  unsigned int i = 0;
  putchar('t');
  int len = log10(243);
  char str1[20];
  itoa(23, str1);
  putstr(str1);
  for(i = 0; i < 20; i++) {
    itoa(i, str1);
    putstr(str1);
    putchar('a');
  }
}