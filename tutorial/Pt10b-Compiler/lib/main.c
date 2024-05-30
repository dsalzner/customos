#include "tinycc_compat.h"
#include "tinycc_interpreter.h"

extern char theprog[];

int main() {
  tinycc_run_code(theprog);
  return 0;
}
