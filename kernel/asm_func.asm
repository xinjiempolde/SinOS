[global far_jmp]
[global load_task_register]
[bits 32]

;extern void far_jmp(uint32_t EIP, uint16_t CS)
far_jmp:
    ; call far_jmp => esp+2
    ; push %ebp => esp+2
    ; so dword ptr [esp+4] = EIP
    jmp far dword [esp+4]
    ret

; extern void load_task_register(uint16_t sel);
load_task_register:
    mov ax, [esp+4]
    ltr ax
    ret