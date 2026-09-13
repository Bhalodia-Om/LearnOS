#include <stdint.h>
#include <stddef.h>

// --- VGA text mode  ---
static const size_t VGA_WIDTH  = 80;
static const size_t VGA_HEIGHT = 25;

static uint16_t* const VGA_MEMORY = reinterpret_cast<uint16_t*>(0xB8000);

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

static size_t cursor_row = 0;
static size_t cursor_col = 0;

static uint8_t text_color = 0;


static void clear_screen() {    
    for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        VGA_MEMORY[i] = make_entry(' ', text_color);  
    }
    cursor_row = 0;                                    
    cursor_col = 0;                                     
}

static void putchar(char c) { 
    if (c == '\b') {     // go back and delete the previous character upon backspace. This is incase the user makes a mistake.
        if (cursor_col > 0) { // If we are not at the left most column, move back a column.
            cursor_col--;  
        } else if (cursor_row > 0) { // If we are at the left most column, wrap back to the previous row.
            cursor_row--;   // Move up a row.
            cursor_col = VGA_WIDTH - 1;// take the position of the last character on the last row.
        }
        const size_t index = cursor_row * VGA_WIDTH + cursor_col; // Get the 1D equivalent of the 2D height and width
        VGA_MEMORY[index] = make_entry(' ', text_color); // Replace the character at this position with a blank space to erase the previous character.
        return;
    }

    if (c == '\n') {
        cursor_col = 0;
        cursor_row++;
        return;
    }

    const size_t index = cursor_row * VGA_WIDTH + cursor_col;
    VGA_MEMORY[index] = make_entry(c, text_color);

    cursor_col++;
    if (cursor_col >= VGA_WIDTH) {
        cursor_col = 0;
        cursor_row++;
    }
}

static void print(const char* str) {
    for (size_t i = 0; str[i] != '\0'; i++) {
        putchar(str[i]);
    }
}

// --- keyboard input ---
// inb (input byte) reads one byte from an I/O port, like a keyboard. Ports are at a separate address from ram, so we cannot use a normal pointer like VGA_MEMORY.
// Instead we drop into the inline assembly to run "in" instructions with the cpu, or inputs.
static inline uint8_t inb(uint16_t port) {
    uint8_t result; //an 8 bit variable to store the result of checking input.
    asm volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));    // asm (assembly) tells the compiler to run the code within the parantheses in assembly. 
    return result;                                              // "inb" tells the CPU to read a byte from an IO port.
    }                                                           // "%1" is a placeholder to tell the code that this is the first input.
                                                                // using more inputs can be done by increasing the number (%2, etc.)
                                                                // Input placeholders need a value to input. In this case it is "Nd"(port). It is seperated from the output by a ':'.
                                                                // More input placeholders can be given values by using commas to make a list after the first input.
                                                                // Our output placeholder is the "%0". There can only be one output, in this case "=a"(result).
                                                                // "=a", "=" means to only write the output, no reading. "a" means the data is retrived from the CPU. This is called an AL Accumulator register, or 8-bit.
                                                                // "Nd", "N" means to encode the value use as a hardcoded constant that will not change between calls.
                                                                // "Nd", "D" means that if the port is larger that 8-bits, the limit of the hardcoded constant, to use the DX Accumulator register, or 16-bit.
                                                                //(result) is the variable that the output will be written to.
                                                                //(port) is the variable the first input is taken from. The fact that this is an input is signified by how it comes after the second ':'.


// We now need to create variables for the keyboard ports.
static const uint16_t KEYBOARD_DATA_PORT   = 0x60;  // the scanned input.
static const uint16_t KEYBOARD_STATUS_PORT = 0x64;  // Status of the keyboard.

// Next we need to convert our scancode, or a identifying number for the physical key into a letter that we can use. For this we create an array that converts the scancode to letters.
// 0's signify keys we cannot handle, like action keys.
static const char scancode_to_char[128] = {     // as there are 58 entries, and the array has 128 slots, the rest are filled with 0.
    0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t','q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,   'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'','`',
    0,   '\\','z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
    0,   '*', 0,   ' ',
}; 

static char getchar() { // Get character from keyboard.
    while (true) {  // By running a forever loop, we can continue running the following code until we return due to an input.

        // The '&' is a comparison between 1(00000001) and the input byte. It compares the 0th(first) bit, and if they are both 1 we continue. 
        // A == would check the entire byte which may have other set values, and would throw off the check.
        // When the 0th byte is equal to one, that means there is a byte in the keyboard controller output. When true, that means there is a value for us to read.
        if (inb(KEYBOARD_STATUS_PORT) & 1) {

            uint8_t scancode = inb(KEYBOARD_DATA_PORT); // Grab the code provided by the input buffer.

            if (scancode & 0x80) { // Comparison between the last byte, represented by 0x80, or 10000000. If the last, or high bit is set, that means the key was released.
                continue; // Ignore releases and return to the start of the loop.
            }

            char c = scancode_to_char[scancode]; // convert the scancode to a character.
            if (c != 0) { // If the character is configured, return the character, otherwise continue the loop.
                return c;
            }
        }
    }
}
// this function will fill the buffer variable until the character presses enter, or the maximum characters is reached.
static void read_line(char* buffer, size_t max) {
    size_t length = 0;
    while (true) {
        char c = getchar(); // Wait until the next keyboard input.

        if (c == '\n') {    // If enter is the key received, move the cursor to the next line and stop reading.
            putchar('\n');
            break;  // end the loop.
        }

        if (c == '\b') {    
            if (length > 0) {
                length --;      // forget the last character in the buffer.
                putchar('\b');  // erase the last character on the screen.
            }                   // It is unnecessary to remove the character from the buffer because it will get overwritten by the next character or null terminate.
            continue; // continue the loop from the beginning.
        }

        if (length < max - 1) { // If we are not at the maximum length, add another character.
            buffer[length] = c; // add the character to the buffer variable that stores whats typed in.
            length++;   // Raise the length of the buffer by 1.
            putchar(c); // Print the character to the screen.
        }
    }

    buffer[length] = '\0';              // null-terminate at the end so it's a valid C string
}

extern "C" void kernel_main() {
    text_color = make_color(VGA_WHITE, VGA_BLACK);

    clear_screen();

    print("LeveretOS - Lesson 3: keyboard input.\n\n");

    print("What is your name? ");
    
    char name[64]; // Create a variable for the name that is an array of characters.
    read_line(name, 64); // Use our read_line function to get an input to name.

    print("Hello, ");
    print(name); //print the variable right after the previous print, as we did not move down a row.
    print("!\n");

    print("Press any key to reboot the computer...\n");
    getchar();
    print("...just kidding. Nothing happens. \n");

    // Challenge: everything types lowercase. Track the Shift key (scancode 0x2A press, 0xAA release)
    // and make letters uppercase while it's held. Hint: 'A' == 'a' - 0x20.
    // The solution to the challenge is present in the next lesson, 04
}
