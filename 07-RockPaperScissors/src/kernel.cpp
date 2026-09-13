#include <stdint.h>
#include <stddef.h>

static void* memcpy(void* dest, const void* src, size_t count);

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

static void scroll() {
    memcpy(VGA_MEMORY,
           VGA_MEMORY + VGA_WIDTH,
           (VGA_HEIGHT - 1) * VGA_WIDTH * sizeof(uint16_t));
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
// --- New Code: 07 Rock Paper Scissors ---

static unsigned int rng_counter = 0; // A counter that we will use as our source of "randomness", as we don't have a real random number generator yet.

// While we wait for input, we will continue spinning the counter, and take the modulus 3 of it, or the remainder of dividing by 3.
// This value is global so it can keep changing between rounds. Real randomness will come at a later lesson.
static char read_move_key() {
    // A lot of getchar() code is reused, as getchar() doesn't return the time taken for the keystroke, we need to reuse it, but add that extra feature.
    while (true) {
        // Is there an input waiting, same code as getchar.
        if (inb(KEYBOARD_STATUS_PORT) & 1) {
            uint8_t scancode = inb(KEYBOARD_DATA_PORT); // Get scancode from data port.
            if (scancode & 0x80) continue;          // Ignore key releases.
            char ch = scancode_to_char[scancode];   // Get character from scancode.
            if (ch != 0) return ch;                 // We got a real key, lets return it.
        }
        rng_counter++;   // No key yet, spin the counter and check again.
    }
}
// Turn a move number, [0,1,2], into its printable name.
static const char* move_name(int move) {
    if (move == 0) return "Rock";
    if (move == 1) return "Paper";
    return "Scissors";
}

// --- New Code continued ---

// --- Kernel entry ---
extern "C" void kernel_main() {
    text_color = make_color(VGA_WHITE, VGA_BLACK);
    clear_screen();

    print("LeveretOS - Lesson 7: Rock Paper Scissors\n\n"); 
    print("Press r, p, or s to play a round.\n\n");   

    // Running score across rounds.
    int wins = 0, losses = 0, ties = 0;

    // Play forever, one round per loop, as scrolling (lesson 6) handles the screen filling up.
    while (true) {
        print("Your move (r/p/s): ");

        // Read one key, advancing the rng counter while we wait, then return the input to the display.
        char c = read_move_key();
        putchar(c);
        putchar('\n');

        // Convert the key to a move number; ignore anything that isn't r/p/s.
        int player_input;
        if (c == 'r' || c == 'R') player_input = 0;
        else if (c == 'p' || c == 'P') player_input = 1;
        else if (c == 's' || c == 'S') player_input = 2;
        else {
            print("Press r, p, or s.\n\n");
            continue;
        }

        // The computer's move is just the counter mod(remainder) 3.
        int cpu_input = rng_counter % 3;

        print("You: ");   print(move_name(player_input));
        print("   Me: "); print(move_name(cpu_input));
        putchar('\n');

        // Decide the winner with modular arithmetic. With 0=rock, 1=paper, 2=scissors,
        // each move beats the one before it in the cycle: paper beats rock, scissors beats paper, rock beats scissors. 
        // So player_input wins exactly when (player_input - cpu_input + 3) % 3 == 1.
        // The + 3 keeps the result positive before the % (player_input - cpu_input can be negative).
        if (player_input == cpu_input) {
            print("Tie!\n");
            ties++;
        } else if ((player_input - cpu_input + 3) % 3 == 1) {
            print("You win!\n");
            wins++;
        } else {
            print("I win!\n");
            losses++;
        }

        // Show the running score using print_int.
        print("Score - you: "); print_int(wins);
        print("  me: ");        print_int(losses);
        print("  ties: ");      print_int(ties);
        print("\n\n");
    }
}
