#include "idt.h"
#include "vga.h"      // so the handler can print
#include <stdint.h>

// One IDT entry, also called a gate, describes one handler. This involves where it is, and how the cpu should enter the "gate".
// Like the GDT, the address is split into two halves accross the struct, and packed, removing padding, bytes, so the CPU can see the exact byte layout.

struct IdtEntry {
    uint16_t offset_low;   // bits 0-15 of the handler's address
    uint16_t selector;     // which GDT code segment to run the handler in (0x08, our code segment)
    uint8_t  always_zero;  // reserved, must be 0
    uint8_t  flags;        // type + privilege + "present" bit (we use 0x8E)
    uint16_t offset_high;  // bits 16-31 of the handler's address
} __attribute__((packed)); // Remove padding bits

// The IDTR: What we hand the CPU's lidt (Load Interrupt Descriptor Table) instruction, or where to find our IDT.
// This is just the table's size - 1 and its address. Same as GDTPointer from lesson 9.

struct IdtPointer {
    uint16_t limit;     // size - 1
    uint32_t base;      // address
} __attribute__((packed));

// The table itself, which is 256 slots. We will only fill 0-31, or the CPU exceptions, this lesson.

static IdtEntry idt[256];
static IdtPointer idt_ptr;

// Fill one IDT slot. The handler is equal to the address of the assembly stub for the interrupt.

static void idt_set_entry(int i, uint32_t handler, uint16_t selector, uint8_t flags) {
    idt[i].offset_low  = handler & 0xFFFF;          // low 16 bits of the address
    idt[i].offset_high = (handler >> 16) & 0xFFFF;  // high 16 bits
    idt[i].selector    = selector;                  // 0x08 = our code segment in the GDT
    idt[i].always_zero = 0;
    idt[i].flags       = flags;                     // 0x8E = present, ring 0 (Highest Privelage), 32-bit interrupt gate (Run the code as 32-bits, and disable interrupts while handling).
}

// The exception names, indexed by their number, so the handler can print which one fired.
// The error and its number are set by the CPU creator, or in this case Intel. A reserved slot is currently not in use by the CPU, but saved for future CPUs
static const char* exception_names[32] = {
    "Divide by zero", "Debug", "Non-maskable interrupt", "Breakpoint",
    "Overflow", "Bound range exceeded", "Invalid opcode", "Device not available",
    "Double fault", "Coprocessor segment overrun", "Invalid TSS", "Segment not present",
    "Stack-segment fault", "General protection fault", "Page fault", "Reserved",
    "x87 floating-point", "Alignment check", "Machine check", "SIMD floating-point",
    "Virtualization", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved", "Security", "Reserved"
};

// Every assembly stub in idt_stubs.s will jump here and pass the exception number. We print which one happened, then stop, as for now they are unrecoverable. 
// Extern "C" so the assembly can call it by this exact name. (Remember name-mangling in C++)

extern "C" void exception_handler(uint32_t exception_number) {
    print("\n*** CPU EXCEPTION ***\n");
    if (exception_number < 32) {
        print(exception_names[exception_number]);
    } else {
        print("Unknown");
    }
    print("\nSystem halted.\n");

    // Stop the CPU: cli will disable interrupts, hlt will halt the cpu, the loop keeps our CPU stopped, avoiding a reboot.
    for (;;) {
        asm volatile ("cli; hlt");
    }
}

// The 32 assembly stubs, one per exception, defined in idt_stubs.s. We use their addresses to fill the table. extern "C" so the names match the assembly exactly.

extern "C" {
    void isr0();  void isr1();  void isr2();  void isr3();
    void isr4();  void isr5();  void isr6();  void isr7();
    void isr8();  void isr9();  void isr10(); void isr11();
    void isr12(); void isr13(); void isr14(); void isr15();
    void isr16(); void isr17(); void isr18(); void isr19();
    void isr20(); void isr21(); void isr22(); void isr23();
    void isr24(); void isr25(); void isr26(); void isr27();
    void isr28(); void isr29(); void isr30(); void isr31();
}

void idt_init() {
    idt_ptr.limit = sizeof(idt) - 1;   // size of the whole table in bytes, minus 1
    idt_ptr.base  = (uint32_t)&idt;    // address of the table

    // Fill our 32 exception slots. Each slot points at its assembly stub, running in our code segment.
    // (0x08) is our code segment. 0x8E has the settings of(present(use this exception), ring 0(kernel level), 32-bit interrupt gate. Turn off interrupts during the handler).
    // Call our previous function to set our exceptions. We also use our table from before.
    idt_set_entry(0,  (uint32_t)isr0,  0x08, 0x8E);
    idt_set_entry(1,  (uint32_t)isr1,  0x08, 0x8E);
    idt_set_entry(2,  (uint32_t)isr2,  0x08, 0x8E);
    idt_set_entry(3,  (uint32_t)isr3,  0x08, 0x8E);
    idt_set_entry(4,  (uint32_t)isr4,  0x08, 0x8E);
    idt_set_entry(5,  (uint32_t)isr5,  0x08, 0x8E);
    idt_set_entry(6,  (uint32_t)isr6,  0x08, 0x8E);
    idt_set_entry(7,  (uint32_t)isr7,  0x08, 0x8E);
    idt_set_entry(8,  (uint32_t)isr8,  0x08, 0x8E);
    idt_set_entry(9,  (uint32_t)isr9,  0x08, 0x8E);
    idt_set_entry(10, (uint32_t)isr10, 0x08, 0x8E);
    idt_set_entry(11, (uint32_t)isr11, 0x08, 0x8E);
    idt_set_entry(12, (uint32_t)isr12, 0x08, 0x8E);
    idt_set_entry(13, (uint32_t)isr13, 0x08, 0x8E);
    idt_set_entry(14, (uint32_t)isr14, 0x08, 0x8E);
    idt_set_entry(15, (uint32_t)isr15, 0x08, 0x8E);
    idt_set_entry(16, (uint32_t)isr16, 0x08, 0x8E);
    idt_set_entry(17, (uint32_t)isr17, 0x08, 0x8E);
    idt_set_entry(18, (uint32_t)isr18, 0x08, 0x8E);
    idt_set_entry(19, (uint32_t)isr19, 0x08, 0x8E);
    idt_set_entry(20, (uint32_t)isr20, 0x08, 0x8E);
    idt_set_entry(21, (uint32_t)isr21, 0x08, 0x8E);
    idt_set_entry(22, (uint32_t)isr22, 0x08, 0x8E);
    idt_set_entry(23, (uint32_t)isr23, 0x08, 0x8E);
    idt_set_entry(24, (uint32_t)isr24, 0x08, 0x8E);
    idt_set_entry(25, (uint32_t)isr25, 0x08, 0x8E);
    idt_set_entry(26, (uint32_t)isr26, 0x08, 0x8E);
    idt_set_entry(27, (uint32_t)isr27, 0x08, 0x8E);
    idt_set_entry(28, (uint32_t)isr28, 0x08, 0x8E);
    idt_set_entry(29, (uint32_t)isr29, 0x08, 0x8E);
    idt_set_entry(30, (uint32_t)isr30, 0x08, 0x8E);
    idt_set_entry(31, (uint32_t)isr31, 0x08, 0x8E);

    // Hand the table to the CPU with the lidt instruction.
    asm volatile ("lidt %0" : : "m"(idt_ptr));
    // lidt, or what we are setting.
    // %0 is a placeholder address filled by the compiler
    // "m"(idt_ptr) defines what %0 actually is. "m" means in memory, and idt_ptr is a variable that the compiler will use.
}