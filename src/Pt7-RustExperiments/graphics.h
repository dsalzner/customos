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
 * @file graphics.h
 * @brief Graphics Buffer Interaction
 *
 * Fills a raw pixel buffer, renders pre-rendered text to it
 *
*/

#pragma once

#include "graphics/fontRobotoMonoMedium10pt.h"

typedef struct __attribute__ ((packed)) {
  unsigned short di, si, bp, sp, bx, dx, cx, ax;
  unsigned short gs, fs, es, ds, eflags;
} regs16_t;

extern void int32(unsigned char intnum, regs16_t *regs);

uint16_t screenWidth = 320;
uint16_t screenHeight = 200;

char * screenBuffer = NULL;

uint16_t fontWidth(uint8_t fontNo, uint16_t charArrId) {
  if(fontNo == 0) return fontRobotoMonoMedium10pt_width[charArrId];
  return 0;
}

uint16_t fontHeight(uint8_t fontNo, uint16_t charArrId) {
  if(fontNo == 0) return fontRobotoMonoMedium10pt_height[charArrId];
  return 0;
}

char * fontBuffer(uint8_t fontNo, uint16_t charArrId) {
  if(fontNo == 0) return fontRobotoMonoMedium10pt_buffer[charArrId];
  return 0;
}

void graphicsRenderChar(char * graphicsBuffer, uint16_t x, uint16_t y, char charToWrite, uint8_t fontNo)  {
  uint16_t curx = x;
  uint16_t cury = y;

  uint16_t charArrId = charToWrite - 33;

  uint16_t charPixelCount = fontWidth(fontNo, charArrId) * fontHeight(fontNo, charArrId);
  for(uint16_t byteNo = 0; byteNo < charPixelCount / 8; byteNo++) {

    char bits = fontBuffer(fontNo, charArrId)[byteNo];
    for(int bit = 0; bit < 8; bit++) {

      // -- start new row when width is reached
      if(curx - x == fontWidth(fontNo, charArrId)) {
        cury += 1;
        curx = x;
      }
      // -- draw pixel if bit is set
      if(bits & 0x01) {
        uint16_t pos = cury * screenWidth + curx;
        graphicsBuffer[pos] = 1; // 1=blue
      }

      // -- shift to next bit
      bits = bits >> 1;

      // -- advance the row
      curx += 1;
    }
  }
}

void graphicsInit() {
  int y;
  regs16_t regs;

  regs.ax = 0x0013;
  int32(0x10, &regs);

  // -- fill screen
  memset((char *)0xA0000, 1, (screenHeight*screenWidth));
  screenBuffer = (char *)0xA0000;

  // -- draw color bands
  int pos = 0;
  for(int y = 0; y < screenHeight; y++) {
    for(int x = 0; x < screenWidth; x++) {
      pos = y * screenWidth + x;
      screenBuffer[pos] = 3; // 3=cyan
    }
  }

  /*
  // -- write text
  graphicsRenderChar(VGA, 8, 10, 'H', 0);
  graphicsRenderChar(VGA, 16, 10, 'e', 0);
  graphicsRenderChar(VGA, 24, 10, 'l', 0);
  graphicsRenderChar(VGA, 32, 10, 'l', 0);
  graphicsRenderChar(VGA, 40, 10, 'o', 0);
  */

  // -- call rust
  graphicsUpdate(screenBuffer, screenWidth, screenHeight);

  // -- wait for key
  regs.ax = 0x0000;
  int32(0x16, &regs);

  // -- switch to 80x25x16 text mode
  regs.ax = 0x0003;
  int32(0x10, &regs);
}
