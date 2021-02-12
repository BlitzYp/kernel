#include "keymap.hpp"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define ENTER_KEY_CODE 0x1C
#define IDT_SIZE 256

extern uint8_t keyboard_map[128];
void keyboard_handler(void);
int8_t read_port(unsigned short port);
void write_port(unsigned short port, uint8_t data);
void load_idt(unsigned long *idt_ptr);

void __stack_chk_fail() {}
void __stack_chk_fail_local() {}
void __stack_chk_guard() {}

/* video memory begins at address 0xb8000 */
int8_t *vidptr = (int8_t *)0xb8000;

typedef struct
{
	uint16_t offset_lowerbits;
	uint16_t selector;
	uint8_t zero;
	uint8_t type_attr;
	uint16_t offset_higherbits;
} IDTEntry;

IDTEntry IDT[IDT_SIZE];

uint16_t *terminal_buffer = (uint16_t *)0xB8000;

static const uint16_t VGA_WIDTH = 80;
static const uint16_t VGA_HEIGHT = 25;

typedef enum 
{
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
} VGA_COLOR;

enum RETURN_CODES
{				  // can be handled by the assembly later, returned by start_kernel
	HALT = 0,	  // all ok, halt cpu
	REBOOT = 1,	  // perform reboot
	SHUTDOWN = 2, // shutdown
	NOTHING = 3,  // nothing
	PANIC = 4,	  // panic was already thrown, so this prints some descriptive message and hangs
	START = 5	  // the kernel should start again
};

size_t vga_index;

static inline uint8_t vga_entry_color(VGA_COLOR fg, VGA_COLOR bg)
{
	return fg | bg << 4;
}

static inline uint16_t vga_entry(uint8_t uc, uint8_t color)
{
	return (uint16_t)uc | (uint16_t)color << 8;
}

void init_vga_textmode()
{
	size_t terminal_color = vga_entry_color(WHITE, BLACK); // the terminal will be white on black. recall to change color...
	for (size_t y = 0; y < VGA_HEIGHT; y++)
	{
		for (size_t x = 0; x < VGA_WIDTH; x++)
		{
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}

void vga_write(const char *str, VGA_COLOR fg)
{
	size_t index = 0;
	while (str[index])
	{
		terminal_buffer[vga_index] = (unsigned short)str[index] | (unsigned short)fg << 8;
		index++;
		vga_index++;
	}
}

void vga_putchar(const char character, VGA_COLOR fg)
{
	terminal_buffer[vga_index] = (unsigned short)character | (unsigned short)fg << 8;
	vga_index++;
}

void init_idt()
{
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

	// almost all of this writing and reading from ports is from
	// https://pdos.csail.mit.edu/6.828/2014/readings/hardware/8259A.pdf. very
	// useful resource

	// start everything
	write_port(0x20, 0x11);
	write_port(0xA0, 0x11);

	// if we are in protected mode, which we will be in the future we need to
	// remap the PICs because the first 32 (0x20 == 32) are reserved. so increase
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
	idt_ptr[0] = ((sizeof(IDTEntry)) * 256) + ((idt_address & 0xffff) << 16);
	idt_ptr[1] = idt_address >> 16;

	load_idt(idt_ptr);
}

void keyboard_irq1_init()
{
	// 0xFD corresponds to IRQ1, we can read this for keyboard interrupts
	write_port(0x21, 0xFD);
}

void vga_write_newline()
{
	size_t line_size = 80 * 2;
	vga_index = vga_index + (line_size - vga_index % (line_size));
}

void vga_init()
{
	unsigned int i = 0;
	while (i < 80 * 25 * 2)
	{
		vidptr[i++] = ' ';
		vidptr[i++] = 0x07;
	}
}

char stuff[512] = {0};

size_t calcsize()
{
	size_t counter = 0;
	for (size_t y = 0; y < 511; y++)
	{
		if (stuff[y] != 0)
		{
			counter++;
		}
	}
	return counter;
}

size_t strlen(const char *str)
{
	size_t x = 0;
	while (*str)
		x++;
	return x;
}

bool strequ(const char *l, const char *r)
{
	for (; *l == *r && *l; l++, r++)
		;
	return *l == *r;
}

void command_parse(const char *command)
{
	if (strequ(command, "bloat"))
	{
		vga_write("Congrats, you have successfully bloated; nice.", WHITE);
	}
	else if (strequ(command, "morebloat"))
	{
		vga_write("More bloat? Ok then...", WHITE);
	}
	else
	{
		vga_write(command, WHITE);
	}
}

void keyboard_handler_main()
{

	// send an End-Of-Interrupt (0x20) to port 0x20.
	// not issuing an EOI makes it think we are still reading
	// the buffer. therefore we do not recieve more interrupts.
	// this tells the interrupts controller that we are done
	// (we will still be able to read from the port)

	write_port(0x20, 0x20);

	uint8_t status = read_port(KEYBOARD_STATUS_PORT);
	int8_t keycode;
	size_t i;

	// first bit, 0x01 (or 1) of the status from the keyboard status port 0x64 will be set to 1 if the buffer of text is not empty;
	// in other words, a key has been pressed
	if (status & 0x01)
	{
		keycode = read_port(KEYBOARD_DATA_PORT); // read the ascii keycode from the dataport 0x60
		if (keycode < 0)
			return;
		if (keycode == ENTER_KEY_CODE)
		{ // hit enter, so add a newline
			char _stuff[512];
			for (i = 0; i < 511; i++)
			{
				_stuff[i] = keyboard_map[(uint8_t)stuff[i]];
			}
			command_parse(_stuff);
			// zero it out
			for (i = 0; i < 511; i++)
			{
				stuff[i] = 0;
			}
			return;
		}
		if (keyboard_map[(uint8_t)keycode] == '\b') { // handle backspaces
			terminal_buffer[--vga_index] = ' ';
			return;
		}
		// print the character
		vga_putchar(keyboard_map[(uint8_t)keycode], LIGHT_CYAN);
		stuff[calcsize()] = keycode;
		// if we don't add an ascii bell, it triple faults or results in some very weird behavior.
		// im probably doing something wrong here, lol.
	}
}

uint8_t _start()
{
	init_vga_textmode();
	vga_write("Loaded VGA Text Mode Driver.", GREEN);
	vga_write_newline();
	init_idt();
	vga_write("Loaded IDT.", GREEN);
	vga_write_newline();
	keyboard_irq1_init();
	vga_write("Loaded PS/2 Keyboard driver.", GREEN);
	vga_write_newline();
	
	while (1)
	{
	}
	return (uint8_t)HALT;
}
