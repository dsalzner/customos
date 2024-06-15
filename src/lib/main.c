#include "tinycc_compat.h"
#include "tinycc_interpreter.h"

int main(int argc, char* argv[]) {
  if(argc == 1) {
    tinyccRunExample();
  } else {
    tinyccRunCode(argv[1]);
  }
	return 0;
}
