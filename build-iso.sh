#!/usr/bin/env bash
# Build-only version of build.sh (no QEMU launch), for use inside the Docker
# container. It produces the bootable ISO, you then run that ISO with QEMU on your host machine.
# Run it from INSIDE a lesson folder. The ISO name is taken from the folder name,
# e.g. 06-Scrolling -> build/LeveretOS-Scrolling.iso
set -e

# Take the folder name, drop the leading "NN-", and build the ISO path from it.
lesson="$(basename "$PWD" | sed 's/^[0-9]*-//')"
iso="build/LeveretOS-${lesson}.iso"

CXXFLAGS="-m32 -ffreestanding -fno-exceptions -fno-rtti -fno-stack-protector -fno-pie -nostdlib -Wall -Wextra"

echo "[1/4] Assembling every src/*.s..."
# Assembles all .s files, so it works whether a lesson has one boot.s or many (e.g. gdt_flush.s).
mkdir -p build
OBJS=""
for asm in src/*.s; do
    obj="build/$(basename "${asm%.s}").o"
    echo "    $asm -> $obj"
    nasm -f elf32 "$asm" -o "$obj"
    OBJS="$OBJS $obj"
done

echo "[2/4] Compiling every src/*.cpp..."
# Compiles all .cpp files, so it works whether a lesson has one kernel.cpp or many.
for cpp in src/*.cpp; do
    obj="build/$(basename "${cpp%.cpp}").o"
    echo "    $cpp -> $obj"
    g++ $CXXFLAGS -c "$cpp" -o "$obj"
    OBJS="$OBJS $obj"
done

echo "[3/4] Linking kernel.bin..."
g++ -m32 -T linker.ld -ffreestanding -nostdlib -no-pie \
    -o build/kernel.bin $OBJS

echo "[4/4] Building the ISO..."
mkdir -p build/isodir/boot/grub
cp build/kernel.bin build/isodir/boot/kernel.bin
cp grub.cfg build/isodir/boot/grub/grub.cfg
grub-mkrescue -o "$iso" build/isodir

echo "Done. ISO is at ${iso}"
echo "Run it on your host with:  qemu-system-i386 -cdrom ${iso}"
