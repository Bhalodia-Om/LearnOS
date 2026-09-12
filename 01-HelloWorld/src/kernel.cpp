// <- signifies a comment.
// kernel.cpp runs asfter boot.s calls kernel_main()
// by reading to memory starting at address 0xB8000, we can write text directly to the screen.
// VGA stands for Video Graphics Array, or the for of communication from the computer to the monitor. This can either be in text mode or display mode.

#include <stdint.h>   // defines fixed-width integer types: uint8_t, uint16_t, etc. The amount of bits that an integer is given to use.
#include <stddef.h>   // defines size_t (an unsigned type for sizes/indices). Creates unsigned(positive only) variables with a size of 32-bits, due to x86/32 bit cpu.
                      // for a x64/64 bits cpu, this would be 64 bits.

// --- VGA text mode  ---
    // The screen is a grid of 80 columns x 25 rows of character cells.
static const size_t VGA_WIDTH  = 80;    //static means only visible within this file, avoiding issues with naming later.
static const size_t VGA_HEIGHT = 25;

    // create a constant 16 bit integer that points to the memory address of the screen. By writing here we can create text on screen.
    // A 16 bit integer is split into low and high bit. The low bytes represents the character, and the high byte represents the color.

    // bit position:  15 14 13 12 11 10 9 8 | 7 6 5 4 3 2 1 0
    //               \_____ HIGH byte _____/ \___ LOW byte __/
    //               (bits 8-15)              (bits 0-7)

static uint16_t* const VGA_MEMORY = reinterpret_cast<uint16_t*>(0xB8000); //The * means that this variable is not equal to anything, but points to the location of its value.

    // There are 16 vga colors. For now we will only define the ones we need.
    // enum allows us to assign readable names to the integers that represent each color. We assign the integer value for the colors to constant integers of length 8.

enum VgaColor : uint8_t {   
    VGA_BLACK = 0,
    VGA_WHITE = 15,
};

    // create a function that returns a 8 bit color byte. This is made up of the foreground color in the low 4 bits, and background color in high 4 bits.
    // The use of 8 bits for color constants instead of 4 bit constants is due to 8 bit contstants being the smallest possible.
    // static shows that this function is local to this file. Inline asks the compiler to directly replace the name make_color with the code present.
static inline uint8_t make_color(uint8_t fg, uint8_t bg) {  // static means this fuction is only able to be accessed in this file. Inline means replace calls with this code. Good for small functions.
    return fg | (bg << 4); // write fg to the start of the byte, and offset bg by four bits
}

    // Make a full 16-bit cell entry, with the character as the low byte, and color in the high byte
static inline uint16_t make_entry(char c, uint8_t color) {
    return static_cast<uint16_t>(c) | (static_cast<uint16_t>(color) << 8);  // static_cast casts the value in the brackets(character or constant).                                                                                                                                                                  
}                                                                           // It is casted into the value in the <>, or in this case a 16 bit constant.
                                                                            // The constant for color is then offset by a byte and added together to create one 16 bit cell.                                            
                                                                            // return returns variable of type "uint16_t" to the caller, defined on line 40. 
                                                                            // This can be any variable type like an int, as long as the item returned matches

    // This is the entry point that boot.s calls. extern "C" stops c++ from renaming it internally, so "kernel_main" matches exactly what boot.s expects.                                                                       
extern "C" void kernel_main() {     //void means no output
    uint8_t color = make_color(VGA_WHITE, VGA_BLACK);  // white text, black bg

        // Step 1: we clear the whole screen by filling every cell with a blank space.
    for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {                   // loop through each cell, and write to the memory in that cell. Each cell is 16 bits/2 bytes. 
        VGA_MEMORY[i] = make_entry(' ', color);                             // move forward i cells and write a blank character to that cell.
    }

        // Step 2: We now write our message into the first cells of the screen.
    const char* message = "Hello world.";
    for (size_t i = 0; message[i] != '\0'; i++) {                           // loop through the message until reaching a '\0' , or a null terminator byte at the end.
        VGA_MEMORY[i] = make_entry(message[i], color);                      // Write the message character by character using the color set above.
    }

    // with no more code, kernel_main resturns, and boot.s hangs with rthe .hang loop.
}
