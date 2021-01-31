#ifndef VGA_H
#define VGA_H

#include <cpu/type.h>

#define BOOTINFO_ADDR 0xff00
#define BOOT_BY_VGA 0x00
#define BOOT_BY_VBE 0x01
#define NO_RESET_PALETTE 0x00
#define RESET_PALETTE    0x01

#define CHAR_W 8
#define CHAR_H 16

#define PORT_PALATTE_INDEX 0x03c8
#define PORT_PALATTE_RGB 0x03c9

/* palette index */
#define BLACK 0
#define BRIGHT_RED 1
#define BRIGHT_GREEN 2
#define BRIGHT_YELLOW 3
#define BRIGHT_BLUE 4
#define BRIGHT_PURPLE 5
#define LIGHT_BRIGHT_BLUE 6
#define WHITE 7
#define BRIGHT_GRAY 8
#define DARK_RED 9
#define DARK_GREEN 10
#define DARK_YELLOW 11
#define DARK_BLUE 12
#define DARK_PURPLE 13
#define LIGHT_DARK_BLUE 14
#define DARK_GRAY 15
#define TRANSPARENT 99

#define SLIDE_BAR_H 28

/* boot infomation, get from assembly */
typedef struct {
    short screen_w;
    short screen_h;
    uint8_t* vram;
    char VGA_OR_VBE;
    char reset_palette;
}BOOTINFO;

void init_palette();

void fill_rect(uint8_t*buf, int buf_w, int x0, int y0, int w, int h, uint8_t color);
void put_char(uint8_t* buf, int buf_w, int x0, int y0, char c, uint8_t color);
void put_string(uint8_t* buf, int buf_w, int x0, int y0, char* str, uint8_t color);

void printf(const char* fmt, ...);
void sprintf(char* buf, const char* fmp, ...);
void vsprintf(char* buf, const char *fmp, va_list args);

#endif