#include "keyboard.h"
#include "io.h"    // inb and the keyboard ports
#include "vga.h"   // putchar (display typed characters)

// shift_held is internal to the keyboard, nothing else needs it, so it stays here.
static bool shift_held = false;

// The scancode to character table (declared extern(defined elsewhere) in keyboard.h so read_move_key can use it).
const char scancode_to_char[128] = {
    0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t','q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,   'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'','`',
    0,   '\\','z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
    0,   '*', 0,   ' ',
};

char getchar() {
    while (true) {
        if (inb(KEYBOARD_STATUS_PORT) & 1) {

            uint8_t scancode = inb(KEYBOARD_DATA_PORT);

            if (scancode == 0x2A) {
                shift_held = true;
                continue;
            }
            if (scancode == 0xAA) {
                shift_held = false;
                continue;
            }

            if (scancode & 0x80) {
                continue;
            }

            char c = scancode_to_char[scancode];
            if (c != 0) {
                 if (shift_held && c >= 'a' && c <= 'z') {
                    c = c - 0x20;
                }
                return c;
            }
        }
    }
}

void read_line(char* buffer, size_t max) {
    size_t length = 0;
    while (true) {
        char c = getchar();

        if (c == '\n') {
            putchar('\n');
            break;
        }

        if (c == '\b') {
            if (length > 0) {
                length --;
                putchar('\b');
            }
            continue;
        }

        if (length < max - 1) {
            buffer[length] = c;
            length++;
            putchar(c);
        }
    }

    buffer[length] = '\0';
}
