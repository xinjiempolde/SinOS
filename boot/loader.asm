;Why bootloader? bootsector has only 512 bytes, and it's insufficient for all boot things.
[bits 16]
%include "./include/const_value.inc"
start_of_loader:
    ;initlize segemnt
    mov ax, cs
    mov ds, ax

    ; compare whether entry name is same as 'LOADER.BIN'
    mov ax, BaseOfRootDir
    mov es, ax
    xor di, di                                 ; set di=0x0000, es:di => entry
find_kernel_loop:
    cld
    mov si, KERNEL_FILENAME                    ; ds:si => 'KERNEL   BIN'
    mov cx, KN_LEN
    repe cmpsb
    jcxz kernel_found

    and di, 0xffe0                             ; di % 32 + 32， get the next root dir entry
    add di, 32
    cmp di, 224*32
    jb find_kernel_loop                        ; all root dir entry searched, but no found
no_kernel:
    mov bx, NO_KN_MSG
    call print
    jmp $
kernel_found:
    mov bx, KN_FOUND_MSG
    call print
    call print_nl


    and di, 0xffe0
    add di, 0x1a
    mov ax, word [es:di]                         ; get DIR_FstClus
    mov bx, BaseOfKernel
    mov es, bx
    mov bx, OffsetOfKernel
load_kernel_loop:
    push ax                                      ; to find FAT entry
    add ax, DeltaDataSecNo                       ; now ax is the sector number of LOADER.BIN's value
    mov cl, 1
    call read_sector                             ; read sector to BaseOfKernel:bx

    pop ax
    call get_FATentry
    add bx, 0x200                                ; read the next sector(if exists)
    cmp ax, 0xff8                               
    jb load_kernel_loop                          ; FAT entry < 0xff8:has next sector, FAT entry > 0xff8(always 0xfff): no next sector
start_switch_gm:
    call switch_vga_13                           ; switch video card to mode 13h
start_switch_pm:
    call switch_to_pm                            ; switch to 32bit protected mode

[bits 32]
BEGIN_PM:
jmp_to_kernel:
    call CODE_SEGMENT:KernelAddress              ; give control to kernel(set IP=0x10000)
    jmp $                                        ; stay here if kernel returns control to us
%include "print.asm"
%include "disk.asm"
%include "32bit_gdt.asm"
%include "32bit_switch.asm"
%include "switch_graphic_mode.asm"

KN_FOUND_MSG db 'kernel found, loading kernel into memory', 0
NO_KN_MSG db 'there is no kernel found',0
KERNEL_FILENAME db 'KERNEL  BIN'              ; KERNEL.BIN
KN_LEN equ $-KERNEL_FILENAME                  ; KERNEL FILENAME LENGTH