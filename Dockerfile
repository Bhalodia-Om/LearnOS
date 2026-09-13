# Build environment for LeveretOS.
# This installs the exact Linux toolchain the lessons' build.sh scripts expect,
# so anyone (macOS, Windows, Linux) can build the OS identically without setting
# the tools up by hand.
#
# Usage (from the repo root):
#   docker build -t leveretos .
#   docker run --rm -v "$PWD":/src -w /src/06-Scrolling leveretos ./build-iso.sh
#
# The ISO is written into that lesson's build/ folder on your machine (thanks to
# the -v mount). You then run it with QEMU on your host:
#   qemu-system-i386 -cdrom 06-Scrolling/build/LeveretOS-Scrolling.iso

FROM ubuntu:24.04

# gcc-multilib provides 32-bit (-m32) support; grub-pc-bin gives grub-mkrescue the
# BIOS boot modules it needs; xorriso is required by grub-mkrescue to build the ISO.
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        nasm \
        g++ \
        gcc-multilib \
        g++-multilib \
        grub-common \
        grub-pc-bin \
        xorriso \
        make \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
