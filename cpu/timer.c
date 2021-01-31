#include <cpu/isr.h>
#include <libc/function.h>
#include <drivers/ports.h>
/** 
 * from what I understand, an ISR should always be there, 
 * so it's better to set handler static, just better. it
 * can also work as same as deleting 'static'(i.e. function's address is same)
 */

unsigned int timer_count = 0;

static void timer_callback(registers_t r) {
    timer_count++;
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
    unsigned int frequency = 1193182 / 100; // max frequency is 65535
                                            // this frequency means 100 IRQ0 per second
    uint8_t low = frequency & 0xff;
    uint8_t high = (frequency >> 8) & 0xff;
    /**
     * command data: 0x36(0011 0110)
     * bit 0 = 0, 16-bit binary mode
     * bit 1~3=110 (rate generator, same as 010b)
     * bit 4~5=11 Access mode: lobyte/hibyte
     * bit 6~7=00 select channel 0
     */
    port_byte_out(0x43, 0x36);
    port_byte_out(0x40, low);
    port_byte_out(0x40, high);
    add_interrupt_handler(IRQ0, timer_callback);
}