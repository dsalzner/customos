/*
CustomOS
Copyright (C) 2023 D.Salzner <mail@dennissalzner.de>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file tinycc_interpreter.c
 * @brief C-Code Interpreter
 *
 * Loads, compiles and runs a C-Code file given by filename using the TinyCC compiler.
 *
*/

#include "tinycc_compat.h"

char exampleApplication[] =
"void* (*putchar)(char);\n"
"\n"
"const char * name() {\n"
"    return \"Example Application\";\n"
"}\n"
"\n"
"void setPutCharCallback(void* (*putchar_)(char)) {\n"
"  putchar = putchar_;\n"
"}\n"
"\n"
"void init() {\n"
"  char c[] = \"Hello from example application\";\n"
"  for(int i = 0; i < 30; i++) {\n"
"    putchar(c[i]);\n"
"  }\n"
"  return;\n"
"}\n";

void tinyccRunExample() {
  tinycc_run_code(exampleApplication);
}

void tinyccRunCode(const char * filename) {
  char buffer[4096];
  ataGetFileContents(filename, &buffer, sizeof(buffer) / sizeof(buffer[0]));
  tinycc_run_code(buffer);
}

void tinycc_run_code(const char * program) {
  TCCState *s;

  // -- configure TinyCC
  s = tcc_new();
  if (!s) {
    printf("[E] could not create tcc state\n");
    return;
  }
  tcc_set_output_type(s, TCC_OUTPUT_MEMORY);

  // -- compile and relocate
  if (tcc_compile_string(s, program) == -1) {
    printf("[E] compile failed\n");
    return;
  }
  if (tcc_relocate(s, TCC_RELOCATE_AUTO) < 0) {
    printf("[E] relocate failed\n");
    return;
  }

  // -- get application name
  const char* (*funcName)(void);
  funcName = tcc_get_symbol(s, "name");
  if (!funcName) {
    printf("[E] name()-function not found\n");
    return;
  }

  const char * name = funcName();
  printf("\nrunning \"%s\"\n\n", name);

  // -- set printf callback
  void* (*funcSetPutCharCallback)(void* (*putchar_)(char));
  funcSetPutCharCallback = tcc_get_symbol(s, "setPutCharCallback");
  if (!funcSetPutCharCallback) {
      printf("[I] funcSetPutCharCallback()-function not found\n");
  } else {
    funcSetPutCharCallback(&putchar);
  }

  // -- run init function
  void* (*funcInit)();
  funcInit = tcc_get_symbol(s, "init");
  if (!funcInit) {
    printf("[E] init()-function not found\n");
    return;
  }
  funcInit();

  // -- cleanup
  tcc_delete(s);
  return;
}
