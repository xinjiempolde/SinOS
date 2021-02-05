#include <kernel/kernel.h>
#include <gui/vga.h>
#include <cpu/isr.h>
#include <drivers/mouse.h>
#include <mm/memory.h>
#include <gui/layer.h>
#include <gui/background.h>
#include <gui/window.h>
#include <cpu/gdt.h>
#include <kernel/task.h>
MemMan* memman = (MemMan*)MEM_MAN_ADDR;
LayerManager* layman;
Layer* mouse_layer;
Layer* window_layer;
extern unsigned int timer_count;
void kernel_main() {
    gdt_install();

    BOOTINFO* bootInfo = (BOOTINFO*)BOOTINFO_ADDR;
    if (bootInfo->reset_palette == RESET_PALETTE) {
        init_palette();
    }
    install_isr();
    install_irq();
    mem_init(memman);
    mem_free(memman, 0x200000, 0x8000000); // 2MB~128MB
    
    uint8_t* mouse_buf = (uint8_t*)mem_alloc_4k(memman, 1024);
    // uint8_t* back_buf = (uint8_t*)mem_alloc_4k(memman, bootInfo->screen_h*bootInfo->screen_w*0x10); // should modify this
    uint8_t* back_buf = (uint8_t*)mem_alloc_4k(memman, 0x1000000); // should modify this
    uint8_t* window_buf = (uint8_t*)mem_alloc_4k(memman, 0x10000);
    init_mouse_cursor(mouse_buf);
    init_background(back_buf);
    init_window_buf(window_buf, 160, 68, "window");
    
    layman = init_layman(memman);
    mouse_layer = alloc_layer(layman, mouse_buf, 16, 16, 2);
    Layer* back_layer = alloc_layer(layman, back_buf, bootInfo->screen_h, bootInfo->screen_w, 0);
    window_layer = alloc_layer(layman, window_buf, 68, 160, 1);
    add_layer(layman, mouse_layer);
    add_layer(layman, back_layer);
    add_layer(layman, window_layer);
    move_layer(layman, window_layer, 50, 50);

    /* just vmware's bug, add this line to avoid */
    fill_rect((uint8_t*)bootInfo->vram, 0, 0, 0, 0, 0, BLACK);
    repaint_layers(layman);

    char* s_buf = (char*)mem_alloc_4k(memman, 64);
    while(1) {
        if (timer_count / 100 == 3) {
            task_switch();
        }
        sprintf(s_buf, "%d", timer_count/100);
        fill_rect(window_buf, 160, 40, 24, 100, 16, BRIGHT_GRAY);
        put_string(window_buf, 160, 40, 24, s_buf, BLACK);
        repaint_partial_layers(layman, window_layer->x + 40, window_layer->y+24, window_layer->x + 159, window_layer->y + 40, window_layer->z);
        
    }
}
