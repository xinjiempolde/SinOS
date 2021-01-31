; see details of dispaly:
;     http://www.ctyme.com/intr/int-10.htm     
;     https://en.wikipedia.org/wiki/INT_10H
;     https://en.wikipedia.org/wiki/VESA_BIOS_Extensions
;     https://pdos.csail.mit.edu/6.828/2008/readings/hardware/vbe3.pdf
[bits 16]

; offset in VbeInfoBlock, get from function 00h
VbeVersion     equ 0x04

; offset in ModeInfoBlock, get from function 01h
XResolution    equ 0x12
YResolution    equ 0x14
BitsPerPixel   equ 0x19
MemoryModel    equ 0x1b
PhysBasePtr    equ 0x28

; VBE mode, 0x105 represents 1024*768
VBEMODE        equ 0x105

; c will use later
SCREEN_W_ADDR  equ 0xff00
SCREEN_H_ADDR  equ 0xff02
VRAM_ADDR      equ 0xff04
VGA_OR_VBE_ADDR equ 0xff08
RESET_PALETTE_ADDR equ 0xff09
BOOT_BY_VGA    equ 0x00
BOOT_BY_VBE    equ 0x01
NO_RESET_PALETTE    equ 0x00
RESET_PALETTE    equ 0x01

; subroutine_name: switch_vbe_mode
switch_vbe_mode:
    pusha
    mov ax, 0x9000
    mov es, ax
    mov di, 0       ; store some info about vbe to es:di

check_vbe_version:
    ;see details: http://www.phatcode.net/res/221/files/vbe20.pdf
    ;Input: AX = 4F00h Return VBE Controller Information
    ;ES:DI = Pointer to buffer in which to place
            ;VbeInfoBlock structure
            ;(VbeSignature should be set to 'VBE2' when
            ;function is called to indicate VBE 2.0 information
            ;is desired and the information block is 512 bytes in size.)
    ;Output: AX = VBE Return Status
    mov ax, 0x4f00
    int 0x10
    cmp ax, 0x004f ; 4f: Function is supported
    jne switch_to_vga
    mov ax, [es:di+VbeVersion] ; VBE version, version2:0x200, version3:0x300
    cmp ax, 0x200
    jb switch_to_vga

get_vbe_mode_info:
    ;Input: AX = 4F01h Return VBE mode information
    ;CX = Mode number
    ;ES:DI = Pointer to ModeInfoBlock structure
    ;Output: AX = VBE Return Status
    mov ax, 0x4f01
    mov cx, VBEMODE
    int 0x10
    mov al, [es:di+BitsPerPixel]
    cmp al, 8                     ; text mode is 4
    jne switch_to_vga
    mov al, [es:di+MemoryModel]
    cmp al, 0x04                  ; packed pixel
    jne switch_to_vga

set_vbe_mode:
    ; Input: AX = 4F02h Set VBE Mode
    ; BX = Desired Mode to set
    ; D0-D8= Mode number
    ; D9-D13 = Reserved (must be 0)
    ; D14 = 0 Use windowed frame buffer model
    ;     = 1 Use linear/flat frame buffer model
    ; D15 = 0 Clear display memory
    ;     = 1 Don't clear display memory
    ; Output: AX = VBE Return Status
    mov ax, 0x4f02
    mov bx, VBEMODE
    or bx, (1<<14)
    int 0x10
    cmp ax, 0x004f
    jnz switch_to_vga

store_vbe_boot_state:
    ; save boot status that c will use
    mov bx, ds    ; store ds
    mov ax, 0
    mov ds, ax    ; set ds=0

    mov ax, [es:di+XResolution]
    mov [ds:SCREEN_W_ADDR], ax
    mov ax, [es:di+YResolution]
    mov [ds:SCREEN_H_ADDR], ax

    mov ax, [es:di+PhysBasePtr]
    mov [ds:VRAM_ADDR], ax
    mov ax, [es:di+PhysBasePtr+2]
    mov [ds:VRAM_ADDR+2], ax   ; c pointer is 32 bit, real mode ax only 16 bit
    
    mov ax, BOOT_BY_VBE
    mov [ds:VGA_OR_VBE_ADDR], ax

    mov ax, NO_RESET_PALETTE
    mov [ds:RESET_PALETTE_ADDR], ax
    mov ds, bx   ; restore ds

set_color_palette:
    ; set DAC bit width, 6 => 8
    mov ax, 0x4f08
    mov bl, 0x00
    mov bh, 0x08
    int 0x10
    cmp ax, 0x004f
    jne vbe_but_reset_palette

    ; set DAC color palette
    mov ax, ds
    mov es, ax
    mov di, RGB_TABLE
    mov ax, 0x4f09
    mov bl, 0x00
    mov cx, 16
    mov dx, 0
    int 0x10
    cmp ax, 0x004f
    jnz vbe_but_reset_palette  ; set vbe mode success, but fail setting palette, we will reset in c
    jmp switch_video_mode_end

; set palette fails
vbe_but_reset_palette:
    mov bx, ds    ; store ds
    mov ax, 0
    mov ds, ax    ; set ds=0
  
    mov ax, RESET_PALETTE
    mov [ds:RESET_PALETTE_ADDR], ax
    mov ds, bx   ; restore ds
    jmp switch_video_mode_end

; if switching vbe mode fails, then use vga mode 13(320*200) to display.
switch_to_vga:
    mov al, 0x13
    mov ah, 0x00
    int 0x10
store_vga_boot_state:
    ; save boot status that c will use
    mov bx, ds    ; store ds
    mov ax, 0
    mov ds, ax    ; set ds=0

    mov ax, 320
    mov [ds:SCREEN_W_ADDR], ax
    mov ax, 200
    mov [ds:SCREEN_H_ADDR], ax

    mov ax, 0x0000
    mov [ds:VRAM_ADDR], ax
    mov ax, 0x000a
    mov [ds:VRAM_ADDR+2], ax   ; c pointer is 32 bit, real mode ax only 16 bit
    
    mov ax, BOOT_BY_VGA
    mov [ds:VGA_OR_VBE_ADDR], ax

    mov ax, RESET_PALETTE
    mov [ds:RESET_PALETTE_ADDR], ax
    mov ds, bx   ; restore ds

switch_video_mode_end:
    popa
    ret

RGB_TABLE: 
db 0x00, 0x00, 0x00, 0x00   ; 0:黑
db 0x00, 0x00, 0xff, 0x00   ; 1:亮红
db 0x00, 0xff, 0x00, 0x00   ; 2:亮绿
db 0x00, 0xff, 0xff, 0x00   ; 3:亮黄
db 0xff, 0x00, 0x00, 0x00   ; 4:亮蓝
db 0xff, 0x00, 0xff, 0x00   ; 5:亮紫
db 0xff, 0xff, 0x00, 0x00   ; 6:浅亮蓝
db 0xff, 0xff, 0xff, 0x00   ; 7:白
db 0xd3, 0xd3, 0xd3, 0x00   ; 8:亮灰
db 0x66, 0x00, 0x84, 0x00   ; 9:暗红
db 0x00, 0x84, 0x00, 0x00   ; 10:暗绿
db 0x00, 0x84, 0x84, 0x00   ; 11:暗黄
db 0x8b, 0x00, 0x00, 0x00   ; 12:暗蓝
db 0x84, 0x00, 0x84, 0x00   ; 13:暗紫
db 0xeb, 0xce, 0x87, 0x00   ; 14:浅暗蓝
db 0xa9, 0xa9, 0xa9, 0x00   ; 15:暗灰
; SWITCH_ERROR db 'switch error',0
; SWITCH_SUCCESS db 'switch success',0