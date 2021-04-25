[bits 16]
switch_to_pm:
    cli
    lgdt [gdt_descriptor]
    mov ebx, cr0
    or ebx, 0x1
    mov cr0, ebx
    jmp CODE_SEGMENT:(LoaderAddress+start_of_pm)  ;it's important to add LoaderAddress!!!

[bits 32]
start_of_pm:
    mov eax, DATA_SEGMENT
    mov ds, eax
    mov ss, eax
    mov es, eax
    mov fs, eax
    mov gs, eax

    mov ebp, 0x90000
    mov esp, ebp
    jmp BEGIN_PM