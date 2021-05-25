#include <gui/gedit.h>
#include <drivers/keyboard.h>
#include <gui/layer.h>
#include <gui/vga.h>
#include <cpu/timer.h>
#include <libc/string.h>
extern LayerManager* layman;
extern char key_buff[256];
extern bool readable;
static int cursorX = DFT_EDIT_BOR;
static int cursorY = DFT_EDIT_TIT_H;
static int line_num = 0; // 目前光标在内容区的第几行
static int kb_idx = 0; // keyboard buffuer index

char content[DFT_EDIT_CON_H][DFT_EDIT_CON_W] = {0}; // 用于保存每一行的数据

Layer* gedit_layer;

void init_gedit_buf(uint8_t* buf, int weight, int height) {
    init_window_buf(buf, weight, height, "gedit");
    fill_rect(buf, weight, DFT_EDIT_BOR, DFT_EDIT_TIT_H, weight - 2 * DFT_EDIT_BOR, height - (DFT_EDIT_TIT_H+DFT_EDIT_BOR), BLACK);
}

int gedit_run() {


    int cursorColor = WHITE;

    timer_t* cursorTimer = set_timer(50);

    MemMan* memMan = (MemMan*)MEM_MAN_ADDR;

    uint8_t* gedit_buf = (uint8_t*)mem_allocate(memMan, DFT_EDIT_W * DFT_EDIT_H);

    init_gedit_buf(gedit_buf, DFT_EDIT_W, DFT_EDIT_H);
    gedit_layer = alloc_layer(layman, gedit_buf, DFT_EDIT_H, DFT_EDIT_W, 6);
    move_layer(gedit_layer, 400, 400);

    init_all_var();
    while (1) {
        if (readable) {
            del_layer(gedit_layer);
            break;
        }
        /* keyboard buff is not null */
        if (key_buff[kb_idx] != '\0') {
            if (key_buff[kb_idx] == KEY_BACKSPACE) {
                edit_backspace();
            } else if (key_buff[kb_idx] == KEY_ENTER) {
                edit_enter();
            } else if (key_buff[kb_idx] == KEY_ESC){
                del_layer(gedit_layer);
                readable = TRUE;
                break;
            } else {
                append(content[line_num], key_buff[kb_idx]);        
                fill_rect(gedit_buf, DFT_EDIT_W, cursorX, cursorY, CHAR_W, CHAR_H, BLACK);
                put_char(gedit_buf, DFT_EDIT_W, cursorX, cursorY, key_buff[kb_idx], WHITE);
                repaint_single_layer(gedit_layer, cursorX, cursorY, CHAR_W, CHAR_H);
                cursorX += CHAR_W;
                if (cursorX >= DFT_EDIT_BOR + EDIT_CON_H_SIZE) {
                    cursorX = DFT_EDIT_BOR;
                    cursorY = edit_newline(gedit_layer, cursorY);
                }
            }
            kb_idx++;
        }
        /* cursor blinks */
        if (cursorTimer->timeoutFlags == TIME_OUT) {
            if (cursorColor == WHITE) {
                cursorColor = BLACK;
            } else {
                cursorColor = WHITE;
            }
            restart_timer(cursorTimer);
        }
        fill_rect(gedit_buf, DFT_EDIT_W, cursorX, cursorY, 8, 16, cursorColor);
        repaint_single_layer(gedit_layer, cursorX, cursorY, CHAR_W, CHAR_H);
    }

    return line_num+1;

}

int edit_newline(Layer* layer, int cursorY) {
    if (cursorY < DFT_EDIT_H - DFT_EDIT_BOR - CHAR_H) {
        cursorY += CHAR_H;
        return cursorY;
    } else { // should scroll window
        int i, j, k;

        // copy the contents of the next line to the previous line
        for (i = 0; i < DFT_EDIT_CON_H - 1; i++) {
            for (j = 0; j < CHAR_H; j++) {
                for (k = DFT_EDIT_BOR; k < DFT_EDIT_W - DFT_EDIT_BOR; k++) {
                    layer->buf[(DFT_EDIT_TIT_H+CHAR_H*i+j)*DFT_EDIT_W + k] = \
                    layer->buf[(DFT_EDIT_TIT_H+CHAR_H*(i+1)+j)*DFT_EDIT_W + k];
                }
            }
        }

        /* cover the last line */
        fill_rect(layer->buf, layer->weight, DFT_EDIT_BOR, DFT_EDIT_H-DFT_EDIT_BOR-CHAR_H, EDIT_CON_H_SIZE, CHAR_H, BLACK);
        // repaint content area (i.e. black color area)
        repaint_single_layer(layer, DFT_EDIT_BOR, DFT_EDIT_TIT_H, EDIT_CON_H_SIZE, EDIT_CON_V_SIZE);
        return DFT_EDIT_H - DFT_EDIT_BOR - CHAR_H;
    }

}

/* 通过cursorY获取这是第几行 */
int get_line_num(int cursorY) {
    return (cursorY - DFT_EDIT_TIT_H) / CHAR_H;
}

void edit_backspace() {
    put_rect_refresh(gedit_layer, cursorX, cursorY, CHAR_W, CHAR_H, BLACK); // 避免这行留白方块
    cursorX -= CHAR_W;

    /* 定位光标的位置 */
    if (cursorX < DFT_EDIT_BOR) { // 光标已经到行最开始
        line_num--;
        cursorX = DFT_EDIT_BOR + strlen(content[line_num]) * CHAR_W - CHAR_W;
        cursorY -= CHAR_H;
        if (cursorY < DFT_EDIT_TIT_H) { // 光标已经到第一行
            cursorX = DFT_EDIT_BOR;
            cursorY = DFT_EDIT_TIT_H;
            return;
        }
    }

    backspace(content[line_num]);
    put_rect_refresh(gedit_layer, cursorX, cursorY, 2 * CHAR_W, CHAR_H, BLACK);
}

void edit_enter() {
    append(content[line_num], ASCII_NL);
    append(content[line_num], 0);
    line_num++; // 下一行

    put_rect_refresh(gedit_layer, cursorX, cursorY, CHAR_W, CHAR_H, BLACK);
    cursorX = DFT_EDIT_BOR;
    cursorY = edit_newline(gedit_layer, cursorY);

    strclear(key_buff);
    kb_idx = -1; // why not 0? kb_idx++ will add 1, so don't worry
}

void init_all_var() {
    int i;
    cursorX = DFT_EDIT_BOR;
    cursorY = DFT_EDIT_TIT_H;
    line_num = 0; // 目前光标在内容区的第几行
    kb_idx = 0; // keyboard buffuer index

    strclear(key_buff);

    for (i = 0; i < DFT_EDIT_CON_H; i++) {
        content[i][0] = 0;
    }
}