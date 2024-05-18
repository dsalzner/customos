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

/*
colours
1 = blue
2 = green
3 = cyan
4 = red
5 = purple
6 = brown
7 = lightgrey
8 = darkgrey
9 = purple
10 = light green
11 = light blue
12 = light red
13 = pink
14 = yellow
15 = white
16 = black
17 = dark grey
18 = dark grey
*/

#![no_std]
use core::ptr;

mod fontsourcecodepro_light_10pt;
use crate::fontsourcecodepro_light_10pt::FONT_SOURCECODEPRO_LIGHT_10PT_BUFFER;
use crate::fontsourcecodepro_light_10pt::FONT_SOURCECODEPRO_LIGHT_10PT_WIDTH;
use crate::fontsourcecodepro_light_10pt::FONT_SOURCECODEPRO_LIGHT_10PT_HEIGHT;
static FONT_BUFFER : &&[[u8; 14]; 93] = &FONT_SOURCECODEPRO_LIGHT_10PT_BUFFER;
static FONT_WIDTH : &&[u16] = &FONT_SOURCECODEPRO_LIGHT_10PT_WIDTH;
static FONT_HEIGHT : &&[u16] = &FONT_SOURCECODEPRO_LIGHT_10PT_HEIGHT;

const COLOR_RED: u8 = 4;
const COLOR_LIGHT_GREY: u8 = 7;
const COLOR_LIGHT_GREEN: u8 = 10;
const COLOR_DARK_GREY: u8 = 17;
const COLOR_CYAN: u8 = 3;
const COLOR_PINK: u8 = 13;

static mut SHELL_TEXT_POSITION: usize = 0;
static mut SHELL_TEXT_BUFFER: [char; 1000] = [' '; 1000];
static mut SHELL_TEXT_COLOR_BUFFER: [u8; 1000] = [COLOR_RED; 1000];
static mut SHELL_PIXEL_BUFFER: *mut cty::c_char = ptr::null_mut();
static mut SHELL_SCREEN_WIDTH: u16 = 0;
static mut SHELL_SCREEN_HEIGHT: u16 = 0;
static mut SHELL_CHANGED: bool = true;


#[no_mangle]
pub unsafe extern fn memset(s: *mut u8, c: i32, n: usize) -> *mut u8 {
    let mut ii = 0;
    while ii < n {
        *s.offset(ii as isize) = c as u8;
        ii += 1;
    }
    s
}

unsafe fn shellCheckCommand(textBufferPos: usize, command: &str) -> bool {
    let pos = textBufferPos - command.len();
    if pos < 0 {
        return false;
    }
    if pos > SHELL_TEXT_BUFFER.len() - command.len() {
        return false;
    }
    for i in 0..command.len() {
        if command.as_bytes()[i] as char != SHELL_TEXT_BUFFER[pos + i] {
            return false;
        }
    }
    return true;
}

#[no_mangle]
pub unsafe extern "C" fn shellWriteChar(ch: char, color: u8) {
    if ch == '\n' { // newline
      shellNewLine();
      return;
    }

    if SHELL_TEXT_POSITION > SHELL_TEXT_BUFFER.len() - 1 {
        return;
    }
    if SHELL_TEXT_POSITION < 0 {
      return;
    }
    SHELL_TEXT_BUFFER[SHELL_TEXT_POSITION] = ch;
    SHELL_TEXT_COLOR_BUFFER[SHELL_TEXT_POSITION] = color;
    SHELL_TEXT_POSITION += 1;

    SHELL_CHANGED = true;
}

unsafe fn shellWriteStr(textBufferPos: usize, text: &str, color: u8) {
    for i in 0..text.len() {
      shellWriteChar(text.as_bytes()[i] as char, color);
    }
}

#[no_mangle]
pub unsafe extern "C" fn shellShowPrompt() {
  let prompt: &str = "cOS>";
  shellWriteStr(SHELL_TEXT_POSITION, prompt, COLOR_LIGHT_GREEN);

  // clear rest of line
  /*
  let charsPerLine = (SHELL_SCREEN_WIDTH / FONT_WIDTH[0]) as usize;
  for i in SHELL_TEXT_POSITION..shellGetLineStart()+charsPerLine {
    if i > 0 && i < SHELL_TEXT_BUFFER.len() { // todo: could write on accessor for SHELL_TEXT_BUFFER and not bounds check everywhere
      SHELL_TEXT_BUFFER[i] = ' ';
    }
  }*/

  for i in SHELL_TEXT_POSITION..SHELL_TEXT_BUFFER.len() {
    SHELL_TEXT_BUFFER[i] = ' ';
  }

}

extern "C" {
    fn ataList(path: *const cty::c_char);
    fn ataShowFileContents(path: *const cty::c_char);
}
unsafe fn shellCommand(textBufferPos: usize) {
    if shellCheckCommand(textBufferPos, "ping") {
        shellWriteStr(SHELL_TEXT_POSITION, "\npong\n", COLOR_LIGHT_GREY);
    } else if shellCheckCommand(textBufferPos, "ls") {
        shellNewLine();
        let bytes: &[u8] = "/".as_bytes(); // + b"\0"; // caution: C-char array is not null-terminated
        let name: *const cty::c_char = bytes.as_ptr() as *const i8;
        ataList(name);
    } else if shellCheckCommand(shellGetLineStart() + 6, "rf") { // note: checkCommand expects position after the command, add prompt
      let beforeParam = shellGetLineStart() + 7;
      let afterParam = SHELL_TEXT_POSITION;
      //shellWriteStr(SHELL_TEXT_POSITION, "\nRead: ", COLOR_LIGHT_GREY);
      let mut param: [cty::c_char;255] = [0; 255];
      for i in beforeParam..afterParam {
        if i < SHELL_TEXT_BUFFER.len() && i > 0 && i - beforeParam < 255{
          //shellWriteChar(SHELL_TEXT_BUFFER[i], COLOR_PINK);
          param[i - beforeParam] = SHELL_TEXT_BUFFER[i] as i8;
        }
      }
      shellNewLine();
      let param: *const cty::c_char = param.as_ptr() as *const i8;
      ataShowFileContents(param);
    } else {
      shellNewLine();
    }
    shellShowPrompt();
}

fn shellSetPixel(x: u16, y: u16, color: u8) {
    unsafe {
      ptr::write(SHELL_PIXEL_BUFFER.wrapping_add(usize::from(x + y*SHELL_SCREEN_WIDTH)), color as i8);
    }
}

fn shellRenderWhite(x: u16, y: u16) {
  let charPixelCount = FONT_WIDTH[0] * FONT_HEIGHT[0];
  let mut curx = x;
  let mut cury = y;

  for bitNo in 0..charPixelCount {
    if bitNo % 8 == 0 {
        curx = x;
    }
    if curx - x == FONT_WIDTH[0] {
      cury += 1;
      curx = x;
    }
    // -- clear pixel
    shellSetPixel(curx, cury, COLOR_DARK_GREY);

    // -- advance in row
    curx += 1;
  }
}

fn shellRenderChar(x: u16, y: u16, charToWrite: char, color: u8) {
  if charToWrite == ' ' {
    shellRenderWhite(x,y);
    return
  }

  let charArrId = charToWrite as usize - 33;

  if 92 < charArrId { // -- avoid implicit bounds check
    return
  }
  let charPixelCount = FONT_WIDTH[charArrId] * FONT_HEIGHT[charArrId];

  let mut curx = x;
  let mut cury = y;

  for bitNo in 0..charPixelCount {
    let byteNo = bitNo / 8;
    let bits = FONT_BUFFER[charArrId][byteNo as usize];

    if bitNo % 8 == 0 {
      curx = x;
    }

    // -- advance to next row
    if curx - x == FONT_WIDTH[charArrId] {
      cury += 1;
      curx = x;
    }
    // -- draw pixel if set
    if bits & (1 << (bitNo % 8)) != 0 {
      shellSetPixel(curx, cury, color);
    } else {
      shellSetPixel(curx, cury, COLOR_DARK_GREY);
    }
    // -- advance in row
    curx += 1;
  }
}

pub unsafe fn shellRedrawBorder() {
  for y in 0..SHELL_SCREEN_HEIGHT {
    for x in 0..SHELL_SCREEN_WIDTH {
      if y < 10 { // top border
        shellSetPixel(x, y, COLOR_CYAN);
      }

      if y >= 190 { // bottom border
        shellSetPixel(x, y, COLOR_CYAN);
      }
    }
  }
}

#[no_mangle]
pub unsafe extern "C" fn shellInit(buffer: *mut cty::c_char, w: u16, h: u16) {
  SHELL_PIXEL_BUFFER = buffer;
  SHELL_SCREEN_WIDTH = w;
  SHELL_SCREEN_HEIGHT = h;
  shellRedrawBorder();
}

#[no_mangle]
pub unsafe extern "C" fn shellUpdate() {
  if SHELL_CHANGED {
    let mut x:u16 = 1;
    let mut y:u16 = 10;
    for i in 0..SHELL_TEXT_BUFFER.len() {
      if x >= SHELL_SCREEN_WIDTH - FONT_WIDTH[0] {
        x = 1;
        y += 10;
      }

      shellRenderChar(x, y, SHELL_TEXT_BUFFER[i], SHELL_TEXT_COLOR_BUFFER[i]);

      let charArrId = SHELL_TEXT_BUFFER[i] as usize - 33;
      if charArrId >= 0 && charArrId < FONT_WIDTH.len() {
          x += FONT_WIDTH[charArrId];
      }
      if SHELL_TEXT_BUFFER[i] == ' ' {
          x += FONT_WIDTH[0];
      }
    }

    shellRedrawBorder();
    SHELL_CHANGED = false;
  }
}

pub unsafe fn shellGetLineStart() -> usize {
  let charsPerLine = (SHELL_SCREEN_WIDTH / FONT_WIDTH[0]) as usize;
  if charsPerLine == 0 {
    return 0;
  }
  let currentRow = SHELL_TEXT_POSITION / charsPerLine;
  let lineStart = currentRow  * charsPerLine;
  if lineStart > SHELL_TEXT_BUFFER.len() {
    return 0;
  }
  if lineStart < 0 {
    return 0;
  }
  return lineStart;
}

pub unsafe fn shellNewLine() {
  let charsPerLine = (SHELL_SCREEN_WIDTH / FONT_WIDTH[0]) as usize;
  if charsPerLine == 0 {
    return
  }
  let currentRow = SHELL_TEXT_POSITION / charsPerLine;
  SHELL_TEXT_POSITION = (currentRow + 1) * charsPerLine;

  if currentRow > 16 { // TODO: calculate actual number of lines
    // -- move all up one row
    for i in charsPerLine..SHELL_TEXT_BUFFER.len() {
      SHELL_TEXT_BUFFER[i - charsPerLine] = SHELL_TEXT_BUFFER[i];
      SHELL_TEXT_COLOR_BUFFER[i - charsPerLine] = SHELL_TEXT_COLOR_BUFFER[i];
    }
    // -- set new cursor position
    SHELL_TEXT_POSITION = (currentRow) * charsPerLine;
    // -- signal buffer changed
    SHELL_CHANGED = true;
  }
}

#[no_mangle]
pub unsafe extern "C" fn shellKeyboardDown(mut key: cty::c_char, scancode: u8) {
  if key as u8 == 10 { // newline
    shellCommand(SHELL_TEXT_POSITION);
    return
  }

  if scancode as u8 == 14 { // backspace
    if SHELL_TEXT_POSITION > 0 {
      SHELL_CHANGED = true;
      SHELL_TEXT_POSITION -= 1;
      if SHELL_TEXT_POSITION < SHELL_TEXT_BUFFER.len() {
        SHELL_TEXT_BUFFER[SHELL_TEXT_POSITION] = ' ';
      }
    }
    return
  }

  // -- swap z <-> y
  if key as u8 as char == 'y' {
    key = 'z' as i8;
  } else if key as u8 as char == 'z' {
    key = 'y' as i8;
  }
  if key as u8 as char == 'Y' {
    key = 'Z' as i8;
  } else if key as u8 as char == 'Z' {
    key = 'Y' as i8;
  }

  shellWriteChar(key as u8 as char, COLOR_LIGHT_GREY);
}
