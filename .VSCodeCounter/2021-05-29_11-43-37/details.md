# Details

Date : 2021-05-29 11:43:37

Directory /home/singheart/projects/hariOS

Total : 70 files,  3700 codes, 473 comments, 731 blanks, all 4904 lines

[summary](results.md)

## Files
| filename | language | code | comment | blank | total |
| :--- | :--- | ---: | ---: | ---: | ---: |
| [Makefile](/Makefile) | Makefile | 47 | 0 | 12 | 59 |
| [boot/32bit_gdt.asm](/boot/32bit_gdt.asm) | nasm | 23 | 1 | 4 | 28 |
| [boot/32bit_switch.asm](/boot/32bit_switch.asm) | nasm | 19 | 0 | 2 | 21 |
| [boot/bootsect.asm](/boot/bootsect.asm) | nasm | 74 | 6 | 12 | 92 |
| [boot/disk.asm](/boot/disk.asm) | nasm | 63 | 13 | 14 | 90 |
| [boot/include/const_value.inc](/boot/include/const_value.inc) | nasm | 15 | 4 | 5 | 24 |
| [boot/include/fat12hdr.inc](/boot/include/fat12hdr.inc) | nasm | 21 | 4 | 2 | 27 |
| [boot/kernel_entry.asm](/boot/kernel_entry.asm) | nasm | 6 | 0 | 0 | 6 |
| [boot/loader.asm](/boot/loader.asm) | nasm | 60 | 3 | 9 | 72 |
| [boot/print.asm](/boot/print.asm) | nasm | 49 | 13 | 23 | 85 |
| [boot/switch_graphic_mode.asm](/boot/switch_graphic_mode.asm) | nasm | 131 | 39 | 24 | 194 |
| [cpu/gdt.c](/cpu/gdt.c) | C | 19 | 3 | 7 | 29 |
| [cpu/idt.c](/cpu/idt.c) | C | 15 | 11 | 1 | 27 |
| [cpu/interrupts.asm](/cpu/interrupts.asm) | nasm | 207 | 42 | 48 | 297 |
| [cpu/isr.c](/cpu/isr.c) | C | 124 | 9 | 17 | 150 |
| [cpu/register.c](/cpu/register.c) | C | 20 | 6 | 4 | 30 |
| [cpu/timer.c](/cpu/timer.c) | C | 97 | 30 | 21 | 148 |
| [drivers/ata.c](/drivers/ata.c) | C | 138 | 16 | 27 | 181 |
| [drivers/keyboard.c](/drivers/keyboard.c) | C | 66 | 5 | 5 | 76 |
| [drivers/mouse.c](/drivers/mouse.c) | C | 111 | 23 | 18 | 152 |
| [drivers/ports.c](/drivers/ports.c) | C | 25 | 16 | 5 | 46 |
| [fs/dir.c](/fs/dir.c) | C | 149 | 20 | 33 | 202 |
| [fs/fs.c](/fs/fs.c) | C | 206 | 33 | 62 | 301 |
| [fs/inode.c](/fs/inode.c) | C | 52 | 3 | 8 | 63 |
| [gui/background.c](/gui/background.c) | C | 13 | 0 | 2 | 15 |
| [gui/console.c](/gui/console.c) | C | 244 | 12 | 38 | 294 |
| [gui/gedit.c](/gui/gedit.c) | C | 127 | 7 | 27 | 161 |
| [gui/layer.c](/gui/layer.c) | C | 203 | 25 | 37 | 265 |
| [gui/vga.c](/gui/vga.c) | C | 128 | 9 | 14 | 151 |
| [gui/window.c](/gui/window.c) | C | 47 | 2 | 1 | 50 |
| [include/cpu/gdt.h](/include/cpu/gdt.h) | C++ | 19 | 0 | 6 | 25 |
| [include/cpu/idt.h](/include/cpu/idt.h) | C++ | 20 | 2 | 7 | 29 |
| [include/cpu/isr.h](/include/cpu/isr.h) | C++ | 60 | 12 | 7 | 79 |
| [include/cpu/register.h](/include/cpu/register.h) | C++ | 10 | 0 | 4 | 14 |
| [include/cpu/timer.h](/include/cpu/timer.h) | C++ | 28 | 2 | 8 | 38 |
| [include/cpu/type.h](/include/cpu/type.h) | C++ | 19 | 0 | 6 | 25 |
| [include/drivers/ata.h](/include/drivers/ata.h) | C++ | 38 | 0 | 7 | 45 |
| [include/drivers/font16.h](/include/drivers/font16.h) | C++ | 132 | 4 | 2 | 138 |
| [include/drivers/keyboard.h](/include/drivers/keyboard.h) | C++ | 10 | 0 | 1 | 11 |
| [include/drivers/mouse.h](/include/drivers/mouse.h) | C++ | 15 | 0 | 4 | 19 |
| [include/drivers/ports.h](/include/drivers/ports.h) | C++ | 7 | 0 | 4 | 11 |
| [include/fs/dir.h](/include/fs/dir.h) | C++ | 29 | 2 | 7 | 38 |
| [include/fs/fs.h](/include/fs/fs.h) | C++ | 39 | 3 | 13 | 55 |
| [include/fs/inode.h](/include/fs/inode.h) | C++ | 22 | 11 | 8 | 41 |
| [include/fs/super_block.h](/include/fs/super_block.h) | C++ | 24 | 1 | 9 | 34 |
| [include/gui/background.h](/include/gui/background.h) | C++ | 5 | 0 | 2 | 7 |
| [include/gui/console.h](/include/gui/console.h) | C++ | 27 | 0 | 9 | 36 |
| [include/gui/gedit.h](/include/gui/gedit.h) | C++ | 25 | 0 | 6 | 31 |
| [include/gui/layer.h](/include/gui/layer.h) | C++ | 38 | 1 | 6 | 45 |
| [include/gui/vga.h](/include/gui/vga.h) | C++ | 46 | 2 | 12 | 60 |
| [include/gui/window.h](/include/gui/window.h) | C++ | 6 | 0 | 2 | 8 |
| [include/kernel/kernel.h](/include/kernel/kernel.h) | C++ | 3 | 0 | 1 | 4 |
| [include/kernel/pci.h](/include/kernel/pci.h) | C++ | 9 | 0 | 3 | 12 |
| [include/kernel/task.h](/include/kernel/task.h) | C++ | 30 | 1 | 7 | 38 |
| [include/kernel/thread.h](/include/kernel/thread.h) | C++ | 10 | 0 | 2 | 12 |
| [include/libc/function.h](/include/libc/function.h) | C++ | 4 | 0 | 1 | 5 |
| [include/libc/math.h](/include/libc/math.h) | C++ | 9 | 1 | 1 | 11 |
| [include/libc/mem.h](/include/libc/mem.h) | C++ | 8 | 0 | 1 | 9 |
| [include/libc/string.h](/include/libc/string.h) | C++ | 14 | 0 | 2 | 16 |
| [include/mm/memory.h](/include/mm/memory.h) | C++ | 24 | 3 | 8 | 35 |
| [kernel/asm_func.asm](/kernel/asm_func.asm) | nasm | 10 | 5 | 2 | 17 |
| [kernel/kernel.c](/kernel/kernel.c) | C | 64 | 2 | 12 | 78 |
| [kernel/pci.c](/kernel/pci.c) | C | 23 | 1 | 4 | 28 |
| [kernel/task.c](/kernel/task.c) | C | 74 | 7 | 15 | 96 |
| [kernel/thread.c](/kernel/thread.c) | C | 0 | 0 | 1 | 1 |
| [libc/math.c](/libc/math.c) | C | 0 | 0 | 1 | 1 |
| [libc/mem.c](/libc/mem.c) | C | 22 | 1 | 2 | 25 |
| [libc/string.c](/libc/string.c) | C | 126 | 12 | 24 | 162 |
| [mm/cache.asm](/mm/cache.asm) | nasm | 27 | 13 | 10 | 50 |
| [mm/memory.c](/mm/memory.c) | C | 125 | 32 | 22 | 179 |

[summary](results.md)