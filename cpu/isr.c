#include <cpu/isr.h>
#include <cpu/idt.h>
#include <cpu/timer.h>
#include <drivers/ports.h>
#include <drivers/keyboard.h>
#include <drivers/mouse.h>
#include <libc/string.h>
handler_fp interrupt_handler_arr[256];
void install_isr() {
    /* 32 idt_gates for cpu exception */
    set_idt_gate(0, (uint32_t)isr0);

    /* set master 8259A */
    port_byte_out(MASTER_PIC_CTRL, 0x11); // ICW1
    port_byte_out(MASTER_PIC_DATA, 0x20); // ICW2, IRQ0~IRQ7 => int 32~39
    port_byte_out(MASTER_PIC_DATA, 0x04); // ICW3, 00000100 means master 8259's IRQ2 is connected to slave 8259A
    port_byte_out(MASTER_PIC_DATA, 0x01); // ICW4
    port_byte_out(MASTER_PIC_DATA, 0x00); // OCW1, mask nothing

    /* set slave 8259A */
    port_byte_out(SLAVE_PIC_CTRL, 0x11); // ICW1
    port_byte_out(SLAVE_PIC_DATA, 0x28); // ICW2, IRQ8~IRQ15 => int 40~47
    port_byte_out(SLAVE_PIC_DATA, 0x02); // ICW3, 00000 010 means connects to master 8259A's IRQ2
    port_byte_out(SLAVE_PIC_DATA, 0x01); // ICW4
    port_byte_out(SLAVE_PIC_DATA, 0x00); // OCW1

    /* customered interrupt */
    set_idt_gate(32, (uint32_t)irq0); // IRQ0 timer
    set_idt_gate(33, (uint32_t)irq1); // IRQ1 keyboard
    set_idt_gate(44, (uint32_t)irq12); // IRQ12 PS/2 mouse

    /* load idt with lidt */
    set_idt();
}

void install_irq() {
    __asm__ __volatile__ ("sti");
    init_timer();
    init_keyboard();
    init_mouse();
}



void isr_handler(registers_t r) {
    //kprint("interrupt number:");
    char s[3];
    uint_to_dec(r.int_no, s);
    //kprint(s);

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