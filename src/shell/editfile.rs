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
 * @file editfile.rs
 * @brief Text Editor for the CustomOS
 *
 * a text editor to allow viewing and editing text files.
 *
 * This is only a basic text editor and so it has some quirks:
 *
 * - replace mode isn't properly implemented, wasn't a priority
 *
 * - y and z are swapped for german keyboard layout, but the rest of the keys follow the english keyboard layout
 *
 * - when writing in german: umlauts aren't implemented
 *
 * - due to the selected font and size the letter 'M' looks like an 'H' or 'W'
 *
 * - when using the cursor keys the text jumps when passing empty lines
 *
*/

extern "C" {
  fn ataGetFileContents(fileName: *const cty::c_char, buffer: *mut cty::c_char, bufferLen: u32);
  fn ataSetFileContents(fileName: *const cty::c_char, buffer: *mut cty::c_char, bufferLen: u32);
}

use crate::shellWidth;
use crate::shellHeight;
use crate::shellSetPixel;

use crate::COLOR_RED;
use crate::COLOR_GREEN;
use crate::COLOR_LIGHT_GREY;
use crate::COLOR_DARK_GREY;
use crate::COLOR_YELLOW;

use crate::shellRenderChar;
use crate::shellFontWidth;
use crate::shellFontHeight;
use crate::shellCharArrayToCChar;

use crate::shellSetActive;

static mut EDITOR_FILENAME: [char; 255] = [' '; 255];
static mut EDITOR_FILENAME_LENGTH: usize = 0;

static mut EDITOR_CHANGED: bool = true;
static mut EDITOR_TEXT_BUFFER: [i8; 65000] = [0; 65000];
static mut EDITOR_CURSOR_POS: usize = 0;

static mut EDITOR_INSERT_ACTIVE: bool = true;
static mut EDITOR_SAVED: bool = false;

const COLOR_TEXT : u8 = COLOR_LIGHT_GREY;
const COLOR_BACKGROUND : u8 = COLOR_DARK_GREY;
const COLOR_HIGHLIGHT : u8 = COLOR_RED;

pub unsafe fn editorSetFilename(filename: &[char;255], length: usize) {
  // -- copy filename
  if length > 254 {
    return
  }
  for i in 0..length {
    EDITOR_FILENAME[i] = filename[i];
  }
  EDITOR_FILENAME_LENGTH = length;
  
  // -- clear buffer
  for i in (0..EDITOR_TEXT_BUFFER.len()) {
    EDITOR_TEXT_BUFFER[i] = 0;
  }
  EDITOR_CURSOR_POS = 0;

  // -- load file contents
  let filename_c = shellCharArrayToCChar(&EDITOR_FILENAME, EDITOR_FILENAME_LENGTH);
  ataGetFileContents(filename_c, EDITOR_TEXT_BUFFER.as_mut_ptr(), EDITOR_TEXT_BUFFER.len() as u32);
  EDITOR_SAVED = true;

  // -- set cursor at end
  /*for i in 0..EDITOR_TEXT_BUFFER.len() {
    if EDITOR_TEXT_BUFFER[i] != 0 {
      EDITOR_CURSOR_POS = i;
    }
  }*/
  
  // -- set curstor at start
  EDITOR_CURSOR_POS = 0;
  
  EDITOR_CHANGED = true;
}

unsafe fn editorShiftTextForward(position: usize) {
  for i in (EDITOR_CURSOR_POS..EDITOR_TEXT_BUFFER.len() - 1).rev() {
    EDITOR_TEXT_BUFFER[i + 1] = EDITOR_TEXT_BUFFER[i];
  }
}

unsafe fn editorShiftTextBackward(position: usize) {
  if position < 1 {
    return;
  }
  if position >= EDITOR_TEXT_BUFFER.len() {
    return;
  }
  for i in position..EDITOR_TEXT_BUFFER.len() {
    EDITOR_TEXT_BUFFER[i - 1] = EDITOR_TEXT_BUFFER[i];
  }
}

unsafe fn getBufferSize() -> usize {
  let mut filesize = 0;
  for i in 0..EDITOR_TEXT_BUFFER.len() {
    if EDITOR_TEXT_BUFFER[i] != 0 as i8 {
      filesize = i;
    }
  }
  filesize as usize
}
pub unsafe fn editorKeyboardDown(key: cty::c_char, scancode: u8) {
  let charsPerLine = (shellWidth() / shellFontWidth()) as usize;
  if charsPerLine == 0 {
    return
  }
  let currentRow = EDITOR_CURSOR_POS / charsPerLine;

  if scancode == 0x1c { // newline
    if EDITOR_INSERT_ACTIVE {
      if EDITOR_CURSOR_POS > EDITOR_TEXT_BUFFER.len() - 1 {
        return;
      }

      editorShiftTextForward(EDITOR_CURSOR_POS);
      
      // -- set new line char
      EDITOR_TEXT_BUFFER[EDITOR_CURSOR_POS] = '\n' as i8;
      EDITOR_CURSOR_POS += 1;
    } else {
      EDITOR_CURSOR_POS = (currentRow + 1) * charsPerLine;
    }
  } else if scancode == 0x0E { // backspace
    if EDITOR_CURSOR_POS > 0 {
      if EDITOR_INSERT_ACTIVE == false {
        EDITOR_CURSOR_POS -= 1;
        if EDITOR_CURSOR_POS < EDITOR_TEXT_BUFFER.len() {
          EDITOR_TEXT_BUFFER[EDITOR_CURSOR_POS] = 0;
          EDITOR_SAVED = false;
        }
      }
      if EDITOR_INSERT_ACTIVE {
        editorShiftTextBackward(EDITOR_CURSOR_POS);
        EDITOR_CURSOR_POS -= 1;
      }
      EDITOR_SAVED = false;
    }
  } else if scancode == 0x53 { // delete
      if EDITOR_INSERT_ACTIVE {
        editorShiftTextBackward(EDITOR_CURSOR_POS+1);
      }
      EDITOR_SAVED = false;
  } else if scancode == 0x4B { // left
    EDITOR_CURSOR_POS -= 1;
  } else if scancode == 0x4D { // right
    EDITOR_CURSOR_POS += 1;
  } else if scancode == 0x50 { // down
    let (_, cy) = bufferPosToVirtualPos(EDITOR_CURSOR_POS);
    EDITOR_CURSOR_POS = virtualLinePosToBufferPos(cy + 1);
  } else if scancode == 0x48 { // up
    let (_, cy) = bufferPosToVirtualPos(EDITOR_CURSOR_POS);
    EDITOR_CURSOR_POS = virtualLinePosToBufferPos(cy - 1);
  } else if scancode == 0x3C { // F2 - save
    // -- save file
    let filename_c = shellCharArrayToCChar(&EDITOR_FILENAME, EDITOR_FILENAME_LENGTH);
    ataSetFileContents(filename_c, EDITOR_TEXT_BUFFER.as_mut_ptr(), getBufferSize() as u32);
    EDITOR_SAVED = true;
  } else if scancode == 0x3E { // F4 - replace/insert
    EDITOR_INSERT_ACTIVE ^= true;
  } else if scancode == 0x43 { // F9 - exit
    shellSetActive();
  } else { // write char
    if EDITOR_CURSOR_POS > EDITOR_TEXT_BUFFER.len() - 1 {
      return;
    }
    if EDITOR_INSERT_ACTIVE {
      editorShiftTextForward(EDITOR_CURSOR_POS);
    }
    EDITOR_TEXT_BUFFER[EDITOR_CURSOR_POS] = key as i8;
    EDITOR_CURSOR_POS += 1;
    EDITOR_SAVED = false;
  }

  EDITOR_CHANGED = true;
  return;
}

unsafe fn editorClear() {
  for y in 0..shellHeight() {
    for x in 0..shellWidth() {
      if y >= 10 && y < 190 {
        shellSetPixel(x, y, COLOR_DARK_GREY);
      }
    }
  }
}

unsafe fn editorWriteStr(x: u16, y: u16, text : &str) {
  let mut px = x;
  for i in 0..text.len() {
    px += shellFontWidth();
    shellRenderChar(px, y, text.as_bytes()[i] as char, COLOR_LIGHT_GREY, COLOR_GREEN);
  }
}

unsafe fn editorRedrawBorder() {
  for y in 0..shellHeight() {
    for x in 0..shellWidth() {
      if y < 10 { // top border
        shellSetPixel(x, y, COLOR_GREEN);
      }
      if y >= 190 { // bottom border
        shellSetPixel(x, y, COLOR_GREEN);
      }
    }
  }

  let header = "| Edit File - ";
  editorWriteStr(0, 0, header);
  
  for i in 0..EDITOR_FILENAME_LENGTH {
    let x = 1 + (i as u16 + header.len() as u16) * shellFontWidth();
    if x < 0 {
      break;
    }
    if x > 300 { // todo: use shellWidth, but causes rustc to add exception handling for out-of-bounds
      break;
    }
    shellRenderChar(x, 0, EDITOR_FILENAME[i] as char, COLOR_LIGHT_GREY, COLOR_GREEN);
  }
  
  if EDITOR_INSERT_ACTIVE {
    editorWriteStr(0, 190, "Insert  [F4] | ");
  } else {
    editorWriteStr(0, 190, "Replace [F4] | ");
  }
  
  if EDITOR_SAVED {
    editorWriteStr(15* shellFontWidth(), 190, "Saved [F2] | ");
  } else {
    editorWriteStr(15* shellFontWidth(), 190, "Save  [F2] | ");
  }
  
  editorWriteStr((15+13)* shellFontWidth(), 190, "Exit [F9] | ");
}

// ---

unsafe fn numberOfLines() -> u16 {
  let mut cx: u16 = 0;
  let mut cy: u16 = 0;
  for i in 0..getBufferSize() {
    let chr = EDITOR_TEXT_BUFFER[i] as u8 as char;
    if chr == '\n' || (cx + 1) * shellFontWidth() > shellWidth() {
      cy += 1;
      cx = 0;
      continue;
    }
  }
  return cy;
}

unsafe fn bufferPosToVirtualPos(bufferPos: usize) -> (u16, u16) {
  if bufferPos < 0 {
    return (0, 0);
  }
  if bufferPos > EDITOR_TEXT_BUFFER.len() - 1 {
    return (0, numberOfLines());
  }

  let mut cy: u16 = 0;
  let mut cx: u16 = 0;
  for i in 0..bufferPos {
    let chr = EDITOR_TEXT_BUFFER[i] as u8 as char;
    if chr == '\n' || (cx + 2) * shellFontWidth() > shellWidth() {
      cy += 1;
      cx = 0;
      continue;
    }
    cx += 1;
  }
 
  (cx, (cy + 1))
}

unsafe fn virtualLinePosToBufferPos(y: u16) -> usize {
  if y < 2 {
    return 0;
  }
  let mut bufferPos = 0;
  let mut cx: u16 = 0;
  let mut cy: u16 = 0;
  for i in 0..getBufferSize() {
    let chr = EDITOR_TEXT_BUFFER[i] as u8 as char;
    if chr == '\n' || (cx + 2) * shellFontWidth() > shellWidth()  {
      cy += 1;
      cx = 0;
    } else {
      cx += 1;
    }
    if cy + 1 == y {
      return i + 1;
    } 
  }
  return getBufferSize() - 1; // jump to end
}

// ---

pub unsafe fn editorUpdate() {
  if EDITOR_CHANGED {
    editorRedrawBorder();
    
    // -- clear screen
    editorClear();

    let linesPerScreen = shellHeight() / shellFontHeight() - 1;
    let (cursorx, cursory) = bufferPosToVirtualPos(EDITOR_CURSOR_POS);

    // -- view area
    let mut startY = 0;
    let mut endY = linesPerScreen;

    if cursory > linesPerScreen {
      endY = cursory;
      startY = endY - linesPerScreen;
    }
    
    // --

    //let startLineBufferPos = virtualPosToBufferPos(0, startY); // possible optimization, search only after line
    
    let mut currentChr = ' ';
    for i in 0..getBufferSize() + 1 {
      let (x,y) = bufferPosToVirtualPos(i);
      
      if y <= startY  {
        continue;
      }
      if y > endY  {
        continue;
      }

      let chr = EDITOR_TEXT_BUFFER[i] as u8 as char;
      if EDITOR_CURSOR_POS == i {
        if chr == ' ' || chr == '\0' || chr == '\n' {
          shellRenderChar(x * shellFontWidth(), (y - startY) * shellFontHeight(), '_', COLOR_TEXT, COLOR_HIGHLIGHT);
        } else {
          shellRenderChar(x * shellFontWidth(), (y - startY) * shellFontHeight(), chr, COLOR_TEXT, COLOR_HIGHLIGHT);
        }
      } else {
        shellRenderChar(x * shellFontWidth(), (y - startY) * shellFontHeight(), chr, COLOR_TEXT, COLOR_BACKGROUND);
      }
    }
    
    EDITOR_CHANGED = false;
  }
}

