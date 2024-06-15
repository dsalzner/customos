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

const uint8_t TCC_VERBOSE = 0;

#ifdef CUSTOMOS
extern void graphicsSetPixel(uint16_t x, uint16_t y, uint8_t color); // -- graphics.h
extern void graphicsFlush(); // -- graphics.h
#endif

extern void delay(); // -- common.h

uint8_t TCC_ERROR = 0;
volatile uint8_t TCC_RUNNING = 0;
volatile void* (*funcKeyboardDown)(char key, uint8_t scancode);

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
"}";

void tinyccRunExample() {
  tinycc_run_code(exampleApplication);
}

#ifndef CUSTOMOS
void ataGetFileContents(const char * filename, unsigned char * buffer, uint32_t bufferLen) {
  FILE *file;
  file = fopen(filename, "r");

  uint32_t i = 0;
  char c;
  if (file) {
    while ((c = getc(file)) != EOF) {
      putchar(c);
      buffer[i] = c;
      i++;
    }
    fclose(file);
  }
  buffer[i] = '\0';
}

void putchar_linux(char c) {
  printf("%c", c);
}

void graphicsSetPixel(int x, int y, int c) {
  printf("putpixel_linux %d,%d,%d\n", x,y,c);
}

void graphicsFlush() {
  printf("flush\n");
}

void delay() {
  for(int i = 0; i < 1000000; i++) { // accurate time delay would require CPU ClockCycles/USec conversion
    __asm__ __volatile__ ("nop");
  }
}
#endif

void application_keyboard_down(char key, uint8_t scancode) {
  // -- activity indicator in bottom right
  for(int x = 610; x < 630; x++) {
    for(int y = 450; y < 470; y++) {
      graphicsSetPixel(x, y, 4);
    }
  }
  graphicsFlush();

  if(TCC_RUNNING == 1) {
    funcKeyboardDown(key, scancode);
  }
}

void application_exit() {
  TCC_RUNNING = 0;
}

void tinyccRunCode(const char * filename) {
  char buffer[64000];
  ataGetFileContents(filename, &buffer, sizeof(buffer) / sizeof(buffer[0]));
  tinycc_run_code(buffer);
}

void tcc_error_func(void *user, const char *msg) {
  printf("[E] TCC %s\n", msg);
  #ifndef CUSTOMOS
  fflush(NULL);
  #else
  graphicsFlush();
  #endif
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
  tcc_set_error_func(s, 0, &tcc_error_func);
  
  TCC_ERROR = 0;

  // -- compile and relocate
  if (tcc_compile_string(s, program) == -1) {
    printf("[E] compile failed\n");
    return;
  }

  if(TCC_ERROR == 1) {
    printf("[E] tinycc_interpreter.c::tinycc_run_code - not running\n");
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
    if(TCC_VERBOSE == 1) printf("[I] setPutCharCallback()-function not found\n");
  } else {
    #ifndef CUSTOMOS
      funcSetPutCharCallback(&putchar_linux);
    #else
      funcSetPutCharCallback(&putchar);
    #endif
  }

  // -- set putpixel callback
  void* (*funcSetGraphicsSetPixelCallback)(void* (*)(uint16_t, uint16_t, uint8_t));
  funcSetGraphicsSetPixelCallback = tcc_get_symbol(s, "setGraphicsSetPixelCallback");
  if (!funcSetGraphicsSetPixelCallback) {
    if(TCC_VERBOSE == 1) printf("[I] setGraphicsSetPixelCallback()-function not found\n");
  } else {
    funcSetGraphicsSetPixelCallback(&graphicsSetPixel);
  }

  // -- set flush callback
  void* (*funcSetGraphicsFlushCallback)(void* (*)());
  funcSetGraphicsFlushCallback = tcc_get_symbol(s, "setGraphicsFlushCallback");
  if (!funcSetGraphicsFlushCallback) {
    if(TCC_VERBOSE == 1) printf("[I] setGraphicsFlushCallback()-function not found\n");
  } else {
    funcSetGraphicsFlushCallback(&graphicsFlush);
  }

  // -- get keyboard down
  funcKeyboardDown = tcc_get_symbol(s, "keyboardDown");
  if (!funcKeyboardDown) {
    if(TCC_VERBOSE == 1) printf("[I] keyboardDown()-function not found\n");
  }

  // -- set exit callback
  void* (*funcSetExitCallback)(void* (*)());
  funcSetExitCallback = tcc_get_symbol(s, "setExitCallback");
  if (!funcSetExitCallback) {
    if(TCC_VERBOSE == 1) printf("[I] setExitCallback()-function not found\n");
  } else {
    funcSetExitCallback(&application_exit);
  }

  // -- run init function   
  void* (*funcInit)();
  funcInit = tcc_get_symbol(s, "init");
  if (!funcInit) {
    printf("[E] init()-function not found\n");
    return;
  }
  funcInit();

  // -- run loop function
  // (note: tinycc disables interrupts blocking the keyboard (?), but graphicsFlush reenables them)
  void* (*funcUpdate)();
  funcUpdate = tcc_get_symbol(s, "update");
  if (!funcUpdate) {
    if(TCC_VERBOSE == 1) printf("[I] update()-function not found\n");
  } else {
    TCC_RUNNING = 1;
    while(TCC_RUNNING == 1) {
      funcUpdate();
      delay();
    }
  }

  // -- cleanup
  tcc_delete(s);
  return;
}
