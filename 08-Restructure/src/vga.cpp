#include "vga.h"
#include "string.h"   // for memcpy, used by scroll

// The screen memory and the shared cursor/colour globals (declared extern in vga.h).
static uint16_t* const VGA_MEMORY = reinterpret_cast<uint16_t*>(0xB8000);

size_t cursor_row = 0;
size_t cursor_col = 0;
uint8_t text_color = 0;

void clear_screen() {
    for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        VGA_MEMORY[i] = make_entry(' ', text_color);
    }
    cursor_row = 0;
    cursor_col = 0;
}

// Internal to this file (not in the header), as only putchar uses it.
static void scroll() {
    memcpy(VGA_MEMORY,
           VGA_MEMORY + VGA_WIDTH,
           (VGA_HEIGHT - 1) * VGA_WIDTH * sizeof(uint16_t));
    for (size_t col = 0; col < VGA_WIDTH; col++) {
        VGA_MEMORY[(VGA_HEIGHT - 1) * VGA_WIDTH + col] = make_entry(' ', text_color);
    }
}

void putchar(char c) {
    if (c == '\b') {
        if (cursor_col > 0) {
            cursor_col--;
        } else if (cursor_row > 0) {
            cursor_row--;
            cursor_col = VGA_WIDTH - 1;
        }
        const size_t index = cursor_row * VGA_WIDTH + cursor_col;
        VGA_MEMORY[index] = make_entry(' ', text_color);
        return;
    }

    if (c == '\n') {
        cursor_col = 0;
        cursor_row++;
        if (cursor_row >= VGA_HEIGHT) {
            scroll();
            cursor_row = VGA_HEIGHT - 1;
        }
        return;
    }

    const size_t index = cursor_row * VGA_WIDTH + cursor_col;
    VGA_MEMORY[index] = make_entry(c, text_color);

    cursor_col++;
    if (cursor_col >= VGA_WIDTH) {
        cursor_col = 0;
        cursor_row++;
        if (cursor_row >= VGA_HEIGHT) {
            scroll();
            cursor_row = VGA_HEIGHT - 1;
        }
    }
}

void print(const char* str) {
    for (size_t i = 0; str[i] != '\0'; i++) {
        putchar(str[i]);
    }
}

void print_int(int value) {
    if (value == 0) {
        putchar('0');
        return;
    }

    if (value < 0) {
        putchar('-');
        value = -value;
    }

    char digits[12];
    int count = 0;

    while (value != 0) {
        digits[count] = '0' + (value % 10);
        value = value / 10;
        count++;
    }

    for(int i = count - 1; i >= 0; i--) {
        putchar(digits[i]);
    }
}
