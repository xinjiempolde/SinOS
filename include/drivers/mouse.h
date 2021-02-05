#ifndef MOUSE_H
#define MOUSE_H

#include <cpu/type.h>

#define MOUSE_DATA_PORT 0x60
#define MOUSE_CMD_PORT 0x64
#define MOUSE_STATE_PORT 0x64

#define MOUSE_ENABLE 0xf4
#define MOUSE_SENDTO 0xd4

#define LEFT_BTN_ON 0x01
#define RIGHT_BTN_ON 0x02
#define MIDDLE_BTN_ON 0x04
void init_mouse();
void init_mouse_cursor(uint8_t* mouse_cursor_buff);
void put_cursor(int x0, int y0);
#endif