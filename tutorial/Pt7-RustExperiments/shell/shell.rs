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

#![no_std]
use core::ptr;

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
pub extern "C" fn keyboardInterrupt(_key: cty::c_char, _scancode: u8) {

}

#[no_mangle]
pub extern "C" fn graphicsUpdate(buffer: *mut cty::c_char, w: u16, h: u16) {
  for y in 0..h {
    for x in 0..w {
      let mut  v = 0;
      if h/2 >= y && w/2 >= x {
        v = 3; // top-left, cyan
      }
      if h/2 >= y && w/2 <= x {
        v = 4; // top-right, red
      }
      if h/2 <= y && w/2 >= x {
        v = 2; // bottom-left, green
      }
      if h/2 <= y && w/2 <= x {
        v = 1; // bottom-right, blue
      }

      unsafe {
        ptr::write(buffer.wrapping_add(usize::from(x + y*w)), v);
      }
    }
  }
}
