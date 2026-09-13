; This assembly code corresponds with "gdt_flush(uint32_t gdt_ptr_address)", in gdt.cpp
; Loads the GDT, then reloads all the segment registers so the CPU starts using it.
; This has to be assembly: the CS (code segment) register can't be set with a normal mov(a copy), it needs a "far jump", which C++ can't do.
global gdt_flush

gdt_flush:
    mov eax, [esp + 4]   ; grab the argument (address of our GdtPointer) off the stack
    lgdt [eax]           ; tell the CPU the location of the new GDT.

; Reload the DATA segment registers with 0x10 (entry 2 is our data segment).
    ; (entry index 2 * 8 bytes per entry = 0x10.)
    mov ax, 0x10         ; Point the data segment registers to entry 2, the data segment.
    mov ds, ax           ; data segment
    mov es, ax           ; extra segment
    mov fs, ax           ; extra segments (fs/gs are general-purpose)
    mov gs, ax
    mov ss, ax           ; stack segment

    ; Reload CS (code segment) = 0x08 (entry 1 is our code segment).
    ; CS can't be set with mov, so a far jump to the new selector reloads it and lands execution on the .flush label just below.
    jmp 0x08:.flush      ; The far jump
.flush:                  ; Sets the name for the stop after the jump.
    ret                  ; return to gdt_init() in gdt.cpp