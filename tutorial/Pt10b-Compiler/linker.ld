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

/**
 * @file linker.ld
 * @brief Linker Script for CustomOS
 *
 * @see https://wiki.osdev.org/Bare_Bones
 * @see https://github.com/sk-io/os/blob/master/kernel.ld
 * @see https://github.com/ozkl/soso/blob/master/link.ld
 * @see https://github.com/mit-pdos/xv6-public/blob/master/kernel.ld
*/

ENTRY(_start)
SECTIONS {
	. = 0x00100000;
	.multiboot ALIGN(4K) : {
		*(.multiboot)
	}
	.boot ALIGN(4K) : {
		*(.boot)
	}
	. += 0xC0000000;
	.text ALIGN(4K) : AT(ADDR(.text) - 0xC0000000) {
		*(.text)
	}
	.rodata ALIGN(4K) : AT(ADDR(.rodata) - 0xC0000000) {
		*(.rodata)
	}
	.data ALIGN(4K) : AT(ADDR(.data) - 0xC0000000) {
		*(.data)
	}
	.bss ALIGN(4K) : AT(ADDR(.bss) - 0xC0000000) {
		*(COMMON)
		*(.bss)
	}
	_kernel_end = .;
}
