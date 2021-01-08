#include <cstdint>
#include "../libs/blitz_stdio.h"
#include "vga.h"

#define TERM_HIGHT 25
#define TERM_WIDTH 80

#ifdef WINDOWS
#error This is not windows you idiot
#endif

uint16_t* video_buffer = (uint16_t*) 0xb8000; // the start of video memory
uint64_t video_position = 0; // our position on the screen
int row, col, color;
char lines[TERM_HIGHT][TERM_WIDTH]; 

static inline uint8_t create_vga_unit(char chr, Color col) {
  return chr | col << 8;
}

static inline void strcpy(char* dest, const char* src) {
  while (*src)
    *dest++ = *src++;
}

// initialize the terminal
void init() {
  row = 0;
  col = 0;
  color = Color::VGA_COLOR_CYAN | Color::VGA_COLOR_BLACK << 4;
  for (uint8_t i = 0; i < TERM_HIGHT * TERM_WIDTH * 2; i++) {
    video_buffer[i] = create_vga_unit(' ', (Color)color);
  }
}

static inline void handle_geometry() {
  if (++col == TERM_WIDTH) col = 0;
  if (++row == TERM_HIGHT) {
    char* upper_most = lines[0];
      // scroll logic here
  }
}

static inline void putchar(uint8_t ch, Color color) {
  video_buffer[video_position++] = create_vga_unit(ch, color);
}

static inline uint8_t strlen(const char* src) {
  uint8_t i = 0;
  for (;*src++ != '\0';i++);
  return i; 
}

void vga_write(const char* str, Color color) {
  while (*str) putchar(*str++, color);
  strcpy(lines[row], "");
  handle_geometry();
}

// entrypoint.
extern "C" void start_kernel() {
  init();
  vga_write("hello world", Color::VGA_COLOR_WHITE);
}
