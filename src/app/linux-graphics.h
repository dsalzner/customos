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
 * @file linux-graphics.h
 * @brief Single-Header to draw on screen for Linux
 *
 * Works on X11 and Wayland.
 * Serves as a compatibility layer to test CustomOS applications on Linux.
 *
*/

#pragma once

#include <stdio.h>
#include <stdlib.h>  // atexit
#include <stdint.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>

struct SGraphics {
  Display * display;
  Visual * visual;
  Window window;
  GC graphics;
  XWindowAttributes attributes;
  GC windowgraphics;
  Pixmap pixmap;
  XImage * image;
  int width;
  int height;
  int depth;
  int screen;
  uint32_t * bufferRGBA8888;
} m_graphics;

void graphicsClose() {
  if(m_graphics.image) free(m_graphics.image);
  if(m_graphics.graphics) XFreeGC( m_graphics.display, m_graphics.graphics);
  if(m_graphics.display) XCloseDisplay( m_graphics.display );
  if(m_graphics.bufferRGBA8888) free(m_graphics.bufferRGBA8888);
  m_graphics.bufferRGBA8888 = NULL;
  m_graphics.display = NULL;
  m_graphics.graphics = NULL;
}

void graphicsLoop() {
  XMapWindow(m_graphics.display, m_graphics.window);
  m_graphics.image = XCreateImage(m_graphics.display, m_graphics.visual, m_graphics.depth, ZPixmap, 0, (char*)m_graphics.bufferRGBA8888, m_graphics.width, m_graphics.height, 32, m_graphics.width*4);
  XPutImage(m_graphics.display, m_graphics.window, m_graphics.windowgraphics, m_graphics.image, 0, 0, 0, 0, m_graphics.width, m_graphics.height);
}

void graphicsInit() {
  m_graphics.width = 640;
  m_graphics.height = 480;

  // -- open display
  m_graphics.display = XOpenDisplay(NULL);
  if(m_graphics.display == NULL) {
    printf("ERR opening Display\n");
    exit(1);
  }

  // -- set close handler
  atexit(graphicsClose);

  // -- get properties
  m_graphics.screen = DefaultScreen(m_graphics.display);
  m_graphics.depth = DefaultDepth(m_graphics.display, m_graphics.screen);
   m_graphics.visual = DefaultVisual(m_graphics.display, m_graphics.screen);
  Window window = DefaultRootWindow(m_graphics.display);

  XSetWindowAttributes attr;
  attr.background_pixel = 0;
  attr.colormap = XCreateColormap(m_graphics.display, window, m_graphics.visual, AllocNone);

  // -- create window
  m_graphics.window = XCreateWindow(m_graphics.display, window, 1, 1, m_graphics.width, m_graphics.height, 0, m_graphics.depth, InputOutput, m_graphics.visual, CWBackPixel | CWColormap, &attr );

  // -- set window title
  XSetStandardProperties(m_graphics.display, m_graphics.window, "Test Window", 0, 0, 0, 0, 0 );

  // -- flush
  XGetWindowAttributes(m_graphics.display, m_graphics.window, &m_graphics.attributes);
  XFlush(m_graphics.display);

  // -- create graphics context and pixmap
  m_graphics.graphics = XCreateGC(m_graphics.display, m_graphics.window, 0, 0);
  m_graphics.pixmap = XCreatePixmap(m_graphics.display, m_graphics.window, m_graphics.attributes.width, m_graphics.attributes.height, m_graphics.attributes.depth);
  m_graphics.windowgraphics = XCreateGC(m_graphics.display, m_graphics.pixmap, 0, 0);
  XSetLineAttributes(m_graphics.display, m_graphics.graphics, 1, LineSolid, CapRound, JoinRound);

  // -- map window
  XMapWindow(m_graphics.display, m_graphics.window);

  // -- reserve space for graphicsbuffer
  m_graphics.bufferRGBA8888 = (uint32_t *)malloc(m_graphics.width*m_graphics.height * sizeof(uint32_t));
  if(!m_graphics.bufferRGBA8888) {
    printf("[ERR] Reserving Memory for graphics Buffer\n");
    exit(1);
  }

  graphicsLoop();
}

void graphicsSetPixel32Bit(uint16_t x, uint16_t y, uint32_t color32bit) {
  if(x > m_graphics.width)
    return;
  if(y > m_graphics.height)
    return;
  m_graphics.bufferRGBA8888[x + y * m_graphics.width] = color32bit;
}

void graphicsSetPixel(uint16_t x, uint16_t y, uint8_t color) {
// -- QEmu Vesa 4 Bit colours

  uint32_t color32bit = 0;

  if(color ==  0) color32bit = 0x000038; // black
  if(color ==  1) color32bit = 0x0000a8; // blue
  if(color ==  2) color32bit = 0x009b0c; // green
  if(color ==  3) color32bit = 0x008aa8; // cyan / teal

  if(color ==  4) color32bit = 0xa80000; // red
  if(color ==  5) color32bit = 0xa300a8; // purple / magenta / violet
  if(color ==  6) color32bit = 0x9e5209; // brown / orange
  if(color ==  7) color32bit = 0x9393a8; // light grey

  if(color ==  8) color32bit = 0x53535a; // dark grey
  if(color ==  9) color32bit = 0x5252fa; // teal / light blue
  if(color == 10) color32bit = 0x52f15b; // light green
  if(color == 11) color32bit = 0x53f3fb; // light blue

  if(color == 12) color32bit = 0xfa5558; // light red / orange / maroon
  if(color == 13) color32bit = 0xe94ff7; // pink
  if(color == 14) color32bit = 0xdede61; // yellow
  if(color == 15) color32bit = 0xffffff; // white

  graphicsSetPixel32Bit(x, y, color32bit);
}
