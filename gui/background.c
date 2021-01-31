#include <gui/background.h>
#include <gui/vga.h>
void init_background(uint8_t* buf) {
    BOOTINFO* bootInfo = (BOOTINFO*)BOOTINFO_ADDR; 
    fill_rect(buf, bootInfo->screen_w, 0, 0, bootInfo->screen_w, bootInfo->screen_h - SLIDE_BAR_H, LIGHT_BRIGHT_BLUE);
    fill_rect(buf, bootInfo->screen_w, 0, bootInfo->screen_h - SLIDE_BAR_H - 3, bootInfo->screen_w, 1, BRIGHT_GRAY);
    fill_rect(buf, bootInfo->screen_w, 0, bootInfo->screen_h - SLIDE_BAR_H - 2, bootInfo->screen_w, 1, WHITE);
    fill_rect(buf, bootInfo->screen_w, 0, bootInfo->screen_h - SLIDE_BAR_H - 1, bootInfo->screen_w, 1, BRIGHT_GRAY);

    fill_rect(buf, bootInfo->screen_w, 0, bootInfo->screen_h - SLIDE_BAR_H, bootInfo->screen_w, SLIDE_BAR_H, DARK_GRAY);

    fill_rect(buf, bootInfo->screen_w, 2, bootInfo->screen_h - SLIDE_BAR_H + 2, 60, SLIDE_BAR_H - 4, BLACK);
    fill_rect(buf, bootInfo->screen_w, 2, bootInfo->screen_h - SLIDE_BAR_H + 2, 58, SLIDE_BAR_H - 6, WHITE);
    fill_rect(buf, bootInfo->screen_w, 4, bootInfo->screen_h - SLIDE_BAR_H + 4, 56, SLIDE_BAR_H - 8, DARK_GRAY);
   
}