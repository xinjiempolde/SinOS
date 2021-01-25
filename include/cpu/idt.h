#ifndef IDT_H
#define IDT_H

#include "type.h"

/* the gdt_code offset represents code segment selector */
#define KERNEL_CS 0x08

// idt_entry or idt_gate (idt has 256 idt_gates)
typedef struct {
    uint16_t offset_1; // offset bits 0..15
    uint16_t selector; // a code segment selector in GDT or LDT
    uint8_t zero;      // unused, set to 0
    uint8_t type_attr; // type and attributes
    uint16_t offset_2; // offset bits 16..31
} __attribute__((packed)) idt_gate_t; //cancel byte aligned

typedef struct {
    uint16_t Limit; // the length of the IDT in bytes - 1
    uint32_t Base;  // linear address where the IDT starts (INT 0)

} __attribute__((packed)) idt_register_t;


#define IDT_ENTRIES 256

void set_idt_gate(int n, uint32_t isr_addr);
void set_idt();
#endif