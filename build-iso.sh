#!/usr/bin/env bash
# Build-only version of build.sh (no QEMU launch), for use inside the Docker
# container. It produces the bootable ISO, you then run that ISO with QEMU on your host machine.
# Run it from INSIDE a lesson folder. The ISO name is taken from the folder name,
# e.g. 06-Scrolling -> build/LeveretOS-Scrolling.iso
set -e

# Take the folder name, drop the leading "NN-", and build the ISO path from it.
lesson="$(basename "$PWD" | sed 's/^[0-9]*-//')"
iso="build/LeveretOS-${lesson}.iso"

echo "[1/4] Assembling boot.s..."
mkdir -p build
nasm -f elf32 src/boot.s -o build/boot.o

echo "[2/4] Compiling kernel.cpp..."
g++ -m32 -ffreestanding -fno-exceptions -fno-rtti -fno-stack-protector \
    -fno-pie -nostdlib -Wall -Wextra -c src/kernel.cpp -o build/kernel.o

echo "[3/4] Linking kernel.bin..."
g++ -m32 -T linker.ld -ffreestanding -nostdlib -no-pie \
    -o build/kernel.bin build/boot.o build/kernel.o

echo "[4/4] Building the ISO..."
mkdir -p build/isodir/boot/grub
cp build/kernel.bin build/isodir/boot/kernel.bin
cp grub.cfg build/isodir/boot/grub/grub.cfg
grub-mkrescue -o "$iso" build/isodir

echo "Done. ISO is at ${iso}"
echo "Run it on your host with:  qemu-system-i386 -cdrom ${iso}"
