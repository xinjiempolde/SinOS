IMG_FILE    = sinos.img
MOUNT_POINT = ~/mntpoint/
BOOTSEC     = bootsect.bin
LOADER      = LOADER.BIN
KERNEL      = KERNEL.BIN
KERNEL_ADDR = 0x10000
CC          = i386-elf-gcc
LD          = i386-elf-ld
CFLAGS      = -g -ffreestanding -Wall -Wextra -fno-exceptions -m32 -nostdinc -I include
C_SOURCES   = ${wildcard cpu/*c drivers/*.c kernel/*.c libc/*.c mm/*.c gui/*.c}
OBJ         = ${C_SOURCES:.c=.o cpu/interrupts.o mm/cache.o}

$(IMG_FILE): bootsect.bin $(LOADER) $(KERNEL)
	dd if=/dev/zero of=$@ bs=512 count=2880
	dd if=$< of=$@ conv=notrunc
	sudo mount -o loop $@ $(MOUNT_POINT)
	sudo cp $(LOADER) $(KERNEL) $(MOUNT_POINT)
	sudo umount $(MOUNT_POINT)

$(BOOTSEC): boot/bootsect.asm
	nasm -I boot/ -f bin $< -o $@

$(LOADER): boot/loader.asm
	nasm -I boot/ -f bin $< -o $@

$(KERNEL): boot/kernel_entry.o ${OBJ}
	$(LD) -Ttext $(KERNEL_ADDR) --oformat binary $^ -o $@

symbol.elf: boot/kernel_entry.o ${OBJ}
	$(LD) -Ttext $(KERNEL_ADDR) $^ -o $@
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.asm
	nasm -f elf $< -o $@


.PHONY: debug-qemu
debug-qemu: symbol.elf
	qemu-system-i386 -s -S -fda $(IMG_FILE) &
	i386-elf-gdb -ex "target remote localhost:1234" -ex "symbol-file symbol.elf"

.PHONY: debug-bochs
debug-bochs: bochsrc.txt
	bochs -q

.PHONY: run
run: $(IMG_FILE)
	qemu-system-i386 -fda $<

.PHONY: clean
clean:
	rm -rf *.bin *.BIN *.img ./*/*.o
	rm -rf symbol.elf