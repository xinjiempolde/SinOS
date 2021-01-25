[bits 16]
;read cl sectors starting from ax sector(ax,cl in 0~2879) to es:bx
; how to get cylinder, sector, head(ax -> cylinder, sector, head)
; -----------------------------------------------------------------------
; assume sector is x
;                                ┌ cylinder = y >> 1
;       x           ┌ quotient y ┤
; -------------- => ┤            └ head = y & 1
; sectors per track │
;                   └ reminder z => sector = z + 1
; -----------------------------------------------------------------------
read_sector:
    pusha
    push bx
    push cx                            ;save cl

    mov bl, 18                         ;18 sectors per track
    div bl

    inc ah
    mov cl, ah                         ;CL, Sector

    mov ch, al
    shr ch, 1                          ;CH, Cylinder

    mov dh, al
    and dh, 1                          ;DH, Head

    pop ax                             ;AL, Sector Counts
    pop bx
    mov ah, 0x02                       ;AH, 0x02 = Read sector from disk
    push ax                            ;save sector counts, checking sector error after

    int 0x13
    jc disk_error
    pop dx
    cmp al, dl                         ;if success, ah=0x00, al=sector count
    jne sectors_error
    jmp end_read_sector
disk_error:
    mov bx, DISK_ERROR
    call print
    jmp disk_loop
sectors_error:
    mov bx, SECOTR_ERROR
    call print

disk_loop:
    jmp $

end_read_sector:
    popa
    ret

DISK_ERROR:
    db "Disk Read Error", 0

SECOTR_ERROR:
    db "Incorrect number of sectors read",0

; get FAT entry
; in: ax is DIR_FstClus
; out: ax is FAT entry
get_FATentry:
    push bx
    push dx

    mov bx, 3
    mul bx
    mov bx, 2
    div bx                             ; (ax * 3) / 2, get FAT entry offset; dx:reminder, ax:quotient
    mov si, ax

    mov bx, es
    push bx                            ; save es
    mov bx, BaseOfFat
    mov es, bx
    mov ax, word [es:si]
    pop bx
    mov es, bx                         ; change es to origin
    test dx, 0x1
    jz FATentry1
FATentry2:
    shr ax, 4
FATentry1:
    and ax, 0x0fff

    pop dx
    pop bx
    ret