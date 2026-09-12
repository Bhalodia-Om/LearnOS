; <- signifies a comment.
; boot.s is the first file that the bootloader will run to hand control to the OS. It is written in assembly as there is no compiler set up for c++.
; A kernel is the core computer program that bridges software applications and the physical hardware.
; bit equal one switch of 1 and 0, byte equals a collection of 8 bits, representing a character, number or any value.
; ---------------------------------------------------------------------------
; MULTIBOOT HEADER CONSTANTS
; ---------------------------------------------------------------------------

MBALIGN equ 1 << 0                  ; MBALIGN means a request to split RAM space into boundaries of 4096 bytes, so each new module starts on a new boundary.
                                    ; By turning on bit 0, the code turns on MBALIGN.
                                    ; equ names a constant, the assembler will then replace all places with the constant.
                                    ; This constant is replaced with the value set here when the file is built.
                                    ; MBALIGN is a constant


MEMINFO equ 1 << 1                  ; MEMINFO asks for info about the RAM allocation, and how much and what is usable by the OS.
                                    ; By turning on bit 1, the OS requests the location of RAM that is usable.
                                    ; This returns mem_lower, or continuous memory up to 640 KB of RAM.
                                    ; This also returns mem_upper, or continuous RAM above 1 MB.'
                                    ; MEMINFO is a constant

MBFLAGS  equ MBALIGN | MEMINFO      ; This code sets the constant MBFLAGS equal to MBALIGN or MEMINFO. 
                                    ; OR(|) in this case refers to aligning both numbers and comparing them. If either bit is 1, the new bit is 1.
                                    ; For this case, MBALIGN equals 0000001, and MEMINFO equals 00000010, so OR'd, MBFLAGS equals 00000011.

MAGIC equ 0x1BADB002                ; Sets the magic number so that the bootloader is able to identify this file as the operating system kernel.
                                    ; The bootloader first starts up our OS.

CHECKSUM equ - (MAGIC + MBFLAGS)    ; A check to make sure that the magic number is correct. 
                                    ; When the bootloader loads the kernel, it will add CHECKSUM + MAGIC + MBFLAGS. This must equal 0x00000000, or 0.
                                    ; This prevents crashing due to file corruption.
                                    ; CHECKSUM equals negative of MAGIC + MBFLAGS. So when the bootload adds MAGIC + MBFLAGS, it returns to 0.

; ---------------------------------------------------------------------------
; THE MULTIBOOT HEADER (actual data placed in the binary)
; ---------------------------------------------------------------------------
; This code is in its own section (defined by section) so that our linker script, or the directions of compiling the OS can run this code first.

section .multiboot                  ; groups the following code into section .multiboot

align 4                             ; Align the multiboot header, or the file identifier to a 4 byte boundary, so that the bootloader can find it.

    dd MAGIC                        ; dd = "define doubleword", or a 32-bit value. Writes a 32-bit value into the binary here.
    dd MBFLAGS                      ; our values from before are converted to binary and directly writen into the executable binary file.
    dd CHECKSUM                     ; The bootloader reads this, so the order matters as a 12 byte sequence in this order is expected.
                                    ; This allows the bootloader to confirm the file and  start the OS.

; ---------------------------------------------------------------------------
; THE STACK
; ---------------------------------------------------------------------------
; A "stack" is a region of memory the CPU uses for function calls and local variables. For most programming languages this is needed.
; The following code reserves some space in memory for the stack.

section .bss                        ; A dedicated section of computer memory used for storing global or static variables that are without an initial value.
                                    ; AKA uninitialized variables.
                                    ; This means that variables used by our program does not directly store in the file and instead saves to the memory.
                        
align 16                            ; for x86 ABI(Application Binary Interface), or the convention for compiling code to binary. The stack should be 16-byte aligned.
                                    ; This is for correctness and speed, and tells the compilor where to look in terms of locations in memory or registers.

stack_bottom:                       ; : <- defines a label. This labels the LOW end of the stack. Indenting does not matter but makes readable code.

    resb 16384                      ; "resb" means reserve bytes. This reserves 16 KB as empty stack space.

stack_top:                           ; A label for the HIGH end of the stack. The stack will grow down from the top to the bottom, and top is the start.

; ---------------------------------------------------------------------------
; THE ENTRY POINT
; ---------------------------------------------------------------------------
; ".text" is the conventional section name for executable CODE (instructions/steps for the cpu). The code that the CPU will run.

section .text                       ; groups the following code into section .text

global _start                       ; Global allows the linker to see the label _start, which is the default name that the linker uses for entry to the program.


_start:                             ; Makes a label for the following as start, or code that is first in memory, allowing the bootloader to find it first.

    ; --- Step 1: give the CPU a working stack ---
    mov esp, stack_top              ; The esp (Extended Stack Pointer) is a built in pointer for x86 CPUs which tracks the current memory address of the stack.
                                    ; mov tells the code to set the esp register to the location of stack_top, or the start of the stack.
                                    ; This way any code written has somewhere to store call data.


    ; --- Step 2: jump into our C++ code ---
    extern kernel_main              ; extern defines that the function "kernel_main" is defined in an external file, in this case kernel.cpp.
                                    ; This is a placeholder for the linker to link the memory address of this function to.

    call kernel_main                ; "call" first sets a return location, using the esp(ln 79), then runs the function kernel_main. 
                                    ; once the function is run, the code triggers a ret(return) command, and the CPU returns using the return location.

    ; --- Step 3: if the kernel ever returns, stop safely ---
        ; kernel_main shouldn't normally return, but if it does we must NOT let
        ; the CPU wander into random memory and execute garbage. So we halt, or freeze the computer until a reboot.
.hang:                              ; A local label (the leading '.' means this label is location to _start) that allows us to loop back to .hang.

    cli                             ; "cli" = clear interrupts. This clears hardware interrupts, or signals from hardware like a keyboard or a network card.
                                    ; This way nothing interrupts the code from halting.

    hlt                             ; "hlt" = HALT. Stops the cpu until interruptted. Effectively forever as interupts were disabled. The CPU sleeps forever.

    jmp .hang                       ; "jmp" unconditional jump. A safety net, where is the cpu wakes anyways, loop back to the .hang label and halt again.

    ; --- In case of the kernel returning, freeze the computer until a reboot that resets the ram---
