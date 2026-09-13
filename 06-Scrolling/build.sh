#!/usr/bin/env bash 


# <- signifies a comment.
# Note that this is not an actual file part of the OS. This is just building the OS into an ISO file.

# Line 1 is not a comment. The #! tells the system which program should run this file. Right now it is telling the system to build our file using BASH(Bourne Again Shell)
# Bash is the shell, which runs terminal commands, like command prompt. This allows our current OS, to be able to for our multiple files into one ISO file for the OS.
# A .sh script is a saved list of commands, run in order.

set -e # "set -e" makes the script stop immediately if any commands fail. This way we don't build an ISO from a broken kernel.

echo "[1/5] Assembling boot.s..." # echo writes a message into terminal for the user. This tells the user boot.s is assembling.

mkdir -p build  # "mkdir" <- make directory/create a folder. "-p" <- A flag that tells BASH to not error if said folder exists, and create parent folders as needed. 
                # Parent folders would be folders before a /. "build" is the name of the folder we are marking.

nasm -f elf32 src/boot.s -o build/boot.o    # "nasm" is the assembler program that turns our source in boot.s into machine readable code.
                                            # "-f elf32" flag for the output. Produces a 32-bit ELF object. An ELF object is a binary file format for executable code.
                                            # "src/boot.s" The location of the file to assemble.
                                            # "-o build/boot.o" THe location of the output for our 32-bit ELF object.

echo "[2/5] Compiling kernel.cpp..." # echo writes a message into terminal for the user.

# The freestanding flags: no OS assumptions, no exceptions/RTTI/stack-protector,
# no position-independent code (kernels load at a fixed address), no stdlib.

# A note for the following line. The '\' is there to signify that the command continues onto the next line.
# The flags can all be on the same line if the '\' is removed.

# g++ is the standard compiler for c++. We are including freestanding flags so that no OS is needed.
g++ -m32 -ffreestanding -fno-exceptions -fno-rtti -fno-stack-protector \
    -fno-pie -nostdlib -Wall -Wextra -c src/kernel.cpp -o build/kernel.o

# The comments below are an exact copy of the flags above, with comments. as writing an inline comment would interupt the continuous line. There can also be no spaces.

    #  -m32 \                                   # Build for 32-bit systems.
    #   -ffreestanding \                        # Assume no OS
    #   -fno-exceptions \                       # Turn off exceptions, which need a runtime(behind the scenes code that c++ relies on). 
    #   -fno-rtti \                             # Turn off "rtti" (run-time type information). A built in program that allows the code do discorver data types of objects while running. Needs a runtime.
    #   -fno-stack-protector \                  # Turn off "stack-protector", which checks for library(saved functions) functions that we do not have.
    #   -fno-pie \                              # Turn off "pie" (Position-Independent Executable), a security feature which randomizes the location of the kernel in ram.
    #   -nostdlib \                             # Don't link the standard library, which we don't have.
    #   -Wall \                                 # Turn on all critical and high impact warnings.
    #   -Wextra \                               # Turn on stricter warnings that "-Wall" doesn't.
    #   -c \                                    # compile only the object file build/kernel.o, but not the linker.
    #   src/kernel.cpp -o build/kernel.o        # Take kernel.cpp, and compile an output at build/kernel.o.


echo "[3/5] Linking kernel.bin..."

# Some flags from before + "-T linker.ld", which tells the compiler to use the linker script for the memory layout.
# "-o build/kernel.bin" outputs kernel.bin from our two inputs in order: build/boot.o, build/kernel.o.
# The linker merges sections from both files, matches up cross-file calls, and lays it out in the order specified in linker.ld.
g++ -m32 -T linker.ld -ffreestanding -nostdlib -no-pie \
    -o build/kernel.bin build/boot.o build/kernel.o

echo "[4/5] Building the ISO..."

# Recreate the folder structure GRUB expects inside the ISO, shown as the following:
#   isodir/boot/kernel.bin
#   isodir/boot/grub/grub.cfg

#isodir is the directory for the ISO file itself.

mkdir -p build/isodir/boot/grub                     # Make directory grub, while creating any files that are missing.

cp build/kernel.bin build/isodir/boot/kernel.bin    # "cp" <- copy the "kernel.bin" from build to our "boot" folder in the ISO.
cp grub.cfg build/isodir/boot/grub/grub.cfg         # "cp" <- copy the "grub.cfg" from build to our "grub" folder in the ISO.

# grub-mkrescue packs the isodir folder into a bootable .iso
grub-mkrescue -o build/LeveretOS-Scrolling.iso build/isodir

echo "[5/5] Launching QEMU..."

# Boot the ISO in our emulated 32-bit PC(represented by i386, representing a 32-bit x86 computer).
qemu-system-i386 -cdrom build/LeveretOS-Scrolling.iso      # "-cdrom". Attach our ISO as a read only(ROM) disk.