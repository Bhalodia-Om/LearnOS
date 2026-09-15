#!/usr/bin/env bash
# Builds the OS into a bootable ISO and launches QEMU.
# Since lesson 8 the kernel lives in several .cpp files, so we compile each one to its own .o and link them together.
set -e

CXXFLAGS="-m32 -ffreestanding -fno-exceptions -fno-rtti -fno-stack-protector -fno-pie -nostdlib -Wall -Wextra"

echo "[1/4] Assembling every src/*.s..."
# We've got two .s files now (boot.s and gdt_flush.s), so loop over all of them instead of just boot.s.
mkdir -p build
OBJS=""
for asm in src/*.s; do
    obj="build/$(basename "${asm%.s}").o"   # e.g. src/gdt_flush.s -> build/gdt_flush.o
    echo "    $asm -> $obj"
    nasm -f elf32 "$asm" -o "$obj"
    OBJS="$OBJS $obj"
done

echo "[2/4] Compiling every src/*.cpp..."
for cpp in src/*.cpp; do
    obj="build/$(basename "${cpp%.cpp}").o"   # e.g. src/vga.cpp -> build/vga.o
    echo "    $cpp -> $obj"
    g++ $CXXFLAGS -c "$cpp" -o "$obj"
    OBJS="$OBJS $obj"
done

echo "[3/4] Linking kernel.bin..."
g++ -m32 -T linker.ld -ffreestanding -nostdlib -no-pie -o build/kernel.bin $OBJS

echo "[4/4] Building the ISO..."
mkdir -p build/isodir/boot/grub
cp build/kernel.bin build/isodir/boot/kernel.bin
cp grub.cfg build/isodir/boot/grub/grub.cfg
grub-mkrescue -o build/LeveretOS-GDT.iso build/isodir

echo "[5/5] Launching QEMU..."
qemu-system-i386 -cdrom build/LeveretOS-GDT.iso
