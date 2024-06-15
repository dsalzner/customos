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
use crate::editfile::editor_set_filename;

const COLOR_GREEN: u8 = 2;
const COLOR_RED: u8 = 4;
const COLOR_LIGHT_GREY: u8 = 7;
const COLOR_LIGHT_GREEN: u8 = 10;
const COLOR_DARK_GREY: u8 = 17;
const COLOR_CYAN: u8 = 3;
const COLOR_PINK: u8 = 13;
//const COLOR_YELLOW: u8 = 14;
const PROMPT: &str = "cOS>";

static mut SHELL_TEXT_POSITION: usize = 0;
static mut SHELL_TEXT_BUFFER: [char; 6000] = [' '; 6000];
static mut SHELL_TEXT_COLOR_BUFFER: [u8; 6000] = [COLOR_RED; 6000];
pub static mut SHELL_SCREEN_WIDTH: u16 = 0;
pub static mut SHELL_SCREEN_HEIGHT: u16 = 0;
static mut SHELL_CHANGED: bool = true;

extern "C" {
  fn ataList(path: *const cty::c_char);
  fn ataShowFileContents(filename: *const cty::c_char);
  fn tinyccRunCode(filename: *const cty::c_char);
  fn tinyccRunExample();
  fn graphicsSetPixel(x: u16, y: u16, color: u8);
}

#[derive(PartialEq)]
enum EState {
  SHELL,
  SCANCODE,
  EDITFILE,
}
static mut STATE: EState = EState::SHELL;

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
pub unsafe extern "C" fn shell_write_char(ch: cty::c_char, color: u8) {
  if ch as u8 as char == '\n' { // newline
    shell_new_line();
    return;
  }
  if ch as u8 as char  == '\0' {
    return;
  }

  if SHELL_TEXT_POSITION > SHELL_TEXT_BUFFER.len() - 1 {
      return;
  }
  SHELL_TEXT_BUFFER[SHELL_TEXT_POSITION] = ch as u8 as char;
  SHELL_TEXT_COLOR_BUFFER[SHELL_TEXT_POSITION] = color;
  SHELL_TEXT_POSITION += 1;

  SHELL_CHANGED = true;
}

pub unsafe fn shell_height() -> u16 {
  return SHELL_SCREEN_HEIGHT;
}

pub unsafe fn shell_width() -> u16 {
  return SHELL_SCREEN_WIDTH;
}

pub unsafe fn shell_font_width() -> u16 {
  return FONT_WIDTH[0];
}

pub unsafe fn shell_font_height() -> u16 {
  return FONT_HEIGHT[0];
}

pub unsafe fn shell_set_active() {
  shell_new_line();
  STATE = EState::SHELL;
}

unsafe fn shell_write_str(text: &str, color: u8) {
  for i in 0..text.len() {
    shell_write_char(text.as_bytes()[i] as cty::c_char, color);
  }
}

unsafe fn shell_write_char_arr(text: [char; 255], length: usize, color: u8) {
  if length > 255 {
      return;
  }
  for i in 0..length {
    shell_write_char(text[i]  as u8 as cty::c_char, color);
  }
}

#[no_mangle]
pub unsafe extern "C" fn shell_show_prompt() {
  shell_new_line();
  shell_write_str(PROMPT, COLOR_LIGHT_GREEN);
  for i in SHELL_TEXT_POSITION..SHELL_TEXT_BUFFER.len() {
    SHELL_TEXT_BUFFER[i] = ' ';
  }
}

unsafe fn shell_check_command(command: &str) -> bool {
  let command_start_pos = shell_get_line_start() + PROMPT.len();
  if command_start_pos + command.len() + 1 >= SHELL_TEXT_BUFFER.len() {
      return false;
  }
  if SHELL_TEXT_BUFFER[command_start_pos + command.len()] != ' ' {
      return false;
  }
  for i in 0..command.len() {
      if command.as_bytes()[i] as char != SHELL_TEXT_BUFFER[command_start_pos + i] {
	  return false;
      }
  }
  return true;
}

unsafe fn shell_command_get_param(command: &str) -> ([char;255], usize) {
  let param_start_pos = shell_get_line_start() + PROMPT.len() + 1 + command.len();
  let param_end_pos = SHELL_TEXT_POSITION ;

  let mut param: [char;255] = [' '; 255];
  if param_end_pos + 1 == param_start_pos {
    return (param, 0)
  }
  for i in param_start_pos..param_end_pos {
      if i < SHELL_TEXT_BUFFER.len() && i > 0 && i - param_start_pos < 255 {
	  param[i - param_start_pos] = SHELL_TEXT_BUFFER[i] as char;
      }
  }
  return (param, param_end_pos - param_start_pos + 1);
}

unsafe fn shell_char_array_to_cchar(param: &[char;255], length: usize) ->  *const cty::c_char {
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

unsafe fn shell_command_get_param_c(command: &str) -> *const cty::c_char {
  let (param, length) = shell_command_get_param(command);
  return shell_char_array_to_cchar(&param, length);
}

unsafe fn shell_command() {
  if shell_check_command("help") {
    shell_write_str("\nhelp:\n", COLOR_LIGHT_GREY);

    shell_write_str("- ping", COLOR_PINK);
    shell_write_str("         ", COLOR_LIGHT_GREY);
    shell_write_str(" - respond with pong\n", COLOR_LIGHT_GREY);

    shell_write_str("- sp ", COLOR_PINK);
    shell_write_str("<param   >", COLOR_CYAN);
    shell_write_str(" - split parameters from command-line\n", COLOR_LIGHT_GREY);

    shell_write_str("- ls ", COLOR_PINK);
    shell_write_str("<dir path>", COLOR_CYAN);
    shell_write_str(" - show directory contents\n", COLOR_LIGHT_GREY);

    shell_write_str("- rf ", COLOR_PINK);
    shell_write_str("<filename>", COLOR_CYAN);
    shell_write_str(" - read file contents\n", COLOR_LIGHT_GREY);

    shell_write_str("- sc ", COLOR_PINK);
    shell_write_str("          ", COLOR_CYAN);
    shell_write_str(" - print scancode of next key press\n", COLOR_LIGHT_GREY);

    shell_write_str("- ed ", COLOR_PINK);
    shell_write_str("<filename>", COLOR_CYAN);
    shell_write_str(" - open text editor\n", COLOR_LIGHT_GREY);

    shell_write_str("- re ", COLOR_PINK);
    shell_write_str("          ", COLOR_CYAN);
    shell_write_str(" - run example code\n", COLOR_LIGHT_GREY);

    shell_write_str("- rc ", COLOR_PINK);
    shell_write_str("<*.c file>", COLOR_CYAN);
    shell_write_str(" - run C code", COLOR_LIGHT_GREY);

  } else if shell_check_command("ping") {
      shell_write_str("\npong", COLOR_LIGHT_GREY);
  } else if shell_check_command("ls") {
      let (param, length) = shell_command_get_param("ls");
      if length < 1 {
	let mut param_c: [cty::c_char; 2] = [0; 2];
	param_c[0] = '/' as i8;
	param_c[1] = '\0' as i8;
	shell_new_line();
	shell_write_str("/\n", COLOR_PINK);
	ataList(param_c.as_ptr());
      } else {
	let param_c_ptr = shell_char_array_to_cchar(&param, length - 1); // todo: -1 should not be needed
	shell_new_line();
	ataList(param_c_ptr);
      }
  } else if shell_check_command("rf") {
    let param_c_ptr = shell_command_get_param_c("rf");
    shell_new_line();
    ataShowFileContents(param_c_ptr);
  } else if shell_check_command("sp") {
    let (param, length) = shell_command_get_param("sp");
    shell_write_str("\nSplit Param: ", COLOR_PINK);
    shell_write_char_arr(param, length, COLOR_CYAN);
  } else if shell_check_command("sc") {
    shell_write_str("\nKeyboard Scancode: ", COLOR_PINK);
    STATE = EState::SCANCODE;
  } else if shell_check_command("ed") {
    let (param, length) = shell_command_get_param("sp");
    editor_set_filename(&param, length);
    STATE = EState::EDITFILE;
  } else if shell_check_command("rc") {
    let param_c_ptr = shell_command_get_param_c("rc");
    tinyccRunCode(param_c_ptr);
  } else if shell_check_command("re") {
    shell_new_line();
    tinyccRunExample();
  }
  shell_show_prompt();
}

pub fn shell_set_pixel(x: u16, y: u16, color: u8) {
  unsafe {
    graphicsSetPixel(x, y, color);
  }
}

fn shell_clear_char(x: u16, y: u16, bgcolor: u8) {
  let char_pixel_count = FONT_WIDTH[0] * FONT_HEIGHT[0];
  let mut curx = x;
  let mut cury = y;

  for bit_no in 0..char_pixel_count {
    if bit_no % 8 == 0 {
	curx = x;
    }
    if curx - x == FONT_WIDTH[0] {
      cury += 1;
      curx = x;
    }
    // -- clear pixel
    shell_set_pixel(curx, cury, bgcolor);

    // -- advance in row
    curx += 1;
  }
}

fn shell_render_char(x: u16, y: u16, char_to_write: char, color: u8, bgcolor: u8) {
  if char_to_write == ' ' {
    shell_clear_char(x,y, bgcolor);
    return
  }
  let char_arr_id = char_to_write as usize - 33;
  if 92 < char_arr_id { // -- avoid implicit bounds check
    return
  }
  let char_pixel_count = FONT_WIDTH[char_arr_id] * FONT_HEIGHT[char_arr_id];
  let mut curx = x;
  let mut cury = y;

  for bit_no in 0..char_pixel_count {
    let byte_no = bit_no / 8;
    let bits = FONT_BUFFER[char_arr_id][byte_no as usize];

    if bit_no % 8 == 0 {
      curx = x;
    }

    // -- advance to next row
    if curx - x == FONT_WIDTH[char_arr_id] {
      cury += 1;
      curx = x;
    }

    // -- draw pixel if set
    if bits & (1 << (bit_no % 8)) != 0 {
      shell_set_pixel(curx, cury, color);
    } else {
      shell_set_pixel(curx, cury, bgcolor);
    }

    // -- advance in row
    curx += 1;
  }
}

pub unsafe fn shell_redraw_border() {
  for y in 0..SHELL_SCREEN_HEIGHT {
    for x in 0..SHELL_SCREEN_WIDTH {
      if y < 10 { // top border
	shell_set_pixel(x, y, COLOR_CYAN);
      }
      if y >= SHELL_SCREEN_HEIGHT-10 { // bottom border
	shell_set_pixel(x, y, COLOR_CYAN);
      }
    }
  }
}

#[no_mangle]
pub unsafe extern "C" fn shell_init(w: u16, h: u16) {
  SHELL_SCREEN_WIDTH = w;
  SHELL_SCREEN_HEIGHT = h;

  shell_redraw_border();

  shell_write_str("Type ", COLOR_LIGHT_GREY);
  shell_write_str("help", COLOR_PINK);
  shell_write_str(" for a list of commands\n\n", COLOR_LIGHT_GREY);
}

#[no_mangle]
pub unsafe extern "C" fn shell_update() {
  if STATE == EState::SHELL {
    if SHELL_CHANGED {
      let mut x:u16 = 1;
      let mut y:u16 = 10;
      /*if SHELL_TEXT_POSITION+1 > SHELL_TEXT_BUFFER.len() {
	return;
      }*/
      for i in 0..SHELL_TEXT_BUFFER.len() { //SHELL_TEXT_POSITION+1 {
	if x >= SHELL_SCREEN_WIDTH - FONT_WIDTH[0] {
	  x = 1;
	  y += 10;
	}

	if y > SHELL_SCREEN_HEIGHT - FONT_HEIGHT[0] {
	  break;
	}

	shell_render_char(x, y, SHELL_TEXT_BUFFER[i], SHELL_TEXT_COLOR_BUFFER[i], COLOR_DARK_GREY);

	let char_arr_id = SHELL_TEXT_BUFFER[i] as usize - 33;
	if char_arr_id < FONT_WIDTH.len() {
	    x += FONT_WIDTH[char_arr_id];
	}
	if SHELL_TEXT_BUFFER[i] == ' ' {
	    /*if x < SHELL_SCREEN_WIDTH / 2 { // TODO: bad efficiency hack
	      shell_clear_char(x, y, 1);
	    }*/
	    x += FONT_WIDTH[0];
	}
      }

      shell_redraw_border();
      SHELL_CHANGED = false;
    }
  }
  if STATE == EState::EDITFILE {
    editfile::editor_update();
  }
}

pub unsafe fn shell_get_line_start() -> usize {
  let chars_per_line = (SHELL_SCREEN_WIDTH / FONT_WIDTH[0]) as usize;
  if chars_per_line == 0 {
    return 0;
  }
  let current_row = SHELL_TEXT_POSITION / chars_per_line;
  let line_start = current_row  * chars_per_line;
  if line_start > SHELL_TEXT_BUFFER.len() {
    return 0;
  }
  return line_start;
}

pub unsafe fn shell_new_line() {
  let chars_per_line = (SHELL_SCREEN_WIDTH / FONT_WIDTH[0]) as usize;
  if chars_per_line == 0 {
    return
  }
  let current_row = SHELL_TEXT_POSITION / chars_per_line;
  SHELL_TEXT_POSITION = (current_row + 1) * chars_per_line;

  let lines_per_screen = SHELL_SCREEN_HEIGHT / (FONT_HEIGHT[0] - 2);
  if current_row > lines_per_screen as usize {
    // -- move all up one row
    for i in chars_per_line..SHELL_TEXT_BUFFER.len() {
      SHELL_TEXT_BUFFER[i - chars_per_line] = SHELL_TEXT_BUFFER[i];
      SHELL_TEXT_COLOR_BUFFER[i - chars_per_line] = SHELL_TEXT_COLOR_BUFFER[i];
    }
    // -- set new cursor position
    SHELL_TEXT_POSITION = (current_row) * chars_per_line;
    // -- signal buffer changed
    SHELL_CHANGED = true;
  }
}

#[no_mangle]
pub unsafe extern "C" fn shell_keyboard_down(mut key: cty::c_char, scancode: u8) {
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

  if STATE == EState::SHELL {
    if key as u8 == 10 { // newline
      shell_command();
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
    shell_write_char(key as u8 as cty::c_char, COLOR_LIGHT_GREY);
  }
  if STATE == EState::SCANCODE {
    let c: char = char::from(scancode);
    shell_write_char(c as u8 as cty::c_char, COLOR_RED);
    STATE = EState::SHELL;
  }
  if STATE == EState::EDITFILE {
    editfile::editor_keyboard_down(key, scancode);
  }
}
