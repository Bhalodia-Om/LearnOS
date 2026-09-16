#include "vga.h"
#include "gdt.h"      
#include "idt.h"       // our interrupt table

// --- Kernel entry ---
extern "C" void kernel_main() {
    gdt_init();       
    idt_init();        // install our interrupt table

    text_color = make_color(VGA_WHITE, VGA_BLACK);
    clear_screen();

    print("LeveretOS - Lesson 10: Interrupts (IDT)\n\n");

    // Trigger a divide-by-zero on purpose. Instead of the machine silently rebooting, our exception handler should catch it and print "Divide by zero".
    print("About to divide by zero...\n");
    volatile int a = 10;
    volatile int b = 0;
    volatile int c = a / b;
    (void)c;   // stops the compiler warning about an unused variable. "Use" it with void
}
