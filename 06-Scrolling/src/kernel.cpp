#include <stdint.h>
#include <stddef.h>

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

    print("LeveretOS - Lesson 5: string and memory helpers.\n\n");

    print("Length of \"LeveretOS\": ");
    print_int((int)strlen("LeveretOS"));
    putchar('\n');

    print("strcmp(\"abc\", \"abc\") = ");
    print_int(strcmp("abc", "abc"));
    print("   strcmp(\"abc\", \"abd\") = ");
    print_int(strcmp("abc", "abd"));
    putchar('\n');

    char buffer[16];
    memcpy(buffer, "copied!", 8);
    print("memcpy gives: ");
    print(buffer);
    putchar('\n');

    memset(buffer, 'X', 5);
    buffer[5] = '\0';
    print("memset gives: ");
    print(buffer);
    print("\n\n");

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
