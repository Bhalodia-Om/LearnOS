# LeveretOS

**LeveretOS** is a small educational Operating System built from scratch. New additions are split into separate lessons, with each step documented. Every change includes educational comments, built for beginners. This repository aims to double as a step-by-step guide on how an OS actually works.

## Why "LeveretOS"?

A leveret is a baby hare, small, quick, and one of the fastest-growing young animals around. It felt like the right mascot for an OS that started as almost nothing and grows a little better with every lesson.

## The idea

Each lesson will have its own folder, and will add one new feature on top of the last. New or changed code will contain comments to guide you through each step. Note that reused code will be left without educative comments to improve simplicity.

## Goals

The first big goal is a LeveretOS that can go online and open a web page. These are the checkpoints we have set to reach our goal.

- [x] Play a game on it (Rock Paper Scissors)
- [ ] Type commands into it, like a real terminal
- [ ] Show a running clock
- [ ] Greet you with a login screen at startup
- [ ] Graphics processing, so our browser gets a display.
- [ ] Open a page from the web

Once we can browse the web, the goals will get bigger. This is where LeveretOS will grow from a single program into something closer to a real operating system.

- [ ] Log in securely, with a real password
- [ ] Create, open, and manage files
- [ ] Write and save notes in a text editor
- [ ] Run separate apps, instead of everything being one program
- [ ] Open the web browser as one of those apps

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
| 10 | Interrupts (IDT) | Catch CPU exceptions with our own interrupt table |

## Building and running a lesson

Don't want to download anything? Every lesson can be booted in browser at [leveretos.me](https://leveretos.me), just press the "Boot" button on any lesson. If you'd rather build it yourself, you can use the following methods.

Each lesson folder is self-contained and includes a `build.sh` that assembles the bootloader, compiles the kernel, makes a bootable ISO, and launches it in QEMU. You will need `nasm`, a 32-bit-capable `g++`, `grub-mkrescue`, `xorriso`, and `qemu-system-i386` (on Windows, a Linux environment such as WSL works well).

A good guide for setting up the necessary software to be able to boot: [OSDev Bare Bones](https://wiki.osdev.org/Bare_Bones)

## Building with Docker (macOS, Windows, or Linux)

If you don't want to install everything by hand, the repo includes a `Dockerfile` with the build environment. You only need [Docker](https://www.docker.com/) to build, and [QEMU](https://www.qemu.org/) to run.

```bash
# From the repo root, build the toolchain container once:
docker build -t leveretos .

# Build a lesson's ISO inside the container (example: lesson 06).
# Change the folder after -w to build a different lesson:
docker run --platform linux/amd64 --rm -v "$PWD":/src -w /src/06-Scrolling leveretos ../build-iso.sh

# Run the resulting ISO with QEMU on your own machine:
qemu-system-i386 -cdrom 06-Scrolling/build/LeveretOS-Scrolling.iso
```

The container is pinned to x86-64, so on Apple Silicon Macs Docker can emulate it (a little slower, but it works). The ISO is written to the lesson's `build/` folder, so you run it with QEMU on the host.
