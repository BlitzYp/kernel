#include <cstdint>

uint16_t* video_buffer = (uint16_t*) 0xb8000; // the start of video memory

uint64_t video_position = 0; // our position on the screen

// all vga colours
enum Color {
    BLACK = 0,
    GREEN = 2,
    RED = 4,
    YELLOW = 14,
    WHITE = 15
};

void clear() { // clear previous stuff from eg GRUB
    uint64_t otherplaceonscreen = 0;
    // in vga text mode, 25 lines exist, each stores 80 columns, and each character is two octets long (or 16 bits)
    while (otherplaceonscreen < (uint64_t)80*25*2) {
        video_buffer[otherplaceonscreen] = ' '; // set to blank char
        otherplaceonscreen += 2; // add the 2 octets
    }
}

static inline void putchar(uint8_t ch, Color color) {
  video_buffer[video_position++] = (color << 8) | ch;
}

void vga_write(const char* str, Color color) {
  while (*str) putchar(*str++, color);
}

extern "C" void start_kernel() { // entrypoint.
    clear();
    vga_write("hello world", Color::WHITE);
}
