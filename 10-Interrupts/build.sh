#!/usr/bin/env bash
# Builds the OS into a bootable ISO and launches QEMU.
# CHANGED for lesson 8: the kernel is now split across multiple .cpp files, so this
# compiles EVERY src/*.cpp into its own .o, then links them all together.
set -e

CXXFLAGS="-m32 -ffreestanding -fno-exceptions -fno-rtti -fno-stack-protector -fno-pie -nostdlib -Wall -Wextra"

echo "[1/4] Assembling every src/*.s..."
# CHANGED for lesson 9: we now have a second .s file (gdt_flush.s), so assemble them all.
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
