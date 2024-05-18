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

#define IRQ(x) x+32 // also in isrs.h

#include "common.h"
#include "terminal.h"
#include "gdt.h"
#include "idt.h"
#include "isrs.h"

#include "keyboard/keyboardScancodeToAscii.h"
#include "keyboard/keyboardActivePolling.h"

#if defined(__cplusplus)
extern "C"
#endif

#define IRQ(x) x+32 // also in isrs.h

void kb_interrupt(struct regs *r) {
  terminal_writestring("KEY");
}

void kernel_main() {
  gdt_install();
  idt_install();
  isrs_install();
  terminal_initialize();
  register_interrupt_handler(IRQ(1), &kb_interrupt);

  terminal_writestring("Hello, kernel World!\n");
  keyboardActivePolling_init();

  while(true) {
    uint8_t scancode = 0;
    scancode = keyboardActivePolling_getScancode(); // required otherwise the keyboard event fires only once !?

    delay();
    terminal_putchar('.');
  }

}
