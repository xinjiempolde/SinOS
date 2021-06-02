#ifndef CONSOLE_H
#define CONSOLE_H

#include <cpu/type.h>
#include <gui/window.h>
#include <gui/layer.h>

#define DFT_CSL_BOR   4   // default console border
#define DFT_CSL_TIT_H DFT_WIN_TIT_H  // default console title height
#define DFT_CSL_CON_W 60  // default console content weight
#define DFT_CSL_CON_H 20  // default console content height
#define CSL_CON_H_SIZE (DFT_CSL_CON_W*CHAR_W) // console content horizontal size
#define CSL_CON_V_SIZE (DFT_CSL_CON_H*CHAR_H) // console content vertical size

#define DFT_CSL_W     \
    (2 * DFT_CSL_BOR + DFT_CSL_CON_W * CHAR_W) // default console weight
#define DFT_CSL_H     \
    (DFT_CSL_TIT_H + DFT_CSL_CON_H * CHAR_H + DFT_CSL_BOR) // default console height


void init_console_buf(uint8_t* buf, int weight, int height);
int console_newline(Layer* layer, int cursorY);
void console_clear(Layer* console_layer);

void console_run();
void parse_cmd(char* cmd_str);
void console_printfn(const char* fmt, ...);
void console_print(int color, const char* fmt, ...);

void gen_path(char* path);
int len_argv(char** argv);
void clear_argv(char** argv);

void cmd_gedit(char** argv);
void cmd_ls(char** argv);
void cmd_cat(char** argv);
void cmd_mv(char** argv);
#endif
