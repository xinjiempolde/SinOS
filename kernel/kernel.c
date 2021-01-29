#include <gui/vga.h>
#include <cpu/isr.h>
#include <drivers/mouse.h>
#include <mm/memory.h>
#include <gui/layer.h>
#include <gui/background.h>
#include <gui/window.h>
MemMan* memman = (MemMan*)MEM_MAN_ADDR;
LayerManager* layman;
Layer* mouse_layer;
void kernel_main() {
    init_palette();

    install_isr();
    install_irq();

    mem_init(memman);
    mem_free(memman, 0x200000, 0x8000000); // 2MB~128MB
    
    uint8_t* mouse_buf = (uint8_t*)mem_alloc_4k(memman, 1024);
    uint8_t* back_buf = (uint8_t*)mem_alloc_4k(memman, 0x10000);
    uint8_t* window_buf = (uint8_t*)mem_alloc_4k(memman, 0x10000);
    init_mouse_cursor(mouse_buf);
    init_background(back_buf);
    init_window_buf(window_buf, 160, 68, "window");
    
    layman = init_layman(memman);
    mouse_layer = alloc_layer(layman, mouse_buf, 16, 16, 2);
    Layer* back_layer = alloc_layer(layman, back_buf, SCREEN_H, SCREEN_W, 0);
    Layer* window_layer = alloc_layer(layman, window_buf, 68, 160, 1);
    add_layer(layman, mouse_layer);
    add_layer(layman, back_layer);
    add_layer(layman, window_layer);
    move_layer(layman, window_layer, 50, 50);

    /* just vmware's bug, add this line to avoid */
    fill_rect((uint8_t*)0xa0000, 0, 0, 0, 0, 0, BLACK);
    repaint_layers(layman);

    char* s_buf = (char*)mem_alloc_4k(memman, 64);
    unsigned int count = 0;
    unsigned int value = 0;
    while(1) {
        count++;
        if (count % 10000000 == 0) {
            value++;
            sprintf(s_buf, "%d", value);
            fill_rect(window_buf, 160, 40, 24, 120, 16, BRIGHT_GRAY);
            put_string(window_buf, 160, 40, 24, s_buf, BLACK);
            repaint_partial_layers(layman, window_layer->x + 40, window_layer->y+24, window_layer->x + 159, window_layer->y + 40);

        }
    }
}
