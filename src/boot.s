; contains code meant for the NASM assembler for booting our operating system, and passing info to multiboot, which is used by grub
BITS 32
section .multiboot ; for multiboot
    dd 0x1BADB002 ; magic number
    dd 0 ; boot flags, we have none
    dd - (0x1BADB002 + 0) ; calculate their checksum

section .text ; our code

; make them global
global start
global keyboard_handler
global read_port
global write_port
global load_idt

; start_kernel starts it, keyboard_handler_main is a c function that is delegated to
extern keyboard_handler_main
extern start_kernel

; here stack values are from CDECL.

read_port:
	mov edx, [esp + 4] ; move val from stack into edx
	in al, dx ; al is lower 8 bits of eax, dx is lower 16 of edx if the osdev manual is right. this will read in to the register 
	ret

write_port:
	mov edx, [esp + 4] ; move val from stack into edx
	mov al, [esp + 4 + 4] ; move val from stack into al
	out dx, al ; write to the port with the stack value in al
	ret

load_idt:
	mov edx, [esp + 4] ; move it into edx
	lidt [edx] ; load the idt
	sti ; turn on interrupts, reverse cli
	ret ; return

keyboard_handler:                 
	call keyboard_handler_main ; go to c
	iretd ; interrupt return instead of normal return. i think iret should work too 


start:
    mov esp, our_stack ; move our stack to the stack pointer esp
    cli ; block interrupts
    call start_kernel ; call the start_kernel function imported from C
    hlt ; halt cpu so it doesnt glitch badly

section .bss ; stack space
resb 8192 ; the 8KB of stack
our_stack: ; empty block, where our stack will be stored
