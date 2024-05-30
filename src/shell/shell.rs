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

mod editfile;
use crate::editfile::editorSetFilename;

const COLOR_GREEN: u8 = 2;
const COLOR_RED: u8 = 4;
const COLOR_LIGHT_GREY: u8 = 7;
const COLOR_LIGHT_GREEN: u8 = 10;
const COLOR_DARK_GREY: u8 = 17;
const COLOR_CYAN: u8 = 3;
const COLOR_PINK: u8 = 13;
const COLOR_YELLOW: u8 = 14;
const PROMPT: &str = "cOS>";

static mut SHELL_TEXT_POSITION: usize = 0;
static mut SHELL_TEXT_BUFFER: [char; 1000] = [' '; 1000];
static mut SHELL_TEXT_COLOR_BUFFER: [u8; 1000] = [COLOR_RED; 1000];
static mut SHELL_PIXEL_BUFFER: *mut cty::c_char = ptr::null_mut();
pub static mut SHELL_SCREEN_WIDTH: u16 = 0;
pub static mut SHELL_SCREEN_HEIGHT: u16 = 0;
static mut SHELL_CHANGED: bool = true;

extern "C" {
  fn ataList(path: *const cty::c_char);
  fn ataShowFileContents(filename: *const cty::c_char);
  fn tinyccRunCode(filename: *const cty::c_char);
  fn tinyccRunExample();
}

#[derive(PartialEq)]
enum EState {
  SHELL,
  SCANCODE,
  EDITFILE,
}
static mut state: EState = EState::SHELL;

#[no_mangle]
pub unsafe extern fn memset(s: *mut u8, c: i32, n: usize) -> *mut u8 {
  let mut ii = 0;
  while ii < n {
      *s.offset(ii as isize) = c as u8;
      ii += 1;
  }
  s
}

#[no_mangle]
pub unsafe extern fn memcpy(dest: *mut u8, src: *const u8, n: usize) -> *mut u8{
    for i in 0..n {
        *dest.offset(i as isize) = *src.offset(i as isize);
    }
    return dest;
}


#[no_mangle]
pub unsafe extern "C" fn shellWriteChar(ch: char, color: u8) {
  if ch == '\n' { // newline
    shellNewLine();
    return;
  }
  if ch == '\0' {
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

pub unsafe fn shellHeight() -> u16 {
  return SHELL_SCREEN_HEIGHT;
}

pub unsafe fn shellWidth() -> u16 {
  return SHELL_SCREEN_WIDTH;
}

pub unsafe fn shellFontWidth() -> u16 {
  return FONT_WIDTH[0];
}

pub unsafe fn shellFontHeight() -> u16 {
  return FONT_HEIGHT[0];
}

pub unsafe fn shellSetActive() {
  shellNewLine();
  state = EState::SHELL;  
}

unsafe fn shellWriteStr(text: &str, color: u8) {
  for i in 0..text.len() {
    shellWriteChar(text.as_bytes()[i] as char, color);
  }
}

unsafe fn shellWriteCharArr(text: [char; 255], length: usize, color: u8) {
  if length > 255 {
      return;
  }
  for i in 0..length {
    shellWriteChar(text[i] as char, color);
  }
}

#[no_mangle]
pub unsafe extern "C" fn shellShowPrompt() {
  shellNewLine();
  shellWriteStr(PROMPT, COLOR_LIGHT_GREEN);
  for i in SHELL_TEXT_POSITION..SHELL_TEXT_BUFFER.len() {
    SHELL_TEXT_BUFFER[i] = ' ';
  }
}

unsafe fn shellCheckCommand(command: &str) -> bool {
  let commandStartPos = shellGetLineStart() + PROMPT.len();
  if commandStartPos < 0 {
      return false;
  }
  if commandStartPos + command.len() + 1 >= SHELL_TEXT_BUFFER.len() {
      return false;
  }
  if SHELL_TEXT_BUFFER[commandStartPos + command.len()] != ' ' {
      return false;
  }
  for i in 0..command.len() {
      if command.as_bytes()[i] as char != SHELL_TEXT_BUFFER[commandStartPos + i] {
          return false;
      }
  }
  return true;
}

unsafe fn shellCommandGetParam(command: &str) -> ([char;255], usize) {
  let paramStartPos = shellGetLineStart() + PROMPT.len() + 1 + command.len(); 
  let paramEndPos = SHELL_TEXT_POSITION ;

  let mut param: [char;255] = [' '; 255];
  if paramEndPos + 1 == paramStartPos {
    return (param, 0)
  }
  for i in paramStartPos..paramEndPos {
      if i < SHELL_TEXT_BUFFER.len() && i > 0 && i - paramStartPos < 255 {
          param[i - paramStartPos] = SHELL_TEXT_BUFFER[i] as char;
      }
  }
  return (param, paramEndPos - paramStartPos + 1);
}

unsafe fn shellCharArrayToCChar(param: &[char;255], length: usize) ->  *const cty::c_char {
  let mut param_c: [cty::c_char; 256] = [0; 256];
  if length > 254 {
    return ptr::null_mut();
  }
  for i in 0..length {
    param_c[i] = param[i] as i8;
  }
  param_c[length + 1] = 0;
  return param_c.as_ptr();
}

unsafe fn shellCommandGetParamC(command: &str) -> *const cty::c_char {
  let (param, length) = shellCommandGetParam(command);
  return shellCharArrayToCChar(&param, length);
}

unsafe fn shellCommand() {
  if shellCheckCommand("help") {
    shellWriteStr("\nhelp:\n", COLOR_LIGHT_GREY);
    
    shellWriteStr("- ping", COLOR_PINK);
    shellWriteStr("         ", COLOR_LIGHT_GREY);
    shellWriteStr(" - respond with pong\n", COLOR_LIGHT_GREY);
    
    shellWriteStr("- sp ", COLOR_PINK);
    shellWriteStr("<param   >", COLOR_CYAN);
    shellWriteStr(" - split parameters from command-line\n", COLOR_LIGHT_GREY);
    
    shellWriteStr("- ls ", COLOR_PINK);
    shellWriteStr("<dir path>", COLOR_CYAN);
    shellWriteStr(" - show directory contents\n", COLOR_LIGHT_GREY);
    
    shellWriteStr("- rf ", COLOR_PINK);
    shellWriteStr("<filename>", COLOR_CYAN);
    shellWriteStr(" - read file contents\n", COLOR_LIGHT_GREY);

    shellWriteStr("- sc ", COLOR_PINK);
    shellWriteStr("          ", COLOR_CYAN);
    shellWriteStr(" - print scancode of next key press\n", COLOR_LIGHT_GREY);

    shellWriteStr("- ed ", COLOR_PINK);
    shellWriteStr("<filename>", COLOR_CYAN);
    shellWriteStr(" - open text editor\n", COLOR_LIGHT_GREY);

    shellWriteStr("- re ", COLOR_PINK);
    shellWriteStr("          ", COLOR_CYAN);
    shellWriteStr(" - run example code\n", COLOR_LIGHT_GREY);

    shellWriteStr("- rc ", COLOR_PINK);
    shellWriteStr("<*.c file>", COLOR_CYAN);
    shellWriteStr(" - run C code", COLOR_LIGHT_GREY);

  } else if shellCheckCommand("ping") {
      shellWriteStr("\npong", COLOR_LIGHT_GREY);
  } else if shellCheckCommand("ls") {
      let (param, length) = shellCommandGetParam("ls");
      if length < 1 {
        let mut param_c: [cty::c_char; 2] = [0; 2];
        param_c[0] = '/' as i8;
        param_c[1] = '\0' as i8;
        shellNewLine();
        shellWriteStr("/\n", COLOR_PINK);
        ataList(param_c.as_ptr());
      } else {
        let param_c_ptr = shellCharArrayToCChar(&param, length - 1); // todo: -1 should not be needed
        shellNewLine();
        ataList(param_c_ptr);
      }
  } else if shellCheckCommand("rf") {
    let param_c_ptr = shellCommandGetParamC("rf");
    shellNewLine();
    ataShowFileContents(param_c_ptr);
  } else if shellCheckCommand("sp") {
    let (param, length) = shellCommandGetParam("sp");
    shellWriteStr("\nSplit Param: ", COLOR_PINK);
    shellWriteCharArr(param, length, COLOR_CYAN);
  } else if shellCheckCommand("sc") {
    shellWriteStr("\nKeyboard Scancode: ", COLOR_PINK);
    state = EState::SCANCODE;
  } else if shellCheckCommand("ed") {
    let (param, length) = shellCommandGetParam("sp");
    editorSetFilename(&param, length);
    state = EState::EDITFILE;
  } else if shellCheckCommand("rc") {
    let param_c_ptr = shellCommandGetParamC("rc");
    tinyccRunCode(param_c_ptr);
  } else if shellCheckCommand("re") {
    shellNewLine();
    tinyccRunExample();
  }
  shellShowPrompt();
}

pub fn shellSetPixel(x: u16, y: u16, color: u8) {
  unsafe {
    ptr::write(SHELL_PIXEL_BUFFER.wrapping_add(usize::from(x + y*SHELL_SCREEN_WIDTH)), color as i8);
  }
}

fn shellClearChar(x: u16, y: u16, bgcolor: u8) {
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
    shellSetPixel(curx, cury, bgcolor);

    // -- advance in row
    curx += 1;
  }
}

fn shellRenderChar(x: u16, y: u16, charToWrite: char, color: u8, bgcolor: u8) {
  if charToWrite == ' ' {
    shellClearChar(x,y, bgcolor);
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
      shellSetPixel(curx, cury, bgcolor);
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
  
  shellWriteStr("Type ", COLOR_LIGHT_GREY);
  shellWriteStr("help", COLOR_PINK);
  shellWriteStr(" for a list of commands\n\n", COLOR_LIGHT_GREY);
}

#[no_mangle]
pub unsafe extern "C" fn shellUpdate() {
  if state == EState::SHELL {
    if SHELL_CHANGED {
      let mut x:u16 = 1;
      let mut y:u16 = 10;
      for i in 0..SHELL_TEXT_BUFFER.len() {
        if x >= SHELL_SCREEN_WIDTH - FONT_WIDTH[0] {
          x = 1;
          y += 10;
        }

        shellRenderChar(x, y, SHELL_TEXT_BUFFER[i], SHELL_TEXT_COLOR_BUFFER[i], COLOR_DARK_GREY);

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
  if state == EState::EDITFILE {
    editfile::editorUpdate();
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

  let linesPerScreen = SHELL_SCREEN_HEIGHT / FONT_HEIGHT[0];
  if currentRow > linesPerScreen as usize {
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
  
  if state == EState::SHELL {
    if key as u8 == 10 { // newline
      shellCommand();
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
    shellWriteChar(key as u8 as char, COLOR_LIGHT_GREY);
  }
  if state == EState::SCANCODE {
    let c: char = char::from(scancode);
    shellWriteChar(c, COLOR_RED);
    state = EState::SHELL;
  }
  if state == EState::EDITFILE {
    editfile::editorKeyboardDown(key, scancode);
  }
}
