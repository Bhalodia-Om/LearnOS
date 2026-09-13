#include "vga.h"
#include "keyboard.h"
#include "io.h" 
#include "gdt.h"       // Our new Global Descriptor Table

// --- Rock Paper Scissors ---

static unsigned int rng_counter = 0;

static char read_move_key() {
    while (true) {
        if (inb(KEYBOARD_STATUS_PORT) & 1) {
            uint8_t scancode = inb(KEYBOARD_DATA_PORT);
            if (scancode & 0x80) continue;
            char ch = scancode_to_char[scancode];
            if (ch != 0) return ch;
        }
        rng_counter++;
    }
}

static const char* move_name(int move) {
    if (move == 0) return "Rock";
    if (move == 1) return "Paper";
    return "Scissors";
}

// --- Kernel entry ---
extern "C" void kernel_main() {
    gdt_init();        // Install our own GDT before doing anything else, overridding the GRUB configurations.

    text_color = make_color(VGA_WHITE, VGA_BLACK);
    clear_screen();

    print("LeveretOS - Lesson 9: GDT (There is no visible change from lesson 7, but we are using our own segment table!)\n\n"); 
    print("Press r, p, or s to play a round.\n\n");   

    int wins = 0, losses = 0, ties = 0;

    while (true) {
        print("Your move (r/p/s): ");

        char c = read_move_key();
        putchar(c);
        putchar('\n');

        int player_input;
        if (c == 'r' || c == 'R') player_input = 0;
        else if (c == 'p' || c == 'P') player_input = 1;
        else if (c == 's' || c == 'S') player_input = 2;
        else {
            print("Press r, p, or s.\n\n");
            continue;
        }

        int cpu_input = rng_counter % 3;

        print("You: ");   print(move_name(player_input));
        print("   Me: "); print(move_name(cpu_input));
        putchar('\n');

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

        print("Score - you: "); print_int(wins);
        print("  me: ");        print_int(losses);
        print("  ties: ");      print_int(ties);
        print("\n\n");
    }
}
