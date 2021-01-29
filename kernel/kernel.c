#include <drivers/vga.h>
#include <cpu/isr.h>
#include <drivers/mouse.h>
#include <mm/memory.h>
#include <gui/layer.h>
#include <gui/background.h>
MemMan* memman = (MemMan*)MEM_MAN_ADDR;
LayerManager* layman;
Layer* mouse_layer;
void kernel_main() {
    init_palette();

    install_isr();
    install_irq();

    mem_init(memman);
    mem_free(memman, 0x200000, 0x8000000); // 2MB~128MB
    
    uint8_t* mouse_buf = mem_alloc_4k(memman, 1024);
    uint8_t* back_buf = mem_alloc_4k(memman, 0x10000);
    init_mouse_cursor(mouse_buf);
    init_background(back_buf);
    
    layman = init_layman(memman);
    mouse_layer = alloc_layer(layman, mouse_buf, 16, 16, 1);
    Layer* back_layer = alloc_layer(layman, back_buf, SCREEN_H, SCREEN_W, 0);
    add_layer(layman, mouse_layer);
    add_layer(layman, back_layer);

    /* just vmware's bug, add this line to avoid */
    fill_rect((uint8_t*)0xa0000, 0, 0, 0, 0, BLACK);
    repaint_layers(layman);
    while(1);
}
