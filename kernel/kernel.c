#include <kernel/kernel.h>
#include <gui/vga.h>
#include <cpu/isr.h>
#include <drivers/mouse.h>
#include <mm/memory.h>
#include <gui/layer.h>
#include <gui/background.h>
#include <gui/window.h>
#include <gui/console.h>
#include <cpu/gdt.h>
#include <kernel/task.h>
#include <cpu/timer.h>
#include <fs/fs.h>
MemMan* memman = (MemMan*)MEM_MAN_ADDR;
LayerManager* layman;
Layer* mouse_layer;
Layer* window_layer;
extern super_block sb;

void kernel_main() {
    gdt_install();

    BOOTINFO* bootInfo = (BOOTINFO*)BOOTINFO_ADDR;
    if (bootInfo->reset_palette == RESET_PALETTE) {
        init_palette();
    }

    /* initialize memory manager which is preceded by install_irq */
    mem_init(memman);
    mem_free(memman, 0x200000, 0x8000000); // 2MB~128MB
    
    install_isr();
    install_irq();
    uint8_t* mouse_buf = (uint8_t*)mem_alloc_4k(memman, 1024);
    uint8_t* back_buf = (uint8_t*)mem_alloc_4k(memman, bootInfo->screen_h*bootInfo->screen_w); // should modify this
    uint8_t* window_buf = (uint8_t*)mem_alloc_4k(memman, 160*68);
    init_mouse_cursor(mouse_buf);
    init_background(back_buf);
    init_window_buf(window_buf, 160, 68, "window");
    
    layman = init_layman(memman);
    mouse_layer = alloc_layer(layman, mouse_buf, 16, 16, 100);
    alloc_layer(layman, back_buf, bootInfo->screen_h, bootInfo->screen_w, 0);
    window_layer = alloc_layer(layman, window_buf, 68, 160, 1);
    move_layer(window_layer, 200, 200);
    

    char* s_buf = (char*)mem_alloc_4k(memman, 64);
    timer_t* sec3Timer = set_timer(2);
    unsigned int count = 0;

    task_init();

    init_sb(&sb);
    if (sb.magic != 0x66) {
        init_fs();
    }

    init_fs();
    
    Task* mytask = task_alloc();
    mytask->tss.EIP = (uint32_t)(&console_task);
    mytask->tss.ESP = mem_alloc_4k(memman, sizeof(0x1000));


        uint8_t* new_buf = mem_alloc_4k(memman, sizeof(400*600));
    char* file_name_list[5] = {"usr", "home", "media", "root", "lostfound"};
    init_file_list(new_buf, 600, 400, "new window", file_name_list, 5);
    Layer* new_layer = alloc_layer(layman, new_buf, 400, 600, 20);
    move_layer(new_layer, 100, 100);
    while (1);
    
    // while(1) {
    //     count++;
    //     if (sec3Timer->timeoutFlags == TIME_OUT) {
    //         //task_switch();
    //         asm volatile ("ljmp $32, $0x0");
    //         restart_timer(sec3Timer);
    //     }
    //     if (count % 100 == 0) {
    //         fill_rect(window_buf, 160, 20, 20, 120, 16, BRIGHT_GRAY);
    //         sprintf(s_buf, "%d", count);
    //         put_string(window_buf, 160, 20, 20, s_buf, BLACK);
    //         refresh_partial_map(layman, window_layer->x+20, 
    //             window_layer->y+20, window_layer->x+20+120, window_layer->y+20+16);
    //         repaint_partial_layers(layman, window_layer->x+20, 
    //             window_layer->y+20, window_layer->x+20+120, window_layer->y+20+16, window_layer->z);

    //     }
    // }
}
