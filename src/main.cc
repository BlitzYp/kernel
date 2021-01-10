#include <cstdint>
#include <cstddef>
#include "keymap.h"

uint16_t* terminal_buffer = (uint16_t*) 0xB8000;

extern unsigned char keyboard_map[128];

extern "C" void keyboard_handler(void);
extern "C" char read_port(unsigned short port);
extern "C" void write_port(unsigned short port, unsigned char data);
extern "C" void load_idt(unsigned long *idt_ptr);

static const uint16_t VGA_WIDTH = 80;
static const uint16_t VGA_HEIGHT = 25;

enum VGA_COLOR {
	BLACK = 0,
	BLUE = 1,
	GREEN = 2,
	CYAN = 3,
	RED = 4,
	MAGENTA = 5,
	BROWN = 6,
	LIGHT_GREY = 7,
	DARK_GREY = 8,
	LIGHT_BLUE = 9,
	LIGHT_GREEN = 10,
	LIGHT_CYAN = 11,
	LIGHT_RED = 12,
	LIGHT_MAGENTA = 13,
	LIGHT_BROWN = 14,
	WHITE = 15
};

enum RETURN_CODES { // can be handled by the assembly later, returned by start_kernel
    HALT = 0, // all ok, halt cpu
    REBOOT = 1, // perform reboot
    SHUTDOWN = 2, // shutdown
    NOTHING = 3, // nothing
    PANIC = 4, // panic was already thrown, so this prints some descriptive message and hangs
    START = 5 // the kernel should start again
};


// IDT stuff; an IDT is an interrupt descriptor table, we can use it for recieving interrupts.
// partially taken from osdev.

size_t vga_index;

static inline uint8_t vga_entry_color(VGA_COLOR fg, VGA_COLOR bg) {
	return fg | bg << 4;
}
 
static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
	return (uint16_t) uc | (uint16_t) color << 8;
}

void init_vga_textmode() {
	size_t terminal_color = vga_entry_color(VGA_COLOR::WHITE, VGA_COLOR::BLACK); // the terminal will be white on black. recall to change color...
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}

void vga_write(const char *str, VGA_COLOR fg) {
    size_t index = 0;
    while (str[index]) {
            terminal_buffer[vga_index] =  (unsigned short)str[index]|(unsigned short)fg << 8; 
            index++;
            vga_index++;
    }
}

void vga_putchar(const unsigned char character, VGA_COLOR fg) {
    terminal_buffer[vga_index] = (unsigned short)character | (unsigned short)fg << 8;
}


extern "C" void keyboard_handler_main() {
	unsigned char status;
	char keycode;

	// write end of interrupt, so that we can recieve more.
	write_port(0x20, 0x20);

	// read the keyboard status port at 0x64
	status = read_port(0x64);
	// first bit (0x01) will only be set if the buffer is not empty, aka input has been recieved
	if (status & 0x01) {
		keycode = read_port(0x60); // read the keyboard data port at 0x60
		if (keycode < 0) // idk what the fuck your keyboard is lmao
			return;

		if (keycode == 0x1C) { // enter
			vga_index += 80; // add newline
			return;
		}

        vga_putchar(keyboard_map[(unsigned char) keycode], VGA_COLOR::LIGHT_CYAN); // write char to keyboard. in a bright and bold color to differentiate.
	}
}

struct IDTEntry {
	unsigned short int offset_lowerbits;
	unsigned short int selector;
	unsigned char zero;
	unsigned char type_attr;
	unsigned short int offset_higherbits;
};

IDTEntry IDT[256]; // make a 256 long IDT.


void idt_init() {
	unsigned long keyboard_address;
	unsigned long idt_address;
	unsigned long idt_ptr[2];

	// get address of the keyboard handler function...
	keyboard_address = (unsigned long)keyboard_handler;
	// and register yourself, ask for it to call that function...
	IDT[0x21].offset_lowerbits = keyboard_address & 0xffff; 
	IDT[0x21].selector = 0x08;
	IDT[0x21].zero = 0;
	IDT[0x21].type_attr = 0x8e;
	IDT[0x21].offset_higherbits = (keyboard_address & 0xffff0000) >> 16;

	/*     Ports
	*	 PIC1	PIC2
	*Command 0x20	0xA0
	*Data	 0x21	0xA1
	*/

	// almost all of this writing and reading from ports is from https://pdos.csail.mit.edu/6.828/2014/readings/hardware/8259A.pdf. 
	// very useful resource

	// start everything
	write_port(0x20, 0x11);
	write_port(0xA0, 0x11);


	// if we are in protected mode, which we will be in the future we need to remap
	// the PICs because the first 32 (0x20 == 32) are reserved. so increase
	// the offset. so set icw2.
	write_port(0x21, 0x20);
	write_port(0xA1, 0x28);

	// setup listening for interrupts with icw3.
	write_port(0x21, 0x00);
	write_port(0xA1, 0x00);

	// get info about the environment by writing to icw4.
	write_port(0x21, 0x01);
	write_port(0xA1, 0x01);
	// finished initalizing.

	// mask all interrupts with 0xff
	write_port(0x21, 0xff);
	write_port(0xA1, 0xff);

	// populate the IDT descriptor
	idt_address = (unsigned long)IDT;
	idt_ptr[0] = ((sizeof (IDTEntry)) * 256) + ((idt_address & 0xffff) << 16);
	idt_ptr[1] = idt_address >> 16;

	load_idt(idt_ptr);
}

extern "C" uint8_t start_kernel() {
    idt_init();
    vga_index = 0;
    init_vga_textmode();
    vga_write("testing...", VGA_COLOR::WHITE);
    return (uint8_t) RETURN_CODES::HALT;
}
