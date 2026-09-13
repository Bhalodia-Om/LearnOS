#include <stdint.h>
#include <stddef.h>

// --- New Code ---
// This is a forward declaration, which promises the compiler that memcpy exists, and leave the full definition where it is. 
// We use forward declarations when we want the code to recognize that we have this function, but want to actually put the function after, where calls to it can't see it.
static void* memcpy(void* dest, const void* src, size_t count);
// --- End Of New Code. New Code continued in scroll() ---

// --- VGA text mode ---
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

// --- Screen output ---
static void clear_screen() {
    for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        VGA_MEMORY[i] = make_entry(' ', text_color);
    }
    cursor_row = 0;
    cursor_col = 0;
}

// --- New Code: Scrolling ---
// scrolling will move every row up by one and make the bottom row blank. We will call this function when text would go past the last line.
// This way the screen doesn't write off the end of the VGA memory.
// We copy rows 1 up into the row above, and repeat for each line. 
// As memcpy counts bytes, multiply the cell count by sizeof(uint16_t), or the size of a 16 bit integer, which we use for a single character of text.
// Our following lines are safe, as this code only overwrites lines above it, and we start from the top.
static void scroll() {
    memcpy(VGA_MEMORY,  // destination is the start of row 0, or the topmost row.
           VGA_MEMORY + VGA_WIDTH, // source is the start of row 0 + the width of the display, or the start of row 1.
           (VGA_HEIGHT - 1) * VGA_WIDTH * sizeof(uint16_t)); // The count of bytes to copy is all the cells on the screen minus the 1st row,
                                                             // multiplied by the size in bytes of each cell.
    // Make the last row blank to overwrite the old text that was there.
    for (size_t col = 0; col < VGA_WIDTH; col++) {
        VGA_MEMORY[(VGA_HEIGHT - 1) * VGA_WIDTH + col] = make_entry(' ', text_color);
    }
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
        if (cursor_row >= VGA_HEIGHT) {     // The new cursor_row ran off the bottom of the screen.
            scroll();                       // Use the shift function to shift everything up.
            cursor_row = VGA_HEIGHT - 1;    // Realign the cursor with the last row.
        }
        return;
    }

    const size_t index = cursor_row * VGA_WIDTH + cursor_col;
    VGA_MEMORY[index] = make_entry(c, text_color);

    cursor_col++;
    if (cursor_col >= VGA_WIDTH) {
        cursor_col = 0;
        cursor_row++;
        if (cursor_row >= VGA_HEIGHT) {     // The new line wrap ran off the bottom of the screen.
            scroll();                       // Use the shift function to shift everything up.
            cursor_row = VGA_HEIGHT - 1;    // Realign the cursor with the last row.
        }
    }
}
// --- End of new code. New code continued in kernel_main() ---
static void print(const char* str) {
    for (size_t i = 0; str[i] != '\0'; i++) {
        putchar(str[i]);
    }
}

// --- Number printing ---
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

// --- String and memory helpers ---
static size_t strlen(const char* str) {
    size_t length = 0;
    while (str[length] != '\0') {
        length++;
    }
    return length;
}

static int strcmp(const char* a, const char* b) {
    size_t i = 0;
    while (a[i] != '\0' && a[i] == b[i]) {
        i++;
    }
    return (int)a[i] - (int)b[i];
}

static void* memcpy(void* dest, const void* src, size_t count) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    for (size_t i = 0; i < count; i++) {
        d[i] = s[i];
    }
    return dest;
}

static void* memset(void* dest, uint8_t value, size_t count) {
    uint8_t* d = (uint8_t*)dest;
    for (size_t i = 0; i < count; i++) {
        d[i] = value;
    }
    return dest;
}

static int atoi(const char* str) {
    int result = 0;
    int sign = 1;
    size_t i = 0;

    if (str[0] == '-') {
        sign = -1;
        i = 1;
    }

    while (str[i] >= '0' && str[i] <= '9') {
        result = result * 10 + (str[i] - '0');
        i++;
    }

    return result * sign;
}

// --- Keyboard input ---
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

// --- Kernel entry ---
extern "C" void kernel_main() {
    text_color = make_color(VGA_WHITE, VGA_BLACK);
    clear_screen();

    print("LeveretOS - Lesson 6: scrolling.\n\n");   // [NEW] lesson 6 banner

    // Print more lines than the screen has rows (25). Without scrolling this would write past the bottom of VGA memory.
    // Now the screen scrolls and shows the most recent lines.
    for (int i = 1; i <= 40; i++) {
        print("Line ");
        print_int(i);
        putchar('\n');
    }

    print("Done, the earliest lines scrolled off the display!\n");
}
