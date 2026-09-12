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

static size_t cursor_row = 0;   // Tracks which row and column the new character will be written to. In lesson one, position was only needed for the "Hello World" for loop.
static size_t cursor_col = 0;   // When having multiple print calls, all must be able to access the row and column of the cursor so that we don't print above another message.

                                // Limitation: no scrolling yet. Past row 25 this writes beyond the screen. A later lesson will add scrolling.

static uint8_t text_color = 0;  // Set text color globally so messages share the same color. Static means this variable is private to this file.


// Use one function for clearing the whole screen so said function can be used multiple times. use text_color instead of a local color like lesson 1. At the end we reset cursor position to (0,0).
static void clear_screen() {    
    for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        VGA_MEMORY[i] = make_entry(' ', text_color);  
    }
    cursor_row = 0;                                    
    cursor_col = 0;                                     
}

//putchar is used to write a character at the cursor and then advance the cursor. A core building block for later print messages. 
static void putchar(char c) {   
    //if our character is '\n', or newline, push the cursor to the next line, like clicking enter on the keyboard when writing.
    if (c == '\n') {
        cursor_col = 0;
        cursor_row++;
        return; // return so the rest of the function doesn't run.
    }

    // Convert our 2D position into the 1 dimensional index of VGA_MEMORY. Each row is VGA_WIDTH cells wide, so we need to find the area of the row count (cursor_row) * row width (VGA_WIDTH).
    // We then add the leftover distance that we are from the first column, using cursor_col.

    const size_t index = cursor_row * VGA_WIDTH + cursor_col;
    VGA_MEMORY[index] = make_entry(c, text_color); //print our character to memory at that position.

    //move the cursor one column right. If we passed the right edge, wrap to the next line.
    cursor_col++;
    if (cursor_col >= VGA_WIDTH) {
        cursor_col = 0;
        cursor_row++;
    }
}

static void print(const char* str) {    //the variable is not passed, but a pointer to its location in RAM is passed. Signified by the '*'. str represents the string, passed as a row of characters.
                                        // a '\0' is a null terminator, or the character at the end of our string.  
    for (size_t i = 0; str[i] != '\0'; i++) {
        putchar(str[i]); //run our putchar function to put a character where the cursor is.
    }
}

// --- Write to screen using our new functions ---
extern "C" void kernel_main() {
    text_color = make_color(VGA_WHITE, VGA_BLACK); // white text, black background. Now using our global color variable instead of a local variable.

    clear_screen();  // use our clear screen function to clear the screen.

    // Unlike lesson one, we can now print out multiple messages without overlap. We can even move to the next line using '\n'

    // Challenge: Write your favorite quote out using our new print command.

    print("LearnOS - Lesson 2: text output using a cursor.\n");

    print("My Favorite Quote: ");
    print("Beware of bugs in the above code; I have only proved it correct, not tried it.\n");
    print("                                                                - Donald Knuth\n"); 
    // Challenge: tabs don't work yet. Try implementing that yourself. Each '\t' must move the cursor a few positions to the right. Or just use spaces...

}
