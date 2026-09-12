# LearnOS

**LearnOS** is a small operating system built completely from scratch, one lesson at a time. Every file is commented for beginners, so the repository doubles as a step-by-step guide to how an OS actually works, starting from the very first instruction that runs at boot.

## The idea

Each lesson has its own numbered folder and adds one new feature on top of the last. Code that is reused from earlier lessons will be left without educative comments, while keeping other comments. New or changed code will contain comments to guide you through each part.

## Lessons so far

| # | Lesson | What it adds |
|---|--------|--------------|
| 01 | Hello World | Booting via GRUB and writing text to VGA memory |
| 02 | Text Output | A moving cursor for typing, `putchar`/`print`, newlines and wrapping |
| 03 | Text Input | Reading the keyboard: scancodes, `getchar`, `read_line` |
| 04 | Print Numbers | Turning integers into on-screen digits |

## Building and running a lesson

Each lesson folder is self-contained and includes a `build.sh` that assembles the bootloader, compiles the kernel, makes a bootable ISO, and launches it in QEMU. You will need `nasm`, a 32-bit-capable `g++`, `grub-mkrescue`, `xorriso`, and `qemu-system-i386` (on Windows, a Linux environment such as WSL works well).

A good guide for setting up the necessary software to be able to boot: [OSDev Bare Bones](https://wiki.osdev.org/Bare_Bones)

## The goal

The current goal is to create an educational OS that can connect to the web.
