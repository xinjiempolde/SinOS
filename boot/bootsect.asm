[bits 16]
[org 0x7c00]
;**************************************************************boot sector starts******************************************************************************
%include "./include/fat12hdr.inc"
%include "./include/const_value.inc"

bootsect_start:
    ; initlize segment
    mov ax, 0
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov sp, 0x7c00

    mov bx, BaseOfFat
    mov es, bx
    mov bx, OffsetOfFat
    mov ax, StartSecOfFAT
    mov cl, SecNumsOfFATs
    call read_sector                           ; load FAT1 and FAT2 to BaseOfFat:OffsetOfFat

    mov ax, BaseOfRootDir
    mov es, ax
    mov bx, OffsetOfRootDir
    mov ax, StartSecOfRootDir
    mov cl, SecNumsOfEntry
    call read_sector                           ; load root directory entries to BaseOfRootDir:OffsetOfRootDir


    ; compare whether entry name is same as 'LOADER.BIN'
    xor di, di                                 ; set di=0x0000, es:di => entry
find_loader_loop:
    cld
    mov si, LOADER_FILENAME                    ; ds:si => 'LOADER   BIN'
    mov cx, LF_LEN
    repe cmpsb
    jcxz loader_found

    and di, 0xffe0                             ; di % 32 + 32， get the next root dir entry
    add di, 32
    cmp di, 224*32
    jb find_loader_loop                        ; all root dir entry searched, but no found
no_loader:
    mov bx, NO_LD_MSG
    call print
    jmp $
loader_found:
    mov bx, LD_FOUND_MSG
    call print
    call print_nl


    and di, 0xffe0
    add di, 0x1a
    mov ax, word [es:di]                         ; get DIR_FstClus
    mov bx, BaseOfLoader
    mov es, bx
    mov bx, OffsetOfLoader
load_loader_loop:
    push ax                                      ; to find FAT entry
    add ax, DeltaDataSecNo                       ; now ax is the sector number of LOADER.BIN's value
    mov cl, 1
    call read_sector                             ; read sector to BaseOfLoader:bx

    pop ax
    call get_FATentry
    add bx, 0x200                                ; read the next sector(if exists)
    cmp ax, 0xff8                               
    jb load_loader_loop                          ; FAT entry < 0xff8:has next sector, FAT entry > 0xff8(always 0xfff): no next sector
    
jmp_to_loader:
    jmp BaseOfLoader:OffsetOfLoader
    jmp $
%include "print.asm"
%include "disk.asm"

LD_FOUND_MSG db 'loader found, loading loader into memory', 0
NO_LD_MSG db 'there is no loader found',0
LOADER_FILENAME db 'LOADER  BIN'              ; LOADER.BIN
LF_LEN equ $-LOADER_FILENAME                  ; LOADER FILENAME LENGTH
times 510-($-$$) db 0
dw 0xaa55
;**************************************************************boot sector ends******************************************************************************

;**************************************************************FAT starts************************************************************************************
FAT1:
    db 0xf0, 0xff, 0xff
    times 9*512 - ($-FAT1) db 0
FAT2:
    db 0xf0, 0xff, 0xff
    times 9*512 - ($-FAT2) db 0
;**************************************************************FAT ends**************************************************************************************