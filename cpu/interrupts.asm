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
global isr1
global isr2
global isr3
global isr4
global isr5
global isr6
global isr7
global isr8
global isr9
global isr10
global isr11
global isr12
global isr13
global isr14
global isr15
global isr16
global isr17
global isr18
global isr19
global isr20
global isr21
global isr22
global isr23
global isr24
global isr25
global isr26
global isr27
global isr28
global isr29
global isr30
global isr31

global irq0
global irq1
global irq12
global irq14
global irq15

; 0: Divide By Zero Exception
isr0:
    cli
    push 0 ;error code, 32bits
    push 0 ;interrupt number, 32bits
    jmp isr_common

; 1: Debug Exception
isr1:
    push 0
    push 1
    jmp isr_common

; 2: Non Maskable Interrupt Exception
isr2:
    push 0
    push 2
    jmp isr_common

; 3: Int 3 Exception
isr3:
    push 0
    push 3
    jmp isr_common

; 4: INTO Exception
isr4:
    push 0
    push 4
    jmp isr_common

; 5: Out of Bounds Exception
isr5:
    push 0
    push 5
    jmp isr_common

; 6: Invalid Opcode Exception
isr6:
    push 0
    push 6
    jmp isr_common

; 7: Coprocessor Not Available Exception
isr7:
    push 0
    push 7
    jmp isr_common

; 8: Double Fault Exception (With Error Code!)
isr8:
    push 8
    jmp isr_common

; 9: Coprocessor Segment Overrun Exception
isr9:
    push 0
    push 9
    jmp isr_common

; 10: Bad TSS Exception (With Error Code!)
isr10:
    push 10
    jmp isr_common

; 11: Segment Not Present Exception (With Error Code!)
isr11:
    push 11
    jmp isr_common

; 12: Stack Fault Exception (With Error Code!)
isr12:
    push 12
    jmp isr_common

; 13: General Protection Fault Exception (With Error Code!)
isr13:
    push 13
    jmp isr_common

; 14: Page Fault Exception (With Error Code!)
isr14:
    push 14
    jmp isr_common

; 15: Reserved Exception
isr15:
    push 0
    push 15
    jmp isr_common

; 16: Floating Point Exception
isr16:
    push 0
    push 16
    jmp isr_common

; 17: Alignment Check Exception
isr17:
    push 0
    push 17
    jmp isr_common

; 18: Machine Check Exception
isr18:
    push 0
    push 18
    jmp isr_common

; 19: Reserved
isr19:
    push 0
    push 19
    jmp isr_common

; 20: Reserved
isr20:
    push 0
    push 20
    jmp isr_common

; 21: Reserved
isr21:
    push 0
    push 21
    jmp isr_common

; 22: Reserved
isr22:
    push 0
    push 22
    jmp isr_common

; 23: Reserved
isr23:
    push 0
    push 23
    jmp isr_common

; 24: Reserved
isr24:
    push 0
    push 24
    jmp isr_common

; 25: Reserved
isr25:
    push 0
    push 25
    jmp isr_common

; 26: Reserved
isr26:
    push 0
    push 26
    jmp isr_common

; 27: Reserved
isr27:
    push 0
    push 27
    jmp isr_common

; 28: Reserved
isr28:
    push 0
    push 28
    jmp isr_common

; 29: Reserved
isr29:
    push 0
    push 29
    jmp isr_common

; 30: Reserved
isr30:
    push 0
    push 30
    jmp isr_common

; 31: Reserved
isr31:
    push 0
    push 31
    jmp isr_common

;=============== end of isr =====================

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

; ide hard disk interrupt
irq14:
    cli
    push 0x0
    push 46
    jmp irq_common

irq15:
    cli
    push 0x0
    push 47
    jmp irq_common