#ifndef GEDIT_H
#define GEDIT_H

#include <gui/window.h>
#include <gui/layer.h>

#define DFT_EDIT_BOR   4   // default console border
#define DFT_EDIT_TIT_H DFT_WIN_TIT_H  // default console title height
#define DFT_EDIT_CON_W 60  // default console content weight
#define DFT_EDIT_CON_H 20  // default console content height
#define EDIT_CON_H_SIZE (DFT_EDIT_CON_W*CHAR_W) // console content horizontal size
#define EDIT_CON_V_SIZE (DFT_EDIT_CON_H*CHAR_H) // console content vertical size

#define DFT_EDIT_W     \
    (2 * DFT_EDIT_BOR + DFT_EDIT_CON_W * CHAR_W) // default console weight
#define DFT_EDIT_H     \
    (DFT_EDIT_TIT_H + DFT_EDIT_CON_H * CHAR_H + DFT_EDIT_BOR) // default console height

#ifdef ASCII_NL
#define ASCII_NL 0x0a
#endif

void init_gedit_buf(uint8_t* buf, int weight, int height);
int gedit_run();
int edit_newline(Layer* layer, int cursorY);
void edit_backspace();
void edit_enter();
int get_line_num(int cursorY);

#endif