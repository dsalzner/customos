%ifdef _ALLCODE_
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
 * @file boot.asm
 * @brief Boot script
 *
 * @see https://wiki.osdev.org/Bare_Bones_with_NASM
 * @see https://github.com/sk-io/os/blob/master/kernel/boot.asm
*/
%endif

MBOOT_PAGE_ALIGN    equ 1 << 0
MBOOT_MEM_INFO      equ 1 << 1
MBOOT_USE_GFX       equ 1 << 2

MBOOT_MAGIC    equ 0x1BADB002
MBOOT_FLAGS    equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO | MBOOT_USE_GFX
MBOOT_CHECKSUM equ -(MBOOT_MAGIC + MBOOT_FLAGS)

section .multiboot
align 4
    dd MBOOT_MAGIC
    dd MBOOT_FLAGS
    dd MBOOT_CHECKSUM
    ; Info about where to load us.
    ; Since we are using ELF, mboot reads from its header instead
    dd 0, 0, 0, 0, 0

    ; Graphics requests
    dd 0 ; 0 = linear graphics
    dd 800
    dd 600
    dd 32

; setup stack
section .bss
align 16
stack_bottom:
    resb 16384*4 ; 16 kib
stack_top:

section .boot
global _start
_start:
    mov eax, (initial_page_dir - 0xC0000000)
    mov cr3, eax

    mov ecx, cr4
    or ecx, 0x10
    mov cr4, ecx

    mov ecx, cr0
    or ecx, 0x80000000
    mov cr0, ecx

    lea ecx, higher_half
    jmp ecx

section .text
higher_half:
    ; protected mode
    ; interrupts and paging disabled
sti

    mov esp, stack_top

    push ebx ; multiboot mem info pointer

sti

    extern kernel_main
    call kernel_main

halt:
    hlt
    jmp halt

section .data
align 4096
global initial_page_dir ; optimization: use bss and build in assembly
initial_page_dir:
    dd 10000011b ; initial 4mb identity map, unmapped later

    times 768-1 dd 0 ; padding

    ; hh kernel start, map 16 mb
    dd (0 << 22) | 10000011b ; 0xC0000000
    dd (1 << 22) | 10000011b
    dd (2 << 22) | 10000011b
    dd (3 << 22) | 10000011b
    times 256-4 dd 0 ; padding

    ; dd initial_page_dir | 11b

global gdt_flush
extern gp
gdt_flush:
    lgdt  [gp]
    mov ax, 0x10
    mov ds, eax
    mov es, eax
    mov fs, eax
    mov gs, eax
    mov ss, eax
    jmp 0x08:flush2
flush2:
    ret

global idt_load
extern idtp
idt_load:
    lidt  [idtp]
    ret

%include "isr.asm"
%include "irq.asm"
