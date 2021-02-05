#include <cpu/idt.h>
// idt_gate_t* idt = (idt_gate_t*)IDT_ADR;
idt_gate_t idt[IDT_ENTRIES];
/**
 * you can see the details below:
 * https://wiki.osdev.org/Interrupt_Descriptor_Table
 */
void set_idt_gate(int n, uint32_t isr_addr) {
    idt[n].offset_1 = low_16(isr_addr);
    idt[n].selector = KERNEL_CS;
    idt[n].zero = 0;
    idt[n].type_attr = 0x8e;
    idt[n].offset_2 = high_16(isr_addr);
}

void set_idt() {
    idt_register_t idt_reg;
    idt_reg.Limit = IDT_ENTRIES * sizeof(idt_gate_t) - 1;
    idt_reg.Base = (uint32_t)idt;
    // volatile: Declare to GCC that optimization of the inline assembly is not allowed
    /**
     * lidt [idt_address] = mov ebx, idt_address; lidt [ebx];
     * and in at&t assembly, [ebx] equals to (ebx),so the assembly can be
     * transfered to inline assembly c below;
     */
    __asm__ __volatile__ ("lidt (%0)"::"r"(&idt_reg)); //don't forget address &
}