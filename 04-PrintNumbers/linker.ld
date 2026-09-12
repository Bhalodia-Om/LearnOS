/* <- signifies a comment. */
/* The linker file merges different sections into one final file. This overrides the default layout */
/* The linker decides what order sections go in, and their memory addresses. It also decides where the kernel should be loaded in memory when running. */
/* A kernel is the core computer program that bridges software applications and the physical hardware. */

ENTRY(_start)           /*Sets the first code to run. In this case the _start section in boot.s */

SECTIONS
{
        /* Start placing sections, starting at the 1 MB mark, as the 1st MB is reserved for BIOS/VGA/etc., so our kernel loads at 0x00100000 (1 MB). */

    . = 1M;             /* the '.' is a special symbol called the "location counter". It tracks the current address as we write sections to ram. Setting it moves the "cursor" for writing to RAM. */
                        /* the M here represents MB, and the K represents KB. */ 

    /* --- Code section --- */

        /* .multiboot must come first so the bootloader can find the header in the first 8 KB, so that this file is recognized as the kernel file. */

    .text ALIGN(4K) : /* Builds a new .text section that is aligned by a multiple 4KB, in a new clean boundary.  */
    {
        *(.multiboot)   /* the * means gather all sections in all files of name .multiboot and write them here. */
        *(.text)        /* the * means gather all sections in all files  of name .text and write them here. */
    }

    /* --- Read-only data (constants) --- */

    .rodata ALIGN(4K) : /* create a new .rodata(read-only data) section aligned to 4KB, after the .text section. */
    {
        *(.rodata)      /* get all sections in all files of name .rodata and write them here. */
    }

    /* --- Initialized data (variables with starting values) --- */
    .data ALIGN(4K) :   /* create a new .data section aligned to 4KB, after the .text section. */
    {
        *(.data)
    }

    /* --- Uninitialized data (the stack) --- */
    .bss ALIGN(4K) :
    {
        *(COMMON)       /* Common symbols, such as uninitialized global variables, like integers. */
        *(.bss)         /* A dedicated section of computer memory used for storing global or static variables that are without an initial value. From boot.s */
    }
}