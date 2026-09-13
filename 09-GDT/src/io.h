#ifndef IO_H
#define IO_H
// Include guard: the two lines above (and #endif at the bottom) stop this header from being pasted in twice if multiple files #include it. Every header uses this.

#include <stdint.h>

// Port I/O lives here because many parts of the OS need it (keyboard now; the PIC,
// timer, and PCI later). inb is inline(defined here) in the header so any file that includes io.h gets the actual code, no separate io.cpp needed.

// Read one byte from an I/O port.
static inline uint8_t inb(uint16_t port) {
    uint8_t result;
    asm volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

// The keyboard ports. Kept here so both keyboard.cpp and the game's read_move_key
// (in kernel.cpp) can reach them.
static const uint16_t KEYBOARD_DATA_PORT   = 0x60;
static const uint16_t KEYBOARD_STATUS_PORT = 0x64;

#endif // IO_H
