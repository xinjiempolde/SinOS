#include <cpu/isr.h>
#include <libc/function.h>
#include <drivers/ports.h>
#include <cpu/timer.h>
#include <mm/memory.h>
#include <kernel/task.h>
/** 
 * from what I understand, an ISR should always be there, 
 * so it's better to set handler static, just better. it
 * can also work as same as deleting 'static'(i.e. function's address is same)
 */

static TIMER_MAN* timerMan;
void tune_timer(timer_t* timer);
/* handle IRQ0 */
static void timer_callback(registers_t r) {
    timer_t* p;
    timerMan->sysTimeCount++;

    UNUSED(r);
    
    /* there is no timer timing out */
    if (timerMan->sysTimeCount < timerMan->next->systimePlusCount) {
        return;
    }

    p = timerMan->next;
    while (p != NULL) {
        if (timerMan->sysTimeCount < p->systimePlusCount) {
            break;
        }
        // if (p->count == 2) {
        //     task_switch();
        //     restart_timer()
        // }
        p->timeoutFlags = TIME_OUT;
        p = p->nextNode;
    }
    timerMan->next = p;
    
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
    timerMan->next  = NULL;
    for (i = 0; i < MAX_TIMER_COUNT; i++) {
        timerMan->timerArray[i].useFlags = TIMER_NO_USE;
    }
}

timer_t* set_timer(unsigned int timeout) {
    timer_t* alloc_timer;
    int i;
    /* interrupt diable */
    asm volatile ("cli");
    for (i = 0; i < MAX_TIMER_COUNT; i++) {
        if (timerMan->timerArray[i].useFlags == TIMER_NO_USE) {
            alloc_timer = &(timerMan->timerArray[i]);
        }
    }

    alloc_timer->count = timeout;
    alloc_timer->systimePlusCount = timeout + timerMan->sysTimeCount;
    alloc_timer->useFlags = TIMER_IN_USE;
    alloc_timer->timeoutFlags = TIME_NO_OUT;
    
    /* sort timer */
    tune_timer(alloc_timer);
    
    /* interrupt enable */
    asm volatile ("sti");
    return alloc_timer;
}

void restart_timer(timer_t* timer) {
    asm volatile ("cli");

    timer->systimePlusCount = timerMan->sysTimeCount + timer->count;
    timer->timeoutFlags = TIME_NO_OUT;
    tune_timer(timer);

    asm volatile ("sti");
}

/**
 * move the timer to the correct position, which makes it order by asc.
 */
void tune_timer(timer_t* timer) {
    timer_t* p;

    /* there is no timer being set. */
    if (timerMan->next == NULL) {
        timerMan->next = timer;
        timer->nextNode = NULL;
        timer->preNode = NULL;
        return;
    }

    p = timerMan->next;
    if (timer->systimePlusCount < p->systimePlusCount) {
        timerMan->next = timer;
        if (p->preNode != NULL) {
            p->preNode->nextNode = timer;
        }
        timer->preNode = p->preNode;
        timer->nextNode = p;
        p->preNode = timer;
    } else {
        while (p->nextNode != NULL && 
            timer->systimePlusCount > p->nextNode->systimePlusCount) {
            
            p = p->nextNode;
        }

        if (p->nextNode != NULL) {
            p->nextNode->preNode = timer;
        }
        timer->nextNode = p->nextNode;
        p->nextNode = timer;
        timer->preNode = p;
    }

}