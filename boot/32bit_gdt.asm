gdt_start:
    dd 0x0 ;the first null descriptor
    dd 0x0

;code segment descriptor
gdt_code:
    dw 0xffff ;Segment Limit 15:00
    dw 0x0    ;Base Address 15:00
    db 0x0    ;Base Address 23:16
    db 10011010b
    db 11001111b
    db 0x0    ;Base Address 31:24

gdt_data:
    dw 0xffff ;Segment Limit 15:00
    dw 0x0    ;Base Address 15:00
    db 0x0    ;Base Address 23:16
    db 10010010b
    db 11001111b
    db 0x0    ;Base Address 31:24

gtd_end:

gdt_descriptor:
    dw gtd_end - gdt_start - 1        ;gdt size(16 bits)
    dd gdt_start+LoaderAddress        ;gdt Address(32 bits),it's important to add LoaderAddress, wasting me a lot of time~~

CODE_SEGMENT equ gdt_code - gdt_start
DATA_SEGMENT equ gdt_data - gdt_start