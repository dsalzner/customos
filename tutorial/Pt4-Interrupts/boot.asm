bits	32
section 	.text
	align	4
	dd	0x1BADB002
	dd	0x00
	dd	- (0x1BADB002+0x00)

global start
extern kernel_main
start:
	sti
	mov    esp,0x4000
	sti ; need this twice for no apparant reason?
	call kernel_main
	cli
	hlt
.Lhang:
	jmp .Lhang

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

%include "../isr.asm"
%include "../irq.asm"
