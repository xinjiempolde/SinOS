#ifndef CONSOLE_H
#define CONSOLE_H

#define DFT_CSL_W 400 // default console weight
#define DFT_CSL_H 200 // default console height

#include <cpu/type.h>
#include <gui/window.h>

void init_console_buf(uint8_t* buf, int weight, int height);

#endif
