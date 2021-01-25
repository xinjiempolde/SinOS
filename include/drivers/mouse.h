#ifndef MOUSE_H
#define MOUSE_H

#define MOUSE_DATA_PORT 0x60
#define MOUSE_CMD_PORT 0x64
#define MOUSE_STATE_PORT 0x64

#define MOUSE_ENABLE 0xf4
#define MOUSE_SENDTO 0xd4

void init_mouse();
void init_mouse_cursor();
void put_cursor(int x0, int y0);
#endif