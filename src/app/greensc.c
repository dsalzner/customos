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
 * @file greensc.c
 * @brief Demo application that draws green rectangles that can be moved with the keyboard arrow keys
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
// --

int xPos = 100;
int yPos = 100;
int loop = 0;
int xDir = 10;
int yDir = 0;

const char * name() {
  return "Green";
}

void keyboardDown(char key, uint8_t scancode) {
  if(scancode == 01) exit(); // escape
  xDir = 0;
  yDir = 0;
  if(scancode == 75) xDir = -10; // left
  if(scancode == 72) yDir = -10; // up
  if(scancode == 77) xDir = 10; // right
  if(scancode == 80) yDir = 10; // down
}

void update() {
  loop++;
  if(loop % 100 == 0) {
    xPos += xDir;
    yPos += yDir;
    xPos = xPos % 630;
    yPos = yPos % 470;
    for(int x = 0; x < 10; x++) {
      for(int y = 0; y < 10; y++) {
	graphicsSetPixel(xPos+x, yPos+y, 2);
      }
    }
    graphicsFlush();
  }
}

void init() { }
