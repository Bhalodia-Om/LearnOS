#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stddef.h>

// Declarations for keyboard input implemented in keyboard.cpp. Tells the compiler that these functions are defined elsewhere
char getchar();
void read_line(char* buffer, size_t max);

// The scancode to character table is defined in keyboard.cpp and declared here so the game's read_move_key (in kernel.cpp) can reuse it. "extern" = defined elsewhere.
extern const char scancode_to_char[128];

#endif // KEYBOARD_H
