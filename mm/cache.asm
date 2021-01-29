;************************************************************************
;* enable/disabel cache
;* see useful links below:
;*     https://stackoverflow.com/questions/48360238/how-can-the-l1-l2-l3-cpu-caches-be-turned-off-on-modern-x86-amd64-chips
;*     https://en.wikipedia.org/wiki/Memory_type_range_register
;************************************************************************

[bits 32]
[global disable_cache]
[global enable_cache]
IA32_MTRR_DEF_TYPE equ 0x2ff
disable_cache:
    pusha  ; important
    mov ecx, IA32_MTRR_DEF_TYPE
    rdmsr

    ;Step 1 - Enter no-fill mode
    mov eax, cr0
    or eax, 1<<30        ; Set bit CD
    and eax, ~(1<<29)    ; Clear bit NW
    mov cr0, eax

    ;Step 2 - Invalidate all the caches
    wbinvd               ; Invalidate cache, with writeback

    ;All memory accesses happen from/to memory now, but UC memory ordering may not be enforced still.  

    ;For Atom processors, we are done, UC semantic is automatically enforced.

    xor eax, eax
    xor edx, edx
    mov ecx, IA32_MTRR_DEF_TYPE    ; MSR number is 2FFH
    wrmsr                          ; Write Model-Specific Registers.
                                   ; Writes the contents of registers EDX:EAX into the 64-bit model specific register (MSR) specified in the ECX register.

    ;P4 only, remove this code from the L1I
    ;wbinvd

    popa
    ret                 ; important

enable_cache:
    pusha
    mov eax, cr0
    and eax, ~(1<<30)   ; Clear bit CD
    and eax, ~(1<<29)   ; Clear bit NW
    mov cr0, eax

    popa
    ret