.text

# multiboot spec
.align 4
.long 0x1BADB002 # magic number.
.long 0 # flags to multiboot, currently none
.long -(0x1BADB002 + 0) # checksum so that multiboot knows everything is ok 👍

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
    hlt 

.bss
.skip 8192
stack_space:
 
