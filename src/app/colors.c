/*
CustomOS
Copyright (C) 2024 D.Salzner <mail@dennissalzner.de>

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
 * @file colors.c
 * @brief Displays the 4-bit color palette
 *
 *
*/

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

void* (*graphicsSetPixel)(uint16_t, uint16_t, uint8_t);
void* (*graphicsFlush)(void);
void* (*exit)(void);

void setGraphicsSetPixelCallback(void* (*graphicsSetPixel_)(uint16_t, uint16_t, uint8_t)) {
  graphicsSetPixel = graphicsSetPixel_;
}

void setGraphicsFlushCallback(void* (*graphicsFlush_)(void)) {
  graphicsFlush = graphicsFlush_;
}

void setExitCallback(void* (*exit_)(void)) {
  exit = exit_;
}

const char * name() {
  return "Colors";
}

void keyboardDown(char key, uint8_t scancode) {
  if(scancode == 01) exit(); // escape
}

uint16_t loop = 0;
uint8_t color = 0;
uint16_t xpos = 0;
uint16_t ypos = 20;

void update() {
  loop++;
  if(loop % 200 == 0) {
    if(color > 16) {
      return;
    }

    for(int x = 0; x < 120; x++) {
      for(int y = 0; y < 120; y++) {
	graphicsSetPixel(xpos + x, ypos + y, color);
      }
    }
    graphicsFlush();

    xpos += 128;
    if (xpos > 640) {
      ypos += 128;
      xpos = 0;
    }
    color++;
  }
}

void init() { }
