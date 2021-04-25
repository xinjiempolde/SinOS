#include <cpu/isr.h>
#include <cpu/idt.h>
#include <cpu/timer.h>
#include <drivers/ports.h>
#include <drivers/keyboard.h>
#include <drivers/mouse.h>
#include <libc/string.h>
#include <gui/vga.h>
handler_fp interrupt_handler_arr[256];
void install_isr() {
    /* 32 idt_gates for cpu exception */
    set_idt_gate(0, (uint32_t)isr0);
    set_idt_gate(1, (uint32_t)isr1);
    set_idt_gate(2, (uint32_t)isr2);
    set_idt_gate(3, (uint32_t)isr3);
    set_idt_gate(4, (uint32_t)isr4);
    set_idt_gate(5, (uint32_t)isr5);
    set_idt_gate(6, (uint32_t)isr6);
    set_idt_gate(7, (uint32_t)isr7);
    set_idt_gate(8, (uint32_t)isr8);
    set_idt_gate(9, (uint32_t)isr9);
    set_idt_gate(10, (uint32_t)isr10);
    set_idt_gate(11, (uint32_t)isr11);
    set_idt_gate(12, (uint32_t)isr12);
    set_idt_gate(13, (uint32_t)isr13);
    set_idt_gate(14, (uint32_t)isr14);
    set_idt_gate(15, (uint32_t)isr15);
    set_idt_gate(16, (uint32_t)isr16);
    set_idt_gate(17, (uint32_t)isr17);
    set_idt_gate(18, (uint32_t)isr18);
    set_idt_gate(19, (uint32_t)isr19);
    set_idt_gate(20, (uint32_t)isr20);
    set_idt_gate(21, (uint32_t)isr21);
    set_idt_gate(22, (uint32_t)isr22);
    set_idt_gate(23, (uint32_t)isr23);
    set_idt_gate(24, (uint32_t)isr24);
    set_idt_gate(25, (uint32_t)isr25);
    set_idt_gate(26, (uint32_t)isr26);
    set_idt_gate(27, (uint32_t)isr27);
    set_idt_gate(28, (uint32_t)isr28);
    set_idt_gate(29, (uint32_t)isr29);
    set_idt_gate(30, (uint32_t)isr30);
    set_idt_gate(31, (uint32_t)isr31);

    /* set master 8259A */
    port_byte_out(MASTER_PIC_CTRL, 0x11); // ICW1, 边缘检测 & 级联使用
    port_byte_out(MASTER_PIC_DATA, 0x20); // ICW2, IRQ0~IRQ7 => int 32~39, 0~31被CPU内部异常使用
    port_byte_out(MASTER_PIC_DATA, 0x04); // ICW3, 00000100 means master 8259's IRQ2 is connected to slave 8259A
    port_byte_out(MASTER_PIC_DATA, 0x01); // ICW4，完全嵌套方式 & 非缓冲方式 & 手动EOI方式
    port_byte_out(MASTER_PIC_DATA, 0x00); // OCW1, mask nothing

    /* set slave 8259A */
    port_byte_out(SLAVE_PIC_CTRL, 0x11); // ICW1
    port_byte_out(SLAVE_PIC_DATA, 0x28); // ICW2, IRQ8~IRQ15 => int 40~47
    port_byte_out(SLAVE_PIC_DATA, 0x02); // ICW3, 00000 010 means connects to master 8259A's IRQ2
    port_byte_out(SLAVE_PIC_DATA, 0x01); // ICW4
    port_byte_out(SLAVE_PIC_DATA, 0x00); // OCW1,mask hard disk IRQ

    /* customered interrupt */
    set_idt_gate(32, (uint32_t)irq0); // IRQ0 timer
    set_idt_gate(33, (uint32_t)irq1); // IRQ1 keyboard
    set_idt_gate(44, (uint32_t)irq12); // IRQ12 PS/2 mouse
    set_idt_gate(46, (uint32_t)irq14); // IRQ14 ide hard disk
    set_idt_gate(47, (uint32_t)irq15); 

    /* load idt with lidt */
    set_idt();
}

void test_ide() {

}

void install_irq() {
    __asm__ __volatile__ ("sti");
    init_timer();
    init_keyboard();
    init_mouse();
    add_interrupt_handler(46, test_ide);
    add_interrupt_handler(47, test_ide);
}


/* To print the message which defines every exception */
char *exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",

    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",

    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",

    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

void isr_handler(registers_t r) {
    printf("int number: %x", r.int_no);
    printf("error code: %x", r.err_code);
    printf("eip is: %x", r.eip);
    printf(exception_messages[r.int_no]);
    asm volatile ("sti");
    return;
}

void irq_handler(registers_t r) {

    /* After every interrupt we need to send an EOI to the PICs
     * or they will not send another interrupt again */
    if (r.int_no >= 40) { // if IRQ comes from slave PIC
        port_byte_out(SLAVE_PIC_CTRL, 0x20); // OCW2
    }
    port_byte_out(MASTER_PIC_CTRL, 0x20);

    // if not null
    if (interrupt_handler_arr[r.int_no] != 0) {
        handler_fp handler = interrupt_handler_arr[r.int_no];
        handler(r);
    }
}

void add_interrupt_handler(int n, handler_fp handler) {
    interrupt_handler_arr[n] = handler;
}