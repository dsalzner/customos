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

#include "common.h"
#include "graphics/fontRobotoMonoMedium10pt.h"

typedef struct __attribute__ ((packed)) {
  unsigned short di, si, bp, sp, bx, dx, cx, ax;
  unsigned short gs, fs, es, ds, eflags;
} regs16_t;

extern void int32(unsigned char intnum, regs16_t *regs);

uint16_t screenWidth = 640;
uint16_t screenHeight = 480;

char screenBlue[640*480/8];
char screenGreen[640*480/8];
char screenRed[640*480/8];
char screenGrey[640*480/8];

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

void select_plane(uint8_t plane) {
  asm volatile("cli");
  outportb(0x3C4, 0x02);
  outportb(0x3C5, (uint8_t)1 << plane);

  outportb(0x3CE, 0x04);
  outportb(0x3CF, plane);
  asm volatile("sti");
}

void graphicsSetPixelInternal(uint16_t x, uint16_t y, uint8_t color) {
  if (x >= screenWidth || y >= screenHeight)
    return;

  // -- update internal buffer
  /*
  0 = blue
  1 = green
  2 = red
  3 = grey

  mixing:
  - if 0,1,2 are set => white
  - if 1,2 are set => yellow
  - if 0,1 are set => cyan
  */
  color = color % 16; // only 4 bits allowed per pixel

  char * currentScreenPlane = NULL;
  unsigned int bytePos = (640 / 8) * y + x / 8;
  unsigned int bitPos = 0x80 >> (x % 8);
  unsigned int colorPlaneMask = 1;

  bool changed = false;

  for(unsigned int p = 0; p < 4; p++) {
    if(p == 0) currentScreenPlane = screenBlue;
    if(p == 1) currentScreenPlane = screenGreen;
    if(p == 2) currentScreenPlane = screenRed;
    if(p == 3) currentScreenPlane = screenGrey;

    char oldValue = currentScreenPlane[bytePos];

    if(colorPlaneMask & color) { // -- bit is set for current plane
      currentScreenPlane[bytePos] = oldValue | bitPos; // set
    } else {
      currentScreenPlane[bytePos] = oldValue & ~bitPos; // unset
    }

    if(currentScreenPlane[bytePos] != oldValue) {
      changed = true;
    }

    colorPlaneMask <<= 1;
  }

  // -- transfer to graphics buffer
  if(changed) {
    char * graphicsBuffer = (char *)0xA000;
    unsigned int graphicsBufferBytePos = 40960 * 15 + bytePos; // -- graphics buffer is at end of 64k
    colorPlaneMask = 1;

    for(unsigned int p = 0; p < 4; p++) {
      select_plane(p);
      if(p == 0) currentScreenPlane = screenBlue;
      if(p == 1) currentScreenPlane = screenGreen;
      if(p == 2) currentScreenPlane = screenRed;
      if(p == 3) currentScreenPlane = screenGrey;

      graphicsBuffer[graphicsBufferBytePos] = currentScreenPlane[bytePos];
    }
  }
}

void graphicsSetPixel(uint16_t x, uint16_t y, uint8_t color) {
  graphicsSetPixelInternal(x, y, color);
}

void graphicsRenderChar(uint16_t x, uint16_t y, char charToWrite, uint8_t fontNo)  {
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
	graphicsSetPixel(x, y, 1);
      }

      // -- shift to next bit
      bits = bits >> 1;

      // -- advance the row
      curx += 1;
    }
  }
}

void graphicsInit() {
  regs16_t regs;
  regs.ax = 0x0012; // mode 13h = 0x0013 = 320x200x8bpp, mode 12h = 0x0012 = 640x480x4bpp
  int32(0x10, &regs);

  // -- clear screen buffers
  for(uint16_t i = 0; i < 640*480/8; i++) {
    screenBlue[i] = 0;
    screenGreen[i] = 0;
    screenRed[i] = 0;
    screenGrey[i] = 0;
  }

  if (GRAPHICS_MODE_FROM_C == 1) {
    // -- write text
    graphicsRenderChar(8, 10, 'H', 0);
    graphicsRenderChar(16, 10, 'e', 0);
    graphicsRenderChar(24, 10, 'l', 0);
    graphicsRenderChar(32, 10, 'l', 0);
    graphicsRenderChar(40, 10, 'o', 0);

    // -- wait for key
    regs.ax = 0x0000;
    int32(0x16, &regs);

    // -- switch to 80x25x16 text mode
    regs.ax = 0x0003;
    int32(0x10, &regs);
  }
}
