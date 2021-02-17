#include <gui/console.h>
#include <gui/vga.h>

void init_console_buf(uint8_t* buf, int weight, int height) {
    init_window_buf(buf, weight, height, "console");
    fill_rect(buf, weight, 4, 24, weight - 8, height - 28, BLACK);    
    put_char(buf, DFT_CSL_W, 4, 24, '#', BRIGHT_GREEN);
}