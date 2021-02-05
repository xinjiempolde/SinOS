#include <cpu/gdt.h>
static gdt_entry* gdt = (gdt_entry*)GDT_ADR;

void gdt_set_gate(int idx, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    (gdt+idx)->limit_low = low_16(limit);
    (gdt+idx)->base_low = low_16(base);
    (gdt+idx)->base_middle = high_16(base) & 0xff;
    (gdt+idx)->access_type = access;
    (gdt+idx)->granularity = gran;
    (gdt+idx)->base_high = (high_16(base) >> 8) & 0xff;
}

void gdt_install(void) {
    gdt_ptr gp;
    gp.base = GDT_ADR;
    gp.limit = sizeof(gdt_entry) * 5 - 1;

    /* NULL descriptor */
    gdt_set_gate(0, 0, 0, 0, 0);

    /* code segment descriptor */
    gdt_set_gate(1, 0, 0xfffff, 0x9a, 0xcf);

    /* data segment descriptor */
    gdt_set_gate(2, 0, 0xfffff, 0x92, 0xcf);

    asm volatile ("lgdt (%0)":: "r"(&gp));
    
}