#ifndef VGA_H
#define VGA_H

#include <stdint.h>
#include <stddef.h>

// Declarations for the screen output implemented in vga.cpp.
// Small helpers (make_color, make_entry) stay inline here, but the bigger functions are declared here and defined in vga.cpp.

static const size_t VGA_WIDTH  = 80;
static const size_t VGA_HEIGHT = 25;

enum VgaColor : uint8_t {
    VGA_BLACK = 0,
    VGA_WHITE = 15,
};

static inline uint8_t make_color(uint8_t fg, uint8_t bg) {
    return fg | (bg << 4);
}

static inline uint16_t make_entry(char c, uint8_t color) {
    return static_cast<uint16_t>(c) | (static_cast<uint16_t>(color) << 8);
}

// Shared globals: DEFINED in vga.cpp, DECLARED here with "extern" so every file uses the same single copy instead of making its own.
extern size_t cursor_row;
extern size_t cursor_col;
extern uint8_t text_color;

void clear_screen();
void putchar(char c);
void print(const char* str);
void print_int(int value);

#endif // VGA_H
