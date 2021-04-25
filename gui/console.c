#include <gui/console.h>
#include <gui/vga.h>
#include <gui/layer.h>
#include <libc/string.h>
void init_console_buf(uint8_t* buf, int weight, int height) {
    init_window_buf(buf, weight, height, "console");
    fill_rect(buf, weight, DFT_CSL_BOR, DFT_CSL_TIT_H, weight - 2 * DFT_CSL_BOR, height - (DFT_CSL_TIT_H+DFT_CSL_BOR), BLACK);    
    put_char(buf, DFT_CSL_W, DFT_CSL_BOR, DFT_CSL_TIT_H, '#', BRIGHT_GREEN);
}

/**
 * new console line
 * return the cursor's y coordinate
 */
int console_newline(Layer* layer, int cursorY) {
    if (cursorY < DFT_CSL_H - DFT_CSL_BOR - CHAR_H) {
        cursorY += CHAR_H;
        return cursorY;
    } else { // should scroll window
        int i, j, k;

        // copy the contents of the next line to the previous line
        for (i = 0; i < DFT_CSL_CON_H - 1; i++) {
            for (j = 0; j < CHAR_H; j++) {
                for (k = DFT_CSL_BOR; k < DFT_CSL_W - DFT_CSL_BOR; k++) {
                    layer->buf[(DFT_CSL_TIT_H+CHAR_H*i+j)*DFT_CSL_W + k] = \
                    layer->buf[(DFT_CSL_TIT_H+CHAR_H*(i+1)+j)*DFT_CSL_W + k];
                }
            }
        }

        /* cover the last line */
        fill_rect(layer->buf, layer->weight, DFT_CSL_BOR, DFT_CSL_H-DFT_CSL_BOR-CHAR_H, CSL_CON_H_SIZE, CHAR_H, BLACK);
        // repaint content area (i.e. black color area)
        repaint_single_layer(layer, DFT_CSL_BOR, DFT_CSL_TIT_H, CSL_CON_H_SIZE, CSL_CON_V_SIZE);
        return DFT_CSL_H - DFT_CSL_BOR - CHAR_H;
    }

}

void console_clear(Layer* console_layer) {
    put_rect_refresh(console_layer, DFT_CSL_BOR, DFT_CSL_TIT_H, CSL_CON_H_SIZE, CSL_CON_V_SIZE, BLACK);
}
