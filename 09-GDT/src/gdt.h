#ifndef GDT_H
#define GDT_H

// The GDT (Global Descriptor Table) tells the CPU how memory is divided into "segments" and what each one is allowed to do (code vs data, which privilege level). 
// We need to build our own so later lessons (interrupts) have a code segment we can control. gdt_init will set it all up and load it.
void gdt_init(); // Declare this here for gdt.cpp

#endif // GDT_H
