; Assembly stubs for the 32 CPU exceptions. When an exception fires, the cpu jumps to one of these.
; Each stub records its exception number and then jumps to a shared routine using our idt.cpp exception_handler().
; We need assembly here because the CPU enters an interrupt in a way that c++ can't handle.

extern exception_handler          ; the C++ function (in idt.cpp) we ultimately call

; For exceptions that don't push an error code, we push a fake 0 to keep the stack layout the same as the ones that do, and then we push the exception number.

%macro ISR_NOERR 1  ; Define a macro named ISR_NOERR that takes 1 argument. From here to %endmacro, everything is a reusable template.
                    ; %1 represents this argument. Works like a parameter in a function.
global isr%1        ; Makes this label visible to the linker.ld. isr%1 means isr(variable).
isr%1:              ; The label where the CPU jumps to for our exception. This is the address stored in idt.cpp.
    push dword 0          ; fake error code (this exception didn't supply one). Push a 0 onto the stack
    push dword %1         ; the exception number. Push the exception number onto the stack.
    jmp isr_common        ; go to the shared handler-caller. This is at the bottom of the file.
%endmacro  ; End the macro.

; For exceptions that do push an error code: the CPU already put the error code on the stack, so we only push the exception number.
; This code is mostly the same as above, just without our fake error code.

%macro ISR_ERR 1
global isr%1
isr%1:
    push dword %1         ; the exception number (error code is already on the stack)
    jmp isr_common
%endmacro

; Generate the 32 stubs. ERR ones are 8, 10, 11, 12, 13, 14, 17; the rest NOERR. Found in the Intel Software Developer's Manual (SDM), volume 3.
ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_ERR   8
ISR_NOERR 9
ISR_ERR   10
ISR_ERR   11
ISR_ERR   12
ISR_ERR   13
ISR_ERR   14
ISR_NOERR 15
ISR_NOERR 16
ISR_ERR   17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_NOERR 21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_NOERR 29
ISR_NOERR 30
ISR_NOERR 31

; We now need to create the share routine that every stub will jump to.
; The stubs pushed [error code][exception number] onto the stack. The exception number in on the top, so we call exception_handler with it and then halt.

isr_common:
    ; The exception number is the top item on the stack, which is exactly where the C calling convention will expect the first argument, so we can just call without moving anything.
    call exception_handler
    ; exception_handler halts the CPU, so we never actually return here. But just in case, stop completely.
    cli
    hlt
    jmp $                 ; infinite loop, Redundancy case.