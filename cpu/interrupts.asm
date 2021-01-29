;EXTERN: Importing Symbols from Other Modules
[extern isr_handler]
[extern irq_handler]

isr_common:
    ;save state
    pusha
    mov eax, ds
    push eax
    
    ;run isr_handler in c program
    call isr_handler

    ;Restore state
    pop eax
    popa
    add esp, 8 ; Cleans up the pushed interrupt number and error code
    sti
    iret ; Pop 5 things that were psuhed by cpu automatically: 
         ; CS, EIP, EFLAGS, SS and ESP

irq_common:
    ;save state
    pusha
    mov eax, ds
    push eax

    call irq_handler

    pop eax
    popa
    add esp, 8
    sti
    iret



;GLOBAL: Exporting Symbols to Other Modules
global isr0

global irq0
global irq1
global irq12

isr0:
    cli
    push 0x0 ;error code, 32bits
    push 0x0 ;interrupt number, 32bits
    jmp isr_common

irq0:
    cli
    push 0x0
    push 32 ;interrupt number
    jmp irq_common

irq1:
    cli
    push 0x0
    push 33
    jmp irq_common

; mouse interrupt
irq12:
    cli
    push 0x0
    push 44
    jmp irq_common