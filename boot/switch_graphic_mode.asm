[bits 16]
switch_vga_13:
    pusha

    ;see int 0x10 details: http://www.ctyme.com/intr/int-10.htm
    mov al, 0x13
    mov ah, 0x00
    int 0x10

    popa
    ret