#ifndef WINDOW_H
#define WINDOW_H
#include <cpu/type.h>

#define DFT_WIN_TIT_H 24 // default window title height
void init_window_buf(uint8_t* buf, int weight, int height, char* title);

#endif