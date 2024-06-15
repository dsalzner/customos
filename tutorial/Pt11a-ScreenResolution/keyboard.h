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
 * @file keyboard.h
 * @brief Keyboard Configuration for Custom OS
 *
 * Single-Header for intializing the Keyboard, active-polling or interrupt-driven, scan-code conversion
 *
 * @see https://github.com/tehplague/baremetal/blob/master/keyboard.c
 * @see https://stackoverflow.com/questions/61124564/convert-scancodes-to-ascii
*/

#pragma once
#define KBC_STATUS   0x64
#define KBC_EA       0x60
#define IRQ(x) x+32 // also in isrs.h

/**
 * @brief function stub for keyboard keys
*/
static void keyboardKeyDown(char key, uint8_t scancode);
static void keyboardKeyUp(char key, uint8_t scancode);

char keyboardScanCodeUs[128] = {
  0,   27,   '1', '2',  '3',  '4',  '5',  '6', '7',
  '8', '9',  '0', '-',  '=',  '\b', '\t', /* <-- Tab */
  'q', 'w',  'e', 'r',  't',  'y',  'u',  'i', 'o',
  'p', '[',  ']', '\n', 0, /* <-- control key */
  'a', 's',  'd', 'f',  'g',  'h',  'j',  'k', 'l',
  ';', '\'', '`', 0,    '\\', 'z',  'x',  'c', 'v',
  'b', 'n',  'm', ',',  '.',  '/',  0,    '*', 0, /* Alt */
  ' ',                                            /* Space bar */
  0,                                              /* Caps lock */
  0,                                              /* 59 - F1 key ... > */
  0,   0,    0,   0,    0,    0,    0,    0,   0, /* < ... F10 */
  0,                                              /* 69 - Num lock*/
  0,                                              /* Scroll Lock */
  0,                                              /* Home key */
  0,                                              /* Up Arrow */
  0,                                              /* Page Up */
  '-', 0,                                         /* Left Arrow */
  0,   0,                                         /* Right Arrow */
  '+', 0,                                         /* 79 - End key*/
  0,                                              /* Down Arrow */
  0,                                              /* Page Down */
  0,                                              /* Insert Key */
  0,                                              /* Delete Key */
  0,   0,    '<',   0,                              /* F11 Key */
  0,                                              /* F12 Key */
  0, /* All other keys are undefined */
};

unsigned char keyboardScanCodeUsShift[128] = {
  0,    27,  '!',  '\"', '#',  0 /* shift+4 */,
  '%',  '&', '/',  '(',        /* 9 */
  ')',  '=', '?',  '`',  '\b', /* Backspace */
  '\t',                        /* Tab */

  'Q',  'W', 'E',  'R', /* 19 */
  'T',  'Y', 'U',  'I',  'O',  'P',
  '{',  '}', '\n', /* Enter key */
  0,               /* 29   - Control */
  'A',  'S', 'D',  'F',  'G',  'H',
  'J',  'K', 'L',  'O', /* 39 */
  '\'', '>', 0,         /* Left shift */
  '*',  'Z', 'X',  'C',  'V',  'B',
  'N',                      /* 49 */
  'M',  ';', ':',  '_',  0, /* Right shift */

  '*',  0, /* Alt */
  ' ',     /* Space bar */
  0,       /* Caps lock */
  0,       /* 59 - F1 key ... > */
  0,    0,   0,    0,    0,    0,
  0,    0,   0,       /* < ... F10 */
  0,                  /* 69 - Num lock*/
  0,                  /* Scroll Lock */
  0,                  /* Home key */
  0,                  /* Up Arrow */
  0,                  /* Page Up */
  '-',  0,            /* Left Arrow */
  0,    0,            /* Right Arrow */
  '+',  0,            /* 79 - End key*/
  0,                  /* Down Arrow */
  0,                  /* Page Down */
  0,                  /* Insert Key */
  0,                  /* Delete Key */
  0,    0,   '>',  0, /* F11 Key */
  0,                  /* F12 Key */
  0,                  /* All other keys are undefined */
};

unsigned char keyboardScanCodeUsShiftAlt[128] = {
  0,    27,  0 /*alt+1*/, '@', 0,    '$', 0,   0,   '{',  '[', /* 9 */
  ']',  '}', '\\',        '=', '\b',                           /* Backspace */
  '\t',                                                        /* Tab */
  'q',  'w', 'e',         'r',                                 /* 19 */
  't',  'y', 'u',         'i', 'o',  'p', '[', ']', '\n',      /* Enter key */
  0, /* 29   - Control */
  'a',  's', 'd',         'f', 'g',  'h', 'j', 'k', 'l',  ';', /* 39 */
  '\'', '`', 0,                                        /* Left shift */
  '\\', 'z', 'x',         'c', 'v',  'b', 'n',         /* 49 */
  'm',  ',', '.',         '/', 0,                      /* Right shift */
  '*',  0,                                             /* Alt */
  ' ',                                                 /* Space bar */
  0,                                                   /* Caps lock */
  0,                                                   /* 59 - F1 key ... > */
  0,    0,   0,           0,   0,    0,   0,   0,   0, /* < ... F10 */
  0,                                                   /* 69 - Num lock*/
  0,                                                   /* Scroll Lock */
  0,                                                   /* Home key */
  0,                                                   /* Up Arrow */
  0,                                                   /* Page Up */
  '-',  0,                                             /* Left Arrow */
  0,    0,                                             /* Right Arrow */
  '+',  0,                                             /* 79 - End key*/
  0,                                                   /* Down Arrow */
  0,                                                   /* Page Down */
  0,                                                   /* Insert Key */
  0,                                                   /* Delete Key */
  0,    0,   '|',         0,                           /* F11 Key */
  0,                                                   /* F12 Key */
  0, /* All other keys are undefined */
};

static uint8_t _keyboardGetScancode() {
  uint8_t scancode = 0;
  do {
      scancode = inportb(0x64);
  } while ((scancode & 0x01) == 0);
  scancode = inportb(0x60);
  return scancode;
}

static bool shift;

static void _keyboardInterrupt(struct regs * r) {
  UNUSED(r);

  uint8_t scancode = _keyboardGetScancode();

  bool down = false;
  if (scancode >> 7 == 0) {
    down = true;
  }

  if ((scancode & 0b01111111) == 42 || (scancode & 0b01111111) == 54) {
    shift = down;
    return;
  }

  char key;
  if(shift) {
    key = keyboardScanCodeUsShift[scancode];
  } else {
    key = keyboardScanCodeUs[scancode];
  }
  if(down) {
    keyboardKeyDown(key, scancode);
  } else {
    keyboardKeyUp(key, scancode);
  }
}

void _keyboardSendCommand(uint8_t command) {
  while ((inportb(KBC_STATUS) & 2)) {} // wait for buffer free
  outportb(KBC_EA, command);
}

static void keyboardInit(bool noRegisterInterrupt) {
  if (!noRegisterInterrupt) {
    register_interrupt_handler(IRQ(1), &_keyboardInterrupt);
  }

  // empty keyboard buffer
  while (inportb(KBC_STATUS) & 1) {
    inportb(KBC_EA);
  }

  // activate keyboard
  _keyboardSendCommand(0xF4);
  while (inportb(KBC_STATUS) & 1) {
    inportb(KBC_EA); // read (and drop) what's left in the keyboard buffer
  }

  // self-test (should answer with 0xEE)
  _keyboardSendCommand(0xEE);
}

/*
static uint8_t keyboardPoll() {
  uint8_t scancode = 0;
  while ((scancode = _keyboardGetScancode()) == 0)  { // loop until we get a keypress
    delay();
  }
  return scancode;
}
*/
