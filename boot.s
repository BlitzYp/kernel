; contains code meant for the NASM assembler for booting our operating system, and passing info to multiboot, which is used by grub
BITS 64
section .multiboot ; for multiboot
    dd 0x1BADB002 ; magic number
    dd 0 ; boot flags, we have none
    dd - (0x1BADB002 + 0) ; calculate their checksum

section .text ; for our code
global start
extern start_kernel
start:
    cli ; clear all the hardware interrupts, from GRUB
    mov esp, our_stack ; move our stack to the stack pointer esp
    call start_kernel ; call the start_kernel function imported from C

section .bss ; stack space
resb 8192 ; the 8KB of stack
our_stack: ; empty block, where our stack will be stored
