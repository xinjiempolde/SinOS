;************************************************************************
;* enable/disabel cache
;* see useful links below:
;*     https://stackoverflow.com/questions/48360238/how-can-the-l1-l2-l3-cpu-caches-be-turned-off-on-modern-x86-amd64-chips
;*     https://en.wikipedia.org/wiki/Memory_type_range_register
;************************************************************************


[global disable_cache]
IA32_MTRR_DEF_TYPE equ 0x2ff
disable_cache:
    ;Step 1 - Enter no-fill mode
    mov eax, cr0
    or eax, 1<<30        ; Set bit CD
    and eax, ~(1<<29)    ; Clear bit NW
    mov cr0, eax

    ;Step 2 - Invalidate all the caches
    wbinvd

    ;All memory accesses happen from/to memory now, but UC memory ordering may not be enforced still.  

    ;For Atom processors, we are done, UC semantic is automatically enforced.

    xor eax, eax
    xor edx, edx
    mov ecx, IA32_MTRR_DEF_TYPE    ;MSR number is 2FFH
    wrmsr

    ;P4 only, remove this code from the L1I
    ;wbinvd