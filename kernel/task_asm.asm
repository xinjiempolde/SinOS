[global jmp_to_new_task]

jmp_to_new_task:
    pusha
    mov ax, 24
    ltr ax
    jmp 32:0
    popa
    ret