
[bits 16] ;this is very important, the assemble result is completely different with using 
          ;[bits 16] and [bits 32],which can drops an unexpected error if there is a [bits 32]
          ;before this code


;*******************************************print_hex starts*****************************************************
;print the values of dx(16 bit) in hex
print_hex:
    pusha
    mov cx, 0

hex_loop:
    cmp cx, 4
    je end

    mov ax, dx
    and ax, 0x000f

    add al, 0x30
    cmp al, 0x39
    jle step2
    add al, 7

step2:
    mov bx, HEX_OUT + 5
    sub bx, cx
    mov [bx], al

    ror dx, 4
    add cx, 1
    jmp hex_loop

end:
    mov bx, HEX_OUT
    call print

    popa
    ret
HEX_OUT:
    db '0x0000', 0
;*******************************************print_hex ends***************************************************


;*******************************************print starts*****************************************************
;function:print strings to screen
;parameters: bx is the address of strings which ends with 0x00 byte;
print:
    pusha

start:
    mov al, [bx]
    cmp al, 0
    je done

    ;raise the interrupt to show character
    mov ah, 0x0e
    int 0x10

    add bx, 1
    jmp start



done:
    popa
    ret
;*******************************************print ends**********************************************************


;*******************************************print_nl starts*****************************************************
;print newline
print_nl:
    pusha

    mov ah, 0x0e
    mov al, 0x0a ;newline char
    int 0x10
    mov al, 0x0d ;carriage return
    int 0x10
    
    popa
    ret
;*******************************************print_nl ends*******************************************************
