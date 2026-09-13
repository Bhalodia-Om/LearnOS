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
static void print_int(int value) {
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

// --- Lesson 5: String and Memory helpers ---

// strlen: (string length) count the characters in a string, not counting the '\0' terminator. Same loop as print(), but taking count instead.
static size_t strlen(const char* str) {
    size_t length = 0;
    while (str[length] != '\0') {
        length++;
    }
    return length;
}

// strcmp: (string compare) compare the two strings, returning 0 if they are identical. If they differ, return the difference between the first mismatched characters.
// This value is negative if string is smaller, positive if it is bigger.
static int strcmp(const char* a, const char* b) {
    size_t i = 0; // start checking from the first character.
    // Advance to the next character in both arrays if the characters checked are the same.
    while (a[i] != '\0' && a[i] == b[i]) { //if the array ends or there is a difference in character, continue to the return statement.
        i++;
    }
    // Return the difference of the first mismatched character. If both hit '\0' together it's 0 and our arrays are the same. 
    // As we are taking in a character array we need to first cast our character to an integer so that we can subtract the values.
    return (int)a[i] - (int)b[i];
}

// memcpy: (memory copy) Copy exact bytes in memory from the source to the destination. Unlike string functions, it does not stop at '\0'.
// We use void* pointers for uint8_t, or a pointer to take 1 byte of data at a time, no matter what type the data is.
// void* means allow any value, unlike void, which means return nothing. This also applies for variables, allowing us to copy anything.
static void* memcpy(void* dest, const void* src, size_t count) {
    uint8_t* d = (uint8_t*)dest;            // by creating a pointer to the memory address of dest itself, we can now use math or array brackets with the location of 32-bits.
                                            // We can't do this with a void* variable with no type, as the compiler doesn't know how to use it.                           
    const uint8_t* s = (const uint8_t*)src; // This allows us to take in any form of input, but then point to the raw bytes. Same for the source.
                                            // Note that these variables do not hold new data, just a pointer to the data. Changes here reflect to the values of dest also.
    for (size_t i = 0; i < count; i++) { // copy the value from the source to the destination byte by byte.
        d[i] = s[i];
    }
    return dest; // Returning the destination allows any calls to immediately use the returned value.
}

// memset: (memory set) Fill a count of bytes with the value of the single byte value. Great for zeroing buffers before use. Returns the destination.
static void* memset(void* dest, uint8_t value, size_t count) {
    uint8_t* d = (uint8_t*)dest;    // same idea here as memcpy
    for (size_t i = 0; i < count; i++) { // replace the number of bytes specified in count.
        d[i] = value;
    }
    return dest;
}

// atoi: ("ASCII to integer") Turn a string in its number equivalent. This is an inverse of print_int, which broke a number apart with % 10 and / 10. 
// atoi instead pushes characters forward/builds up with * 10 instead. 
static int atoi(const char* str) {
    int result = 0;
    int sign = 1;   // a multiplier for positive/negatives.
    size_t i = 0;

    // Handle a possible leading minus sign, then step past it.
    if (str[0] == '-') {
        sign = -1;
        i = 1;
    }

    // Walk through the digits, shifting the running total left one place (* 10) and adding the new digit.
    // Stop at the first character that isn't 0-9 (including '\0').
    while (str[i] >= '0' && str[i] <= '9') {
        result = result * 10 + (str[i] - '0'); // by subtracting '0', we get the value of the integer, and inverse to lesson 4.
        i++;
    }

    return result * sign; // use our multiplier for sign changes.
}

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

    print("LeveretOS - Lesson 5: string and memory helpers.\n\n");

    // strlen
    print("Length of \"LeveretOS\": "); // the '\' shows that the following " is not the end of the print command, but something to be printed.
    print_int((int)strlen("LeveretOS"));
    putchar('\n');

    // strcmp, compare strings "abc" and "abd"
    print("strcmp(\"abc\", \"abc\") = ");
    print_int(strcmp("abc", "abc"));            //Should return 0
    print("   strcmp(\"abc\", \"abd\") = ");
    print_int(strcmp("abc", "abd"));            //Should return the difference of c and d, or -1.
    putchar('\n');

    // memcpy
    char buffer[16];
    memcpy(buffer, "copied!", 8);   // 7 characters plus the '\0'. Copy our string to the buffer, and print it.
    print("memcpy gives: ");
    print(buffer);
    putchar('\n');

    // memset
    memset(buffer, 'X', 5);
    buffer[5] = '\0';               // terminate so print stops after the 5 X's
    print("memset gives: ");
    print(buffer);
    print("\n\n");

    // atoi: read two numbers and add them
    char a[16];
    char b[16];

    print("Enter a number: ");
    read_line(a, 16);

    print("Enter another:  ");
    read_line(b, 16);

    print("Sum: ");
    print_int(atoi(a) + atoi(b));
    putchar('\n');
}
// Challenge: Use atoi to create a pythagorean theorem calculator.
// Hint: For square roots, look into the Babylonian method of square roots.
