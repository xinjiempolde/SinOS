#include <drivers/vga.h>
#include <cpu/isr.h>
#include <drivers/mouse.h>
#include <mm/memory.h>
void kernel_main() {
    init_palette();

    fill_rect(0, 0, SCREEN_W, SCREEN_H - SLIDE_BAR_H, LIGHT_BRIGHT_BLUE);
    fill_rect(0, SCREEN_H - SLIDE_BAR_H - 3, SCREEN_W, 1, BRIGHT_GRAY);
    fill_rect(0, SCREEN_H - SLIDE_BAR_H - 2, SCREEN_W, 1, WHITE);
    fill_rect(0, SCREEN_H - SLIDE_BAR_H - 1, SCREEN_W, 1, BRIGHT_GRAY);

    fill_rect(0, SCREEN_H - SLIDE_BAR_H, SCREEN_W, SLIDE_BAR_H, DARK_GRAY);

    fill_rect(2, SCREEN_H - SLIDE_BAR_H + 2, 60, SLIDE_BAR_H - 4, BLACK);
    fill_rect(2, SCREEN_H - SLIDE_BAR_H + 2, 58, SLIDE_BAR_H - 6, WHITE);
    fill_rect(4, SCREEN_H - SLIDE_BAR_H + 4, 56, SLIDE_BAR_H - 8, DARK_GRAY);

    init_mouse_cursor();

    install_isr();
    install_irq();

    memcheck(0x00400000, 0xbfffffff);
    while(1);
}
