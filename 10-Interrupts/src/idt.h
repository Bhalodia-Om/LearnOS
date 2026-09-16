#ifndef IDT_H
#define IDT_H

// The IDT (Interrupt Descriptor Table) is the sibling of the GDT. The IDT tells the CPU what to do when an error happens.
// When interrupt X happens, run the handler for interrupt for X.
// There are 256 slots (0-255). Slots 0-31 will be CPU exceptions, like dividing by zero, which is what this lesson will handle.
// idt_init() will build the table and load it, so instead of rebooting on an error, our own code will run instead.

void idt_init();

#endif // IDT_H