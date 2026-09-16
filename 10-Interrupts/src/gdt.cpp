#include "gdt.h"
#include <stdint.h>

// One GDT(Global Descriptor Table) entry describes a memory segment, meaning where it starts, how big it is, and what permissions it has.
// The x86 layout split these fields across the struct, which bundles the fields into one named type.
// But it's in an awkward order, so the variable names are a little scattered.
// __attribute__((packed)) stops the compiler adding padding between fields, as the CPU expects these bytes laid out with no gaps.

struct GdtEntry { // This holds a structure of all the data points we needed for a memory segment.
    uint16_t limit_low;    // bits 0-15 of the segment limit (size)
    uint16_t base_low;     // bits 0-15 of the base address (start)
    uint8_t  base_mid;     // bits 16-23 of the base
    uint8_t  access;       // permission flags (code/data, privilege, present)
    uint8_t  granularity;  // high 4 bits of limit + size/granularity flags
    uint8_t  base_high;    // bits 24-31 of the base
} __attribute__((packed)); // No padding between variables.

// The Global Descriptor Table Register(GDTR) is the value we give the CPU's Load Global Descriptor Table(LGDT) instruction, or where to find the GDT.
// Its just the size of the table - 1, and a pointer to it in memory.

struct GdtPointer {
    uint16_t limit;        // Size of the table in bytes, minus 1.
    uint32_t base;         // Address of the first GdtEntry.
} __attribute__((packed));


static GdtEntry gdt[3]; // Use our struct from before, with 3 entries, null, code, data. The null entry is needed by the CPU.
static GdtPointer gdt_ptr; //Create a variable of struct GdtPointer

extern "C" void gdt_flush(uint32_t gdt_ptr_address); // Implemented in gdt_flush.s. Loads the GDT and reloads the segment registers.
                                                     // Segment registers define what parts of memory our code can read, write, or execute.
                                                     // extern "C", so that the assembly names match exactly without any changes by C++.

// Fill one GDT entry from plain numbers, split apart to fit the CPU's split apart layout.
static void gdt_set_entry(int i, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity) {
    gdt[i].base_low    = base & 0xFFFF;         // set base_low to the lowest 16 bits of the address. (0xFFFF means 16 bits), (0xFF means 8 bits)
    gdt[i].base_mid    = (base >> 16) & 0xFF;   // First shift everything right by 16 bits, skipping the bits stored in base_low, and store the next 8 bits.
    gdt[i].base_high   = (base >> 24) & 0xFF;   // Shift everything right by 24 bits, skipping the bits stored in base_low and base_mid and store the next 8 bits.

    gdt[i].limit_low   = limit & 0xFFFF; // limit is the segments size, or 20 bits, but the struct doesn't have a 20 bit field. 
                                         // So we split it into 2 places, being limit_low and 4 more bits in granularity. Doing & 0xFFFF compares where both bytes are 1.
                                         // This effectively keeps the first 16 bits.

    // top 4 bits of the 20-bit limit go in the low nibble(half a byte) of the granularity variable;
    // the high nibble holds the size/granularity flags, which tell the CPU to measure the segment in 4 KB blocks or single bytes.
    // We use 4 KB blocks because we want our segment to cover the full 4 GB, letting our small 20-bit number reach that far.
    gdt[i].granularity = ((limit >> 16) & 0x0F) | (granularity & 0xF0);

    gdt[i].access      = access;
}                                                     


// The access byte is a set of permission flags. The two we use differ by only one bit:
//   0x9A = 1001 1010  (code segment)
//   0x92 = 1001 0010  (data segment)
// The high nibble (1001) is the same for both: present (in use), ring 0 (kernel), normal segment.
// The low nibble is where they are different:
//   bit 3 = executable: 1 = code (0xA), 0 = data (0x2).
//   bit 1 = read/write: on a code segment it means "readable", on a data segment it means "writable".
//           Both have this bit set, so our code is readable and our data is writable.

void gdt_init() {
    gdt_ptr.limit = sizeof(gdt) - 1; // A pointer to our table we will give the CPU. the limit field is the size of the whole table in bytes - 1. sizeof(gdt) = 3 entries × 8 byte, or 24 bytes - 1.
                                     // We minus one so we give the address of the last valid byte, and not the total count.
    gdt_ptr.base  = (uint32_t)&gdt;  // Provide the memory address where the table starts. &gdt means the address of the gdt array, and we cast to a 32-bit number.
                                     // Now gdt_ptr holds both pieces the cpu needs, or how big the table is and its location.
    
    gdt_set_entry(0, 0, 0, 0, 0);    // Fills entry 0 with all zeros, or a null descriptor. The CPU needs the first entry to be null, as a safety rule.

    gdt_set_entry(1, 0, 0xFFFFF, 0x9A, 0xCF); // The code segment, where our program's instructions run. Starts at the beginning of the memory.
                                              // Covers the full 4 GB. 0x9A = present (in use), kernel level, readable code segment.

    gdt_set_entry(2, 0, 0xFFFFF, 0x92, 0xCF); // The data segment, where our variables and stacks are. Same range of 4 GB.
                                              // 0x92 = present, kernel level, writable data segment.

    gdt_flush((uint32_t)&gdt_ptr);            // Give the table to the CPU and reload segment registers, with the assembly function.
}