MBALIGN  equ  1 << 0            ; align loaded modules on page boundaries
MEMINFO  equ  1 << 1            ; provide memory map
FLAGS    equ  MBALIGN | MEMINFO ; this is the Multiboot 'flag' field
MAGIC    equ  0x1BADB002        ; 'magic number' lets bootloader find the header
CHECKSUM equ -(MAGIC + FLAGS)   ; checksum of above, to prove we are multiboot

; multiboot header [..] magic values that are documented in the multiboot standard
section .multiboot
align 4
	dd MAGIC
	dd FLAGS
	dd CHECKSUM

; allocate room for a small stack by creating a symbol at the bottom of it
; then allocating 16384 bytes for it, and finally creating a symbol at the top.
section .bss
align 16
stack_bottom:
resb 16384 ; 16 KiB
stack_top:

; The linker script specifies _start as the entry point to the kernel and the
; bootloader will jump to this position once the kernel has been loaded.
section .text
global _start:function (_start.end - _start)
_start:

	; The bootloader has loaded us into 32-bit protected mode on a x86
	; machine.

	; To set up a stack, we set the esp register to point to the top of our
	; stack
	mov esp, stack_top

	; Enter the high-level kernel.
	extern kernel_main
	call kernel_main

	; If the system has nothing more to do, put the computer into an
	; infinite loop.
	cli
.hang:	hlt
	jmp .hang
.end:
