---
title: LeveretOS
description: Building an operating system from scratch, one commented lesson at a time.
---

# LeveretOS

**LeveretOS** is a small operating system built completely from scratch, one lesson
at a time. Every file is commented for beginners, so the project doubles as a
step-by-step guide to how an OS actually works, starting from the very first
instruction that runs at boot.

No Windows. No Linux underneath. Just code running on bare metal.

## Lessons so far

| # | Lesson | What it adds |
|---|--------|--------------|
| 01 | Hello World | Booting via GRUB and writing text to VGA memory |
| 02 | Text Output | A moving cursor, `putchar`/`print`, newlines and wrapping |
| 03 | Text Input | Reading the keyboard: scancodes, `getchar`, `read_line` |
| 04 | Print Numbers | Turning integers into on-screen digits |
| 05 | String Helpers | `strlen`, `strcmp`, `memcpy`, `memset`, and `atoi` |

## The idea

Each lesson has its own numbered folder and adds one new feature on top of the last.
Code reused from earlier lessons keeps its comments but gets no new teaching comments;
only new or changed code is freshly commented, so what's new each lesson stands out.

## The goal

The long-term goal is to grow LeveretOS far enough to connect to the internet and
pull down a simple web page over HTTP, building interrupts, memory management, and a
network stack along the way.

## Links

- Source code: [github.com/Bhalodia-Om/LeveretOS](https://github.com/Bhalodia-Om/LeveretOS)
