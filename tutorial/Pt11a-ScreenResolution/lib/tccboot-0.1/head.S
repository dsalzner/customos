/*
 *  linux/boot/head.S
 *
 *  Copyright (C) 1991, 1992, 1993  Linus Torvalds
 */

/*
 *  head.S contains the 32-bit startup code.
 *
 * NOTE!!! Startup happens at absolute address 0x00001000, which is also where
 * the page directory will exist. The startup code will be overwritten by
 * the page directory. [According to comments etc elsewhere on a compressed
 * kernel it will end up at 0x1000 + 1Mb I hope so as I assume this. - AC]
 *
 * Page 0 is deliberately kept safe, since System Management Mode code in
 * laptops may need to access the BIOS data stored there.  This is also
 * useful for future device drivers that either access the BIOS via VM86
 * mode.
 */

/*
 * High loaded stuff by Hans Lermen & Werner Almesberger, Feb. 1996
 */
.text

#include <linux/linkage.h>
#include <asm/segment.h>

	.globl startup_32

startup_32:
	cld
	cli
	movl $(__KERNEL_DS),%eax
	movl %eax,%ds
	movl %eax,%es
	movl %eax,%fs
	movl %eax,%gs

	lss SYMBOL_NAME(stack_start),%esp
	xorl %eax,%eax
1:	incl %eax		# check that A20 really IS enabled
	movl %eax,0x000000	# loop forever if it isn't
	cmpl %eax,0x100000
	je 1b

/*
 * Initialize eflags.  Some BIOS's leave bits like NT set.  This would
 * confuse the debugger if this code is traced.
 * XXX - best to initialize before switching to protected mode.
 */
	pushl $0
	popfl
/*
 * Clear BSS
 */
	xorl %eax,%eax
	movl $ SYMBOL_NAME(_edata),%edi
	movl $ SYMBOL_NAME(_end),%ecx
	subl %edi,%ecx
	cld
	rep
	stosb
/*
 * Do the decompression, and jump to the new kernel..
 */
	subl $16,%esp	# place for structure on the stack
	movl %esp,%eax
	pushl %esi	# real mode pointer as second arg
	pushl %eax	# address of structure as first arg
	call SYMBOL_NAME(compile_kernel)
	orl  %eax,%eax
	jnz  3f
	popl %esi	# discard address
	popl %esi	# real mode pointer
	xorl %ebx,%ebx
	ljmp $(__KERNEL_CS), $0x100000

/*
 * We come here, if we were loaded high.
 * We need to move the move-in-place routine down to 0x1000
 * and then start it with the buffer addresses in registers,
 * which we got from the stack.
 */
3:
	movl $move_routine_start,%esi
	movl $0x1000,%edi
	movl $move_routine_end,%ecx
	subl %esi,%ecx
	addl $3,%ecx
	shrl $2,%ecx
	cld
	rep
	movsl

	popl %esi	# discard the address
	popl %ebx	# real mode pointer
	popl %esi	# low_buffer_start
	popl %ecx	# lcount
	popl %edx	# high_buffer_start
	popl %eax	# hcount
	movl $0x100000,%edi
	cli		# make sure we don't get interrupted
	ljmp $(__KERNEL_CS), $0x1000 # and jump to the move routine

/*
 * Routine (template) for moving the decompressed kernel in place,
 * if we were high loaded. This _must_ PIC-code !
 */
move_routine_start:
	movl %ecx,%ebp
	shrl $2,%ecx
	rep
	movsl
	movl %ebp,%ecx
	andl $3,%ecx
	rep
	movsb
	movl %edx,%esi
	movl %eax,%ecx	# NOTE: rep movsb won't move if %ecx == 0
	addl $3,%ecx
	shrl $2,%ecx
	rep
	movsl
	movl %ebx,%esi	# Restore setup pointer
	xorl %ebx,%ebx
	ljmp $(__KERNEL_CS), $0x100000
move_routine_end:
