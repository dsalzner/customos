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
 * @file graphics-linux.h
 * @brief Single-Header to read keyboard input for Linux
 *
 * Serves as a compatibility layer to test CustomOS applications on Linux.
 *
 * @see https://github.com/cntools/rawdraw/blob/a53ea30df86f5876197873975efee3dbc983a942/CNFGXDriver.c#L407
 *
*/

#pragma once

struct SInput {
  uint16_t mouseX;
  uint16_t mouseY;
  int mouseButtons;
  int g_x_global_key_state;
  int g_x_global_shift_key;

  int keyboardKeycode;
  int keyboardDown;
} m_input;

void inputInit() { }

void inputLoop() {
  if(!m_graphics.window) {
    printf("ERR input no window\n");
    return;
  }

  XSelectInput (m_graphics.display, m_graphics.window, KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | ExposureMask | PointerMotionMask );

  static int ButtonsDown;
  XEvent report;

  int bKeyDirection = 1;
  while(XPending(m_graphics.display)) {
    XNextEvent( m_graphics.display, &report );
    bKeyDirection = 1;
    switch  (report.type) {
      case NoExpose:
	break;
      case Expose:
	break;
      case KeyRelease: {
	bKeyDirection = 0;
	if(XPending(m_graphics.display)) {
	  XEvent nev;
	  XPeekEvent(m_graphics.display, &nev );
	  if (nev.type == KeyPress && nev.xkey.time == report.xkey.time && nev.xkey.keycode == report.xkey.keycode )
	    bKeyDirection = 2;
	}
      }
      case KeyPress: {
	m_input.g_x_global_key_state = report.xkey.state;
	m_input.g_x_global_shift_key = XLookupKeysym(&report.xkey, 1);

	m_input.keyboardKeycode = report.xkey.keycode; // XLookupKeysym(&report.xkey, 0);
	m_input.keyboardDown = bKeyDirection;
	break;
      case ButtonRelease:
	bKeyDirection = 0;
      case ButtonPress:
	m_input.mouseX = report.xbutton.x;
	m_input.mouseY = report.xbutton.y;
	ButtonsDown = (ButtonsDown & (~(1<<report.xbutton.button))) | ( bKeyDirection << report.xbutton.button );
      case MotionNotify:
	m_input.mouseX = report.xmotion.x;
	m_input.mouseY = report.xmotion.y;
	m_input.mouseButtons = ButtonsDown>>1;
	break;
      case ClientMessage:
	exit( 0 );
	break;
      default:
	break;
      }
    }
  }
}
