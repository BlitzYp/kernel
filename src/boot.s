
.set MULTIBOOT_MAGIC,       0x1BADB002 # magic shit
.set MULTIBOOT_ALIGN,       0x00000001 # align all boot modules
.set MULTIBOOT_MEMINFO,     0x00000002 # tell the kernel about memory
.set MULTIBOOT_FLAGS,       MULTIBOOT_ALIGN | MULTIBOOT_MEMINFO # xor the flags
.set MULTIBOOT_CHECKSUM, -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS) # checksum

.section .multiboot
.align 4 # alignment
multiboot_header:
    .int 0x1BADB002 # magick
    .int 1 | 2 # flags
    .int -(0x1BADB002 + (1 | 2))

.section .text

# exports to C++:
.global start
.global keyboard_handler
.global read_port
.global write_port
.global load_idt

# imports from C++:
.extern _start 
.extern keyboard_handler_main

read_port:
    movl 4(%esp), %edx
    in %dx, %al 
    ret

write_port: 
    movl 4(%esp), %edx
    movb 4 + 4(%esp), %al
    outb %al, %dx
    ret

load_idt:
    movl 4(%esp), %edx
    lidt (%edx)
    sti 
    ret

keyboard_handler:                 
    call keyboard_handler_main # cheat and write it in C :tm:
    iretl

start:
    cli 
    movl $stack_space, %esp
    call _start
    cli
    jmp hang

hang:
    hlt
    jmp hang

.section .bss
.skip 8192
stack_space:
