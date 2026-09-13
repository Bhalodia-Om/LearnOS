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

static bool shift_held = false;

static void clear_screen() {    
    for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        VGA_MEMORY[i] = make_entry(' ', text_color);  
    }
    cursor_row = 0;                                    
    cursor_col = 0;                                     
}

static void putchar(char c) { 
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
// --- Lesson 4: Printing numbers ---
// print_int turns an integer into text and prints it, as our print only handles strings. Passing a number would lead to passing the character with such number code.
static void print_int(int value) {
    // Special case for 0. If the value of the integer is 0, the following code would print nothing.
    if (value == 0) {
        putchar('0');
        return;
    }

    // Special case for negatives. Print a minus sign, and work with the positive version so the following code can still be used.
    if (value < 0) {
        putchar('-');
        value = -value;
    }

    // hold the digits as separate characters in a buffer. A 32-bit integer (anything longer is classified as a long in c++) is 10 digits, or 11 digits for negatives consider the sign
    // so have an array with 12 spaces leaves extra space. 
    // This is filled back to front, due to the next issue.
    char digits[12];
    int count = 0;

    // Extract digits with % and /. '%' gives us the last digit, "/ 10" divides the number by ten.
    // Example: 426 -> 6, then 42 -> 2, then 4. Notice that the numbers taken come out backwards, with 6 first. 
    while (value != 0) {
        digits[count] = '0' + (value % 10); // get the last (ones) digit from our int. adding a char, '0' converts our digits into its respective digit. 
                                            // This works by getting the characters respective integer value by adding the difference from 0 to the character of 0.
        value = value / 10; //Drop our tens digit down to our last. decimals are dropped as integers need to be whole numbers.
        count++;
    }

    for(int i = count - 1; i >= 0; i--) { //print our digits buffer back to front to account for our numbers being backward from before.
        putchar(digits[i]);
    }
}
// --- end of new code block, new code is continued in kernel_main

// --- keyboard input ---
static inline uint8_t inb(uint16_t port) {
    uint8_t result;
    asm volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

static const uint16_t KEYBOARD_DATA_PORT   = 0x60;
static const uint16_t KEYBOARD_STATUS_PORT = 0x64;

static const char scancode_to_char[128] = {
    0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t','q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,   'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'','`',
    0,   '\\','z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
    0,   '*', 0,   ' ',
}; 

static char getchar() {
    while (true) {
        if (inb(KEYBOARD_STATUS_PORT) & 1) {

            uint8_t scancode = inb(KEYBOARD_DATA_PORT);

            //solution to challenge
            if (scancode == 0x2A) { // if shift is pressed, set shift_held to true, using the keyboard scancode 0x2A for pressing.
                shift_held = true;
                continue;
            }
            if (scancode == 0xAA) { // if shift is release, set shift_held to false, using the keyboard scancode 0xAA for releasing of shift key.
                shift_held = false;
                continue;
            }

            if (scancode & 0x80) {
                continue;
            }

            char c = scancode_to_char[scancode];
            if (c != 0) {
                 if (shift_held && c >= 'a' && c <= 'z') { // solution to challenge: if shift is held and the character is a lowercase letter, convert it to uppercase.
                    c = c - 0x20;
                }
                return c;
            }
        }
    }
}

static void read_line(char* buffer, size_t max) {
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

extern "C" void kernel_main() {
    text_color = make_color(VGA_WHITE, VGA_BLACK);

    clear_screen();

    print("LeveretOS - Lesson 4: printing numbers.\n\n");

    print("A positive number: ");
    print_int(12345);
    putchar('\n');

    print("Zero: ");
    print_int(0);
    putchar('\n');

    print("A negative number: ");
    print_int(-42);
    putchar('\n');

    print("Math result (6 * 7): ");
    print_int(6 * 7); //calculation done within the equation.
    putchar('\n');
}
