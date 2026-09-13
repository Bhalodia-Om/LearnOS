# LeveretOS

**LeveretOS** is a small educational operating system built completely from scratch, with each new addition split into a separate lesson. Every change is commented for beginners, so this repository doubles as a step-by-step guide to how an OS actually works, starting from the very first instruction that runs at boot.

## Why "LeveretOS"?

A leveret is a baby hare, small, quick, and one of the fastest-growing young animals around. It felt like the right mascot for an OS that started as almost nothing and grows a little better with every lesson.

## The idea

Each lesson has its own numbered folder and adds one new feature on top of the last. Code that is reused from earlier lessons will be left without educative comments, while keeping other comments. New or changed code will contain comments to guide you through each part.

## Lessons so far

| # | Lesson | What it adds |
|---|--------|--------------|
| **Fundamentals** | | |
| 01 | Hello World | Booting via GRUB and writing text to VGA memory |
| 02 | Text Output | A moving cursor for typing, `putchar`/`print`, newlines and wrapping |
| 03 | Text Input | Reading the keyboard: scancodes, `getchar`, `read_line` |
| 04 | Print Numbers | Turning integers into on-screen digits |
| 05 | String helpers | Create helpers for strings and memory |
| 06 | Scrolling | Shift lines up when text reaches the bottom, like a real terminal |
| 07 | Rock Paper Scissors | A playable terminal game built from everything so far |
| **Talking to the Hardware** | | |
| 08 | Restructure | Split the kernel into multiple files and headers (no behaviour change) |
| 09 | GDT | Build our own segment table so the kernel controls its own code and data (no behaviour change) |

## Building and running a lesson

Each lesson folder is self-contained and includes a `build.sh` that assembles the bootloader, compiles the kernel, makes a bootable ISO, and launches it in QEMU. You will need `nasm`, a 32-bit-capable `g++`, `grub-mkrescue`, `xorriso`, and `qemu-system-i386` (on Windows, a Linux environment such as WSL works well).

A good guide for setting up the necessary software to be able to boot: [OSDev Bare Bones](https://wiki.osdev.org/Bare_Bones)

## Building with Docker (macOS, Windows, or Linux)

If you don't want to install the toolchain by hand, the included `Dockerfile` provides the exact Linux build environment. You only need [Docker](https://www.docker.com/) to build, and [QEMU](https://www.qemu.org/) to run.

```bash
# From the repo root, build the toolchain container once:
docker build -t leveretos .

# Build a lesson's ISO inside the container (example: lesson 06).
# Change the folder after -w to build a different lesson:
docker run --platform linux/amd64 --rm -v "$PWD":/src -w /src/06-Scrolling leveretos ../build-iso.sh

# Run the resulting ISO with QEMU on your own machine:
qemu-system-i386 -cdrom 06-Scrolling/build/LeveretOS-Scrolling.iso
```

The container is pinned to x86-64 (the toolchain needs the amd64 Ubuntu packages), so on Apple Silicon Macs Docker emulates it via QEMU, the build is a little slower but works. The ISO is built inside the container but written to the lesson's `build/` folder on your machine, so you run it with QEMU on the host. On macOS, install QEMU with `brew install qemu` (the 32-bit PC is emulated in software, which is fine for a small OS).

## The goal

The current goal is to create an educational OS that can connect to the web.
