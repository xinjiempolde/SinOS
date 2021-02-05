#ifndef GDT_H
#define GDT_H

#include <cpu/type.h>

#define GDT_ADR 0x100000

typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access_type;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed)) gdt_entry; // packed is important

typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) gdt_ptr;

void gdt_set_gate(int idx, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);

void gdt_install(void);
#endif