#ifndef ISR_H
#define ISR_H
#include <cpu/type.h>

// 8259A I/O port
#define MASTER_PIC_CTRL 0x20
#define MASTER_PIC_DATA 0x21
#define SLAVE_PIC_CTRL 0xa0
#define SLAVE_PIC_DATA 0xa1

// IRQ maps to interrupt number
#define IRQ0 32
#define IRQ1 33
#define IRQ12 44

/* 
 * struct which aggregates many register
 * its value is from stack which assembly pushes
 */
typedef struct {
    uint32_t ds; //data segment selector
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha
    uint32_t int_no, err_code; // Interrupt number and error code
    uint32_t eip, cs, eflags, useresp, ss; // Pushed by the processor automatically
}__attribute__((packed)) registers_t;

/**
 * define function pointer, see example below:
 *     handler_fp fp = handler(register_t r) {};
 *     fp(r) or (*fp)(r) to call the function
 */
typedef void (*handler_fp)(registers_t);

//Interrupt Service Routine, implemented by assembly
extern void isr0();

extern void irq0();
extern void irq1();
extern void irq12();

void install_isr();
void install_irq();
void isr_handler(registers_t r);
void irq_handler(registers_t r);
void add_interrupt_handler(int n, handler_fp handler);
#endif