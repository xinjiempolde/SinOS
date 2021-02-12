#include <cpu/isr.h>
#include <libc/function.h>
#include <drivers/ports.h>
#include <cpu/timer.h>
#include <mm/memory.h>
/** 
 * from what I understand, an ISR should always be there, 
 * so it's better to set handler static, just better. it
 * can also work as same as deleting 'static'(i.e. function's address is same)
 */

static TIMER_MAN* timerMan;

/* handle IRQ0 */
static void timer_callback(registers_t r) {
    timerMan->sysTimeCount++;

    UNUSED(r);
    
    unsigned int i, j;
    if (timerMan->sysTimeCount < timerMan->nextTimeOut) {
        return;
    }

    /* delete the timer which is time out */
    for (i = timerMan->nextTimeOutIndex; i < timerMan->timerNum; i++) {
        if (timerMan->sysTimeCount < timerMan->timerIndexArray[i]->systimePlusCount) {
            break;
        }
        timerMan->timerIndexArray[i]->timeoutFlags = TIME_OUT;
    }
    timerMan->nextTimeOutIndex = i;
    // timerMan->timerNum -= i;
    // for (j = 0; (j < i) && (j+i < MAX_TIMER_COUNT); j++) {
        // timerMan->timerIndexArray[j] = timerMan->timerIndexArray[j+i];
    // }

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

    MemMan* memMan = (MemMan*)MEM_MAN_ADDR;
    int i;
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

    timerMan = (TIMER_MAN*)mem_alloc_4k(memMan, sizeof(TIMER_MAN));
    timerMan->sysTimeCount = 0;
    timerMan->timerNum = 0;
    timerMan->nextTimeOutIndex = 0;
    for (i = 0; i < MAX_TIMER_COUNT; i++) {
        timerMan->timerArray[i].useFlags = TIMER_NO_USE;
    }
}

timer_t* set_timer(unsigned int timeout) {
    timer_t* alloc_timer;
    int i, j, index;

    /* interrupt diable */
    asm volatile ("cli");
    for (i = 0; i < MAX_TIMER_COUNT; i++) {
        if (timerMan->timerArray[i].useFlags == TIMER_NO_USE){
            alloc_timer = &(timerMan->timerArray[i]);
            break;
        }
    }

    /* insert into the correct position order by asc */
    alloc_timer->count = timeout;
    alloc_timer->systimePlusCount = timeout + timerMan->sysTimeCount;
    for (i = 0; i < timerMan->timerNum; i++) {
        if (alloc_timer->systimePlusCount < timerMan->timerIndexArray[i]->systimePlusCount) {
            break;
        }
    }
    /* move one by one */
    for (j = timerMan->timerNum; j > i; j--) {
        timerMan->timerIndexArray[j] = timerMan->timerIndexArray[j-1];
    }
    timerMan->timerIndexArray[i] = alloc_timer;
    timerMan->timerNum++;
    alloc_timer->useFlags = TIMER_IN_USE;

    index = timerMan->nextTimeOutIndex;
    timerMan->nextTimeOut = timerMan->timerIndexArray[index]->systimePlusCount;

    /* interrupt enable */
    asm volatile ("sti");
    return alloc_timer;
}

void restart_timer(timer_t* timer) {
    asm volatile ("cli");

    timer->systimePlusCount = timerMan->sysTimeCount + timer->count;
    timer->timeoutFlags = TIME_NO_OUT;

    asm volatile ("sti");
}