#include "qwerty_keymap.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include "vga.h"

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define IDT_SIZE 256

void keyboard_handler(void);
int8_t read_port(unsigned short port);
void write_port(unsigned short port, uint8_t data);
void load_idt(unsigned long *idt_ptr);

// sometimes compilers complain, sometimes they don't.
// but these are stub functions to call.
void __stack_chk_fail() {}
void __stack_chk_fail_local() {}
void __stack_chk_guard() {}

/* video memory begins at address 0xb8000 */
int8_t *vidptr = (int8_t *)0xb8000;

struct IDTEntry {
  uint16_t offset_lowerbits;
  uint16_t selector;
  uint8_t zero;
  uint8_t type_attr;
  uint16_t offset_higherbits;
};

struct IDTEntry IDT[IDT_SIZE];

enum RETURN_CODES { // can be handled by the assembly later, returned by
                    // start_kernel
  HALT = 0,         // all ok, halt cpu
  REBOOT = 1,       // perform reboot
  SHUTDOWN = 2,     // shutdown
  NOTHING = 3,      // nothing
  PANIC = 4,        // panic was already thrown, so this prints some descriptive
                    // message and hangs
  START = 5         // the kernel should start again
};


int init_idt() {
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
  idt_ptr[0] =
      ((sizeof(struct IDTEntry)) * 256) + ((idt_address & 0xffff) << 16);
  idt_ptr[1] = idt_address >> 16;

  load_idt(idt_ptr);

  return 0;
}

int init_ps2_keyboard() {
  // 0xFD corresponds to IRQ1, we can read this for keyboard interrupts
  write_port(0x21, 0xFD);
  return 0;
}


char stuff[512] = {0};
bool shift_down;

size_t calcsize() {
  size_t counter = 0;
  for (size_t y = 0; y < 511; y++) {
    if (stuff[y] != 0) {
      counter++;
    }
  }
  return counter;
}


bool strequ(const char *l, const char *r) {
  for (; *l == *r && *l; l++, r++)
    ;
  return *l == *r;
}

void command_parse(const char *command) {
  if (strequ(command, "test1")) {
    vga_write("success.", VGA_COLOR_WHITE);
    vga_write_newline();
  } else if (strequ(command, "test2")) {
    vga_write("going down the chain?...", VGA_COLOR_WHITE);
    vga_write_newline();
  } else {
    vga_write(command, VGA_COLOR_WHITE);
  }
}
void keyboard_handler_main() {

  // send an End-Of-Interrupt (0x20) to port 0x20.
  // not issuing an EOI makes it think we are still reading
  // the buffer. therefore we do not recieve more interrupts.
  // this tells the interrupts controller that we are done
  // (we will still be able to read from the port)

  write_port(0x20, 0x20);

  uint8_t status = read_port(KEYBOARD_STATUS_PORT);
  uint8_t keycode;
  size_t i;

  // first bit, 0x01 (or 1) of the status from the keyboard status port 0x64
  // will be set to 1 if the buffer of text is not empty; in other words, a key
  // has been pressed
  if (status & 0x01) {
    keycode = read_port(
        KEYBOARD_DATA_PORT); // read the ascii keycode from the dataport 0x60
    if (keycode == 0x2a || keycode == 0x36) { // Lshift or Rshift down
      shift_down = true;
      return;
    }
    if (keycode == 0xaa || keycode == 0xb6) { // Lshift or Rshift release
      shift_down = false;
      return;
    }
    // ignore garbage
    if (keycode & 128) return;

    if (keyboard_map[keycode] == '\n') { // hit enter, so handle it because this will be a shell later on. all of this should be abstracted away into userspace.
      vga_write_newline();
      command_parse(stuff);
      // zero it out
      for (i = 0; i < 511; i++) {
        stuff[i] = 0;
      }
      return;
    }
    if (keyboard_map[(uint8_t)keycode] == '\b') { // handle backspaces
      vga_backspace();
      stuff[calcsize() - 1] = 0;
      return;  // quick return, or it will try to print nothignness.
    }

    // print the character
    if (shift_down) {
      vga_putchar(shift_map[(uint8_t)keycode], VGA_COLOR_LIGHT_CYAN);
      stuff[calcsize()] = shift_map[keycode];
    } else {
      vga_putchar(keyboard_map[(uint8_t)keycode], VGA_COLOR_LIGHT_CYAN);
      stuff[calcsize()] = keyboard_map[keycode];
    }
  }
}

/* service represents a service in the system.
name: the name of the service. stored as a const char*
init: the function to call when this service is initialized, stored as a void
pointer.
*/
struct service {
  const char *name;
  int (*init)();
};

int load_service(struct service serv) {
  int ret = serv.init();
  vga_write("Loading ", VGA_COLOR_GREEN);
  vga_write(serv.name, VGA_COLOR_GREEN);
  vga_write_newline();
  return ret;
}

uint8_t _start() {
  // create each service
  struct service vga_textmode = {
      "VGA Text Mode",
      vga_initialize,
  };
  struct service idt = {
      "Interrupt Descriptor Table",
      init_idt,
  };
  struct service ps2_keyboard = {
      "PS/2 keyboard driver (US QWERTY only)",
      init_ps2_keyboard,
  };

  // mass-initialize them
  load_service(vga_textmode);
  load_service(idt);
  load_service(ps2_keyboard);

  // test vga textmode driver
  vga_write("Hello world", VGA_COLOR_WHITE);
  vga_write_newline();

  while (1) {
  }
  return (uint8_t)HALT;
}
