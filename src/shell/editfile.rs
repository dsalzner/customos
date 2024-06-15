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
 *
*/

extern "C" {
  fn ataGetFileContents(fileName: *const cty::c_char, buffer: *mut cty::c_char, bufferLen: u32);
  fn ataSetFileContents(fileName: *const cty::c_char, buffer: *mut cty::c_char, bufferLen: u32);
}

use crate::shell_width;
use crate::shell_height;
use crate::shell_set_pixel;
use crate::shell_flush;

use crate::COLOR_RED;
use crate::COLOR_GREEN;
use crate::COLOR_LIGHT_GREY;
use crate::COLOR_BLUE;

use crate::shell_render_char;
use crate::shell_font_width;
use crate::shell_font_height;
use crate::shell_char_array_to_cchar;

use crate::shell_set_active;

static mut EDITOR_FILENAME: [char; 255] = ['\0'; 255];
static mut EDITOR_FILENAME_LENGTH: usize = 0;

static mut EDITOR_CHANGED: bool = true;
static mut EDITOR_TEXT_BUFFER: [i8; 65000] = [0; 65000];
static mut EDITOR_CURSOR_POS: usize = 0;

static mut EDITOR_INSERT_ACTIVE: bool = true;
static mut EDITOR_SAVED: bool = false;

const COLOR_TEXT : u8 = COLOR_LIGHT_GREY;
const COLOR_BACKGROUND : u8 = COLOR_BLUE;
const COLOR_HIGHLIGHT : u8 = COLOR_RED;

pub unsafe fn editor_set_filename(filename: &[char;255], length: usize) {
  // -- copy filename
  if length > 254 {
    return
  }
  for i in 0..length {
    EDITOR_FILENAME[i] = filename[i];
  }
  EDITOR_FILENAME_LENGTH = length;
  
  // -- clear buffer
  for i in 0..EDITOR_TEXT_BUFFER.len() {
    EDITOR_TEXT_BUFFER[i] = 0;
  }
  EDITOR_CURSOR_POS = 0;

  // -- load file contents
  let filename_c = shell_char_array_to_cchar(&EDITOR_FILENAME, EDITOR_FILENAME_LENGTH);
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
  
  editor_clear();
  shell_flush();
  EDITOR_CHANGED = true;
}

unsafe fn editor_shift_text_forward(position: usize) {
  for i in (position..EDITOR_TEXT_BUFFER.len() - 1).rev() {
    EDITOR_TEXT_BUFFER[i + 1] = EDITOR_TEXT_BUFFER[i];
  }
}

unsafe fn editor_shift_text_backward(position: usize) {
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

unsafe fn get_buffer_size() -> usize {
  let mut filesize = 0;
  for i in 0..EDITOR_TEXT_BUFFER.len() {
    if EDITOR_TEXT_BUFFER[i] != 0 as i8 {
      filesize = i;
    }
  }
  filesize as usize
}
pub unsafe fn editor_keyboard_down(key: cty::c_char, scancode: u8) {
  let chars_per_line = (shell_width() / shell_font_width()) as usize;
  if chars_per_line == 0 {
    return
  }
  let current_row = EDITOR_CURSOR_POS / chars_per_line;

  if scancode == 0x1c { // newline
    if EDITOR_INSERT_ACTIVE {
      if EDITOR_CURSOR_POS > EDITOR_TEXT_BUFFER.len() - 1 {
        return;
      }

      editor_shift_text_forward(EDITOR_CURSOR_POS);
      
      // -- set new line char
      EDITOR_TEXT_BUFFER[EDITOR_CURSOR_POS] = '\n' as i8;
      EDITOR_CURSOR_POS += 1;
    } else {
      EDITOR_CURSOR_POS = (current_row + 1) * chars_per_line;
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
	editor_shift_text_backward(EDITOR_CURSOR_POS);
        EDITOR_CURSOR_POS -= 1;
      }
      EDITOR_SAVED = false;
    }
  } else if scancode == 0x53 { // delete
      if EDITOR_INSERT_ACTIVE {
	editor_shift_text_backward(EDITOR_CURSOR_POS+1);
      }
      EDITOR_SAVED = false;
  } else if scancode == 0x4B { // left
    EDITOR_CURSOR_POS -= 1;
  } else if scancode == 0x4D { // right
    EDITOR_CURSOR_POS += 1;
  } else if scancode == 0x50 { // down
    let (_, cy) = buffer_pos_to_virtual_pos(EDITOR_CURSOR_POS);
    EDITOR_CURSOR_POS = virtual_line_pos_to_buffer_pos(cy + 1);
  } else if scancode == 0x48 { // up
    let (_, cy) = buffer_pos_to_virtual_pos(EDITOR_CURSOR_POS);
    EDITOR_CURSOR_POS = virtual_line_pos_to_buffer_pos(cy - 1);
  } else if scancode == 0x3C { // F2 - save
    // -- save file
    let filename_c = shell_char_array_to_cchar(&EDITOR_FILENAME, EDITOR_FILENAME_LENGTH);
    ataSetFileContents(filename_c, EDITOR_TEXT_BUFFER.as_mut_ptr(), get_buffer_size() as u32);
    EDITOR_SAVED = true;
  } else if scancode == 0x3E { // F4 - replace/insert
    EDITOR_INSERT_ACTIVE ^= true;
  } else if scancode == 0x43 { // F9 - exit
    shell_set_active();
  } else { // write char
    if EDITOR_CURSOR_POS > EDITOR_TEXT_BUFFER.len() - 1 {
      return;
    }
    if EDITOR_INSERT_ACTIVE {
      editor_shift_text_forward(EDITOR_CURSOR_POS);
    }
    EDITOR_TEXT_BUFFER[EDITOR_CURSOR_POS] = key as i8;
    EDITOR_CURSOR_POS += 1;
    EDITOR_SAVED = false;
  }

  EDITOR_CHANGED = true;
  return;
}

unsafe fn editor_clear() {
  let footer_pos = shell_height() - 10;

  for y in 0..shell_height() {
    for x in 0..shell_width() {
      if y >= 10 || y < footer_pos {
	shell_set_pixel(x, y, COLOR_BACKGROUND);
      }
      if y <= 10 {
	shell_set_pixel(x, y, COLOR_GREEN);
      }
      if y >= footer_pos {
	shell_set_pixel(x, y, COLOR_GREEN);
      }
    }
  }
}

unsafe fn editor_write_str(x: u16, y: u16, text : &str) {
  let mut px = x;
  for i in 0..text.len() {
    px += shell_font_width();
    shell_render_char(px, y, text.as_bytes()[i] as char, COLOR_LIGHT_GREY, COLOR_GREEN);
  }
}

unsafe fn editor_redraw_border() {
  let footer_pos = shell_height() - 10;

  editor_clear();

  let header = "| Edit File - ";
  editor_write_str(0, 0, header);
  
  for i in 0..EDITOR_FILENAME_LENGTH {
    let x = 1 + (i as u16 + header.len() as u16) * shell_font_width();
    if x > 640 { // TODO: use shell_width, but causes rustc to add exception handling for out-of-bounds
      break;
    }
    shell_render_char(x, 0, EDITOR_FILENAME[i] as char, COLOR_LIGHT_GREY, COLOR_GREEN);
  }

  if EDITOR_INSERT_ACTIVE {
    editor_write_str(0, footer_pos, "Insert  [F4] | ");
  } else {
    editor_write_str(0, footer_pos, "Replace [F4] | ");
  }
  
  if EDITOR_SAVED {
    editor_write_str(15 * shell_font_width(), footer_pos, "Saved [F2] | ");
  } else {
    editor_write_str(15 * shell_font_width(), footer_pos, "Save  [F2] | ");
  }
  
  editor_write_str((15 + 13) * shell_font_width(), footer_pos, "Exit [F9] | ");
}

// ---

unsafe fn number_of_lines() -> u16 {
  let mut cx: u16 = 0;
  let mut cy: u16 = 0;
  for i in 0..get_buffer_size() {
    let chr = EDITOR_TEXT_BUFFER[i] as u8 as char;
    if chr == '\n' || (cx + 1) * shell_font_width() > shell_width() {
      cy += 1;
      cx = 0;
      continue;
    }
  }
  return cy;
}

unsafe fn buffer_pos_to_virtual_pos(buffer_pos: usize) -> (u16, u16) {
  if buffer_pos > EDITOR_TEXT_BUFFER.len() - 1 {
    return (0, number_of_lines());
  }
  let mut cy: u16 = 0;
  let mut cx: u16 = 0;
  for i in 0..buffer_pos {
    let chr = EDITOR_TEXT_BUFFER[i] as u8 as char;
    if chr == '\n' || (cx + 2) * shell_font_width() > shell_width() {
      cy += 1;
      cx = 0;
      continue;
    }
    cx += 1;
  }
  (cx, (cy + 1))
}

unsafe fn virtual_line_pos_to_buffer_pos(y: u16) -> usize {
  if y < 2 {
    return 0;
  }
  let mut cx: u16 = 0;
  let mut cy: u16 = 0;
  for i in 0..get_buffer_size() {
    let chr = EDITOR_TEXT_BUFFER[i] as u8 as char;
    if chr == '\n' || (cx + 2) * shell_font_width() > shell_width()  {
      cy += 1;
      cx = 0;
    } else {
      cx += 1;
    }
    if cy + 1 == y {
      return i + 1;
    } 
  }
  return get_buffer_size() - 1; // jump to end
}

// ---

pub unsafe fn editor_update() {
  if EDITOR_CHANGED {
    editor_redraw_border();

    let lines_per_screen = shell_height() / shell_font_height() - 1;
    let (_cursorx, cursory) = buffer_pos_to_virtual_pos(EDITOR_CURSOR_POS);

    // -- view area
    let mut start_y = 0;
    let mut end_y = lines_per_screen;

    if cursory > lines_per_screen {
      end_y = cursory + 1;
      start_y = end_y - lines_per_screen + 1;
    }
    
    // --
    // let startLineBufferPos = virtualPosToBufferPos(0, start_y); // possible optimization, search only after line
    for i in 0..get_buffer_size() + 1 {
      let (x,y) = buffer_pos_to_virtual_pos(i);
      
      if y <= start_y  {
        continue;
      }
      if y > end_y  {
        continue;
      }

      let chr = EDITOR_TEXT_BUFFER[i] as u8 as char;
      if EDITOR_CURSOR_POS == i {
        if chr == ' ' || chr == '\0' || chr == '\n' {
	  shell_render_char(x * shell_font_width(), (y - start_y) * shell_font_height(), '_', COLOR_TEXT, COLOR_HIGHLIGHT);
        } else {
	  shell_render_char(x * shell_font_width(), (y - start_y) * shell_font_height(), chr, COLOR_TEXT, COLOR_HIGHLIGHT);
        }
      } else {
	shell_render_char(x * shell_font_width(), (y - start_y) * shell_font_height(), chr, COLOR_TEXT, COLOR_BACKGROUND);
      }
    }

    shell_flush();

    EDITOR_CHANGED = false;
  }
}

