#include <cpu/isr.h>
#include <libc/function.h>
#include <drivers/ports.h>
/** 
 * from what I understand, an ISR should always be there, 
 * so it's better to set handler static, just better. it
 * can also work as same as deleting 'static'(i.e. function's address is same)
 */

unsigned int count = 0;

static void timer_callback(registers_t r) {
    count++;
    UNUSED(r);
    // char s[256];
    // uint_to_ascii(count, s);
    // kprint("tricks");
    // kprint(s);
    // kprint(" ");
}


void init_timer() {
    /**
     * see 8253 details below: 
     * https://wiki.osdev.org/Programmable_Interval_Timer
     */
    unsigned int frequency = 1193182 / 50;
    uint8_t low = frequency & 0xff;
    uint8_t high = (frequency >> 8) & 0xff;
    port_byte_out(0x43, 0x36);
    port_byte_out(0x40, low);
    port_byte_out(0x40, high);
    add_interrupt_handler(IRQ0, timer_callback);
}