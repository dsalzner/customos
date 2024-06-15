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
 * @file kernel.c
 * @brief Kernel Entry-Point
 *
 * Initializes Terminal, Interrupts, Ramdisk and Keyboard
 *
 * boot sequence
 * - Bootloader Grub loads grub.cfg
 * - grub.cfg loads myos.bin
 * - myos.bin is linked following structure defined in linker.ld to set adress of boot-section
 * - boot.asm contains boot-section, loads kernel_main function here
*/

#include "common.h"
#include "gdt.h"
#include "idt.h"
#include "isrs.h"
#include "timer.h"

#if(INIT_RAMDISK == 1)
  #include "ramdisk.h"
#endif

#include "keyboard.h"
#include "printf.h"

#include "ata.h"

#include "graphics.h"

#if defined(__cplusplus)
extern "C"
#endif

char cmdBuffer[120] = {};

extern void shell_keyboard_down(char key, uint8_t scancode); // -- shell/shell.rs
extern void application_keyboard_down(char key, uint8_t scancode); // -- tinycc_interpreter.c
static void keyboardKeyDown(char key, uint8_t scancode) {
  application_keyboard_down(key, scancode);

  if (TEXT_MODE_FROM_C == 1) {
    int8_t size = sizeof(cmdBuffer) / sizeof(cmdBuffer[0]);
    int8_t pos = strlen(cmdBuffer);

    // -- backspace
    if (scancode == 14) {
      if(terminal_column > 0) {
        terminal_column--;
        terminal_putchar(' ');
        terminal_column--;
      }
      if(pos > 0) {
        cmdBuffer[--pos] = '\0';
      }
      return;
    }

    // -- enter
    if (scancode == 28) {
      if(pos > 0) {
        printf("\n> %s\n", cmdBuffer);
      }
      cmdBuffer[0] = '\0';
      return;
    }

    terminal_putchar(key);
    if (pos < size - 1) {
      cmdBuffer[pos++] = key;
      cmdBuffer[pos] = '\0';
    }
  } else {
    // -- call rust
    shell_keyboard_down(key, scancode);
  }
}

static void keyboardKeyUp(char key, uint8_t scancode) {
  UNUSED(key);
  UNUSED(scancode);
}

int counterRead();
void counterWrite(int counterValue);

extern void shell_update(); // -- in shell/shell.rs
extern void shell_init(uint16_t w, uint16_t h); // -- in shell/shell.rs
extern void shell_show_prompt(); // -- in shell/shell.rs
typedef struct multiboot_info multiboot_info_t;
void kernel_main(multiboot_info_t* info) {
  UNUSED(info);
  if(TEXT_MODE_FROM_C == 0) {
    graphicsInit(); // note: we initialize graphics early so line-wrapping will work
    shell_init(screenWidth, screenHeight); // screenBuffer
  } else {
    terminal_initialize();
  }
  printf("Booting...\n");

  gdt_install();
  idt_install();
  isrs_install();

  #if(INIT_RAMDISK == 1)
    ramdiskInit(info);
    ramdiskList("");
  #endif

  keyboardInit(false);

  ataInit();

  if (GRAPHICS_MODE_FROM_C == 1) {
    ataList("");
    ataShowFileContents("testfile.txt");

    uint8_t counterValue = counterRead();
    printf("\n[ ] counter value: %u\n", counterValue);
    counterWrite(++counterValue);
  } else {
    shell_show_prompt();
  }

  //timerInit(1000);

  while(true) {
    if (GRAPHICS_MODE_FROM_C == 0) {
      // -- call Rust
      shell_update();
    }
    delay();
  }
}

extern int atoi(uint8_t *s); // -- in lib/tinycc_compat.c
int counterRead() {
  char * fileName = "counter.txt";
  FIL file;
  FRESULT res;
  res = f_open(&file, fileName, FA_READ);
  if (res != FR_OK) {
    printf("failed to open file %s. error=%u\n", fileName, res);
    return 0;
  }
  uint32_t fileSize = f_size(&file);
  printf("fileName %s, fileSize %d\n", fileName, fileSize);
  uint8_t buffer[10];
  UINT br;
  res = f_read(&file, &buffer, fileSize, &br);
  if (res != FR_OK) {
      printf("failed to read from file %s. error=%u\n", fileName, res);
      return 0;
  }
  f_close(&file);
  buffer[fileSize] = '\0';
  return atoi(buffer);
}

void counterWrite(int counterValue) {
  char * fileName = "counter.txt";
  FIL file;
  FRESULT res;
  // ---
  // note: fatfs is strict about flags
  // - FA_WRITE - only writes, if the file already exists
  // - FA_CREATE_NEW | FA_WRITE - only writes, if the file doesn't already exist
  // - FA_CREATE_ALWAYS | FA_WRITE - will write always creating, if the file doesn't exist
  // ---
  res = f_open(&file, fileName, FA_CREATE_ALWAYS | FA_WRITE);
  if (res != FR_OK) {
    printf("failed to open file %s. error=%u\n", fileName, res);
    return;
  }
  char buffer [10];
  sprintf(buffer, "%d", counterValue);
  uint32_t bufferLen = sizeof(buffer) / sizeof(buffer[0]) * 8; // needs to be multiple of 8
  UINT br;
  res = f_write(&file, (const void*)buffer, (UINT)bufferLen, &br);
  if (res != FR_OK) {
      printf("failed to write file %s. error=%u\n", fileName, res);
      return;
  }
  f_close(&file);
}
