#include <kernel/task.h>
#include <cpu/gdt.h>
#include <gui/vga.h>
#include <mm/memory.h>
#include <gui/window.h>
#include <gui/layer.h>
#include <cpu/timer.h>
TSS tss_curr;
TSS tss_test;
extern LayerManager* layman;
static unsigned int count = 0;
static MemMan* memMan;
static uint8_t* tt_window_buf;
static Layer* tt_window_layer;
static timer_t* sec3timer;
static int times = 0;
void task_test() {
    if (count == 0) {
        memMan = (MemMan*)MEM_MAN_ADDR;
        tt_window_buf = (uint8_t*)mem_alloc_4k(memMan, 0x10000);
        tt_window_layer = alloc_layer(layman, tt_window_buf, 68, 160, 8);
        init_window_buf(tt_window_buf, 160, 68, "task_test");
        add_layer(layman, tt_window_layer);
        move_layer(layman, tt_window_layer, 100, 100);
        sec3timer = set_timer(2);

    } else {

    }
    char* s_buf = (char*)mem_alloc_4k(memMan, 64);
    while (1) {
        count++;
        if (sec3timer->timeoutFlags == TIME_OUT) {
            asm volatile ("ljmp $0x18, $0x00");
            restart_timer(sec3timer);
        }
        fill_rect(tt_window_buf, 160, 20, 20, 120, 16, BRIGHT_GRAY);
        sprintf(s_buf, "%d", count);
        put_string(tt_window_buf, 160, 20, 20, s_buf, BLACK);
        refresh_partial_map(layman, tt_window_layer->x+20, 
                tt_window_layer->y+20, tt_window_layer->x+20+120, tt_window_layer->y+20+16);
        repaint_partial_layers(layman, tt_window_layer->x+20, 
                tt_window_layer->y+20, tt_window_layer->x+20+120, tt_window_layer->y+20+16, tt_window_layer->z);

    }
    
}

void task_switch() {
    if (times == 0) {
        times++;
    /* current task status segment, it's necessary for switching task */
    tss_curr.LDTR = 0;
    tss_curr.IOPB = sizeof(TSS) << 16;

    tss_test.EIP = (uint32_t)&task_test;
    tss_test.EFLAGS = 0x202;
    tss_test.CS = 8;
    tss_test.DS = 16;
    tss_test.ES = 16;
    tss_test.SS = 16;

    tss_test.ESI = 0;
    tss_test.EDI = 0;
    tss_test.ESP = 0x2000000;
    tss_test.IOPB = sizeof(TSS) << 16;
    tss_test.LDTR = 0;

    gdt_set_gate(3, &tss_curr, sizeof(TSS), 0x89, 0x40);
    gdt_set_gate(4, &tss_test, sizeof(TSS), 0x89, 0x40);

    /* note: ltr loads current task rather than that we will switch to */
    asm volatile("mov $0x18, %%ax\n\t"
                "ltr %%ax\n\t"
                "ljmp $0x20, $0x00"
                :
                :
                : "%ax" );
   } else {
       asm volatile ("ljmp $0x20, $0x00");
   }

}