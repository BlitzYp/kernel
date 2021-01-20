#pragma once
#include <cstddef>
#include <cstdint>
static const uint16_t VGA_WIDTH = 80;
static const uint16_t VGA_HEIGHT = 25;
uint16_t* terminal_buffer = (uint16_t *)0xB8000;
int8_t *vidptr = (int8_t *)0xb8000;

size_t current_loc = 0;

enum VGA_COLOR
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
    size_t terminal_color = vga_entry_color(VGA_COLOR::WHITE, VGA_COLOR::BLACK); // the terminal will be white on black. recall to change color...
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

void vga_write_newline()
{
    size_t line_size = 80 * 2;
    current_loc = current_loc + (line_size - current_loc % (line_size));
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
