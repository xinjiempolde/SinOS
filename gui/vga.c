#include <gui/vga.h>
#include <drivers/ports.h>
#include <drivers/font16.h>
#include <libc/mem.h>
#include <libc/string.h>
static BOOTINFO* bootInfo = (BOOTINFO*)BOOTINFO_ADDR; 

void set_palette(int start, int end, uint8_t* rgb);
void put_pixel(int row, int col, uint8_t color);


static int print_cursor_x = 0;
static int print_cursor_y = 0;
void init_palette() {
    /* see links: https://en.wikipedia.org/wiki/Web_colors */
    static uint8_t table_rgb[16 * 3] = {
        0x00, 0x00, 0x00,       /* 0:黑 */
        0xff, 0x00, 0x00,       /* 1:亮红 */
        0x00, 0xff, 0x00,       /* 2:亮绿 */
        0xff, 0xff, 0x00,       /* 3:亮黄 */
        0x00, 0x00, 0xff,       /* 4:亮蓝 */
        0xff, 0x00, 0xff,       /* 5:亮紫 */
        0x00, 0xff, 0xff,       /* 6:浅亮蓝 */
        0xff, 0xff, 0xff,       /* 7:白 */
        0xd3, 0xd3, 0xd3,       /* 8:亮灰 */
        0x84, 0x00, 0x66,       /* 9:暗红 */
        0x00, 0x84, 0x00,       /* 10:暗绿 */
        0x84, 0x84, 0x00,       /* 11:暗黄 */
        0x00, 0x00, 0x8b,       /* 12:暗蓝 */
        0x84, 0x00, 0x84,       /* 13:暗紫 */
        0x87, 0xce, 0xeb,       /* 14:浅暗蓝 */
        0xa9, 0xa9, 0xa9        /* 15:暗灰 */
    };
    set_palette(0, 15, table_rgb);
}

void set_palette(int start, int end, uint8_t* rgb) {
    if (start > end || start < 0 || end < 0) return; // prevent error

    int i;
    port_byte_out(PORT_PALATTE_INDEX, start);
    for (i = start; i <= end; i++) {
        port_byte_out(PORT_PALATTE_RGB, rgb[3*i]);
        port_byte_out(PORT_PALATTE_RGB, rgb[3*i+1]);
        port_byte_out(PORT_PALATTE_RGB, rgb[3*i+2]);
    }
}

void fill_rect(uint8_t* buf, int buf_w, int x0, int y0, int w, int h, uint8_t color) {
    int x, y;
    for (y = y0; y < y0 + h; y++) {
        for (x = x0; x < x0 + w; x++) {
            buf[x + y * buf_w] = color;
        }
    }
}

void put_pixel(int x, int y, uint8_t color) {
    char* vram = (char*)0xa0000;
    vram[x + bootInfo->screen_w * y] = color;
}


void put_char(uint8_t* buf, int buf_w, int x0, int y0, char c, uint8_t color) {
    int i, j;
    for (i = 0; i < CHAR_H; i++) {
        uint8_t mask = 0x01;
        for (j = 0; j < CHAR_W; j++) {
            // this postion has pixel
            if ((asc1608[(uint8_t)c][i] & mask) != 0) {
                buf[(y0+i)*buf_w + (x0+j)] = color;
            }
            mask = mask << 1;
        }
    }
}

void put_string(uint8_t* buf, int buf_w, int x0, int y0, char* str, uint8_t color) {
    int i = 0;
    int x = x0, y = y0;
    while (str[i] != '\0') {
        put_char(buf, buf_w, x, y, str[i], color);
        x += CHAR_W;
        if (x >= buf_w) {
            x = 0;
            y += CHAR_H;
        }
        i++;
    }
}

void printf(const char* fmt, ...) {
    char buf[64];
    /**
     * fmt is value's address in the memory
     * &fmt is the variable 'fmt''s address in the memory
     * *fmt is the firt charactor 
     */
    va_list args = (va_list)((char*)(&fmt) + 4);     // get the next parameter from stack, see c call conventions
    vsprintf(buf, fmt, args);
    put_string((uint8_t*)bootInfo->vram, bootInfo->screen_w, print_cursor_x, print_cursor_y, buf, BRIGHT_RED);
    print_cursor_x = 0;
    print_cursor_y += CHAR_H;
}

void sprintf(char* buf, const char* fmt, ...) {
    va_list args = (va_list)((char*)(&fmt) + 4);
    vsprintf(buf, fmt, args);
}

void vsprintf(char* buf, const char* fmt, va_list args) {
    va_list p_next_arg = args;
    int i_buf = 0,  i_fmt = 0;
    int n_digits = 0; // number of digits
    char tmp[11]; // for decimal number, unsigned int ranges 0～4294967295, the last bit for '\0'
                  // for hex number, 0x12345678, 11 bits, the last bit for '\0'
    char* string = NULL;
    for (i_fmt = 0; fmt[i_fmt] != '\0'; i_fmt++) {
        if (fmt[i_fmt] != '%') {
            buf[i_buf++] = fmt[i_fmt];
            continue;
        }
        i_fmt++;
        switch (fmt[i_fmt]) {
            case 'x':
                n_digits = uint_to_hex(*(unsigned int*)p_next_arg, tmp);
                memory_copy(tmp, buf + i_buf, n_digits);
                i_buf += n_digits;
                p_next_arg += 4;
                break;
            case 'd':
                n_digits = uint_to_dec(*(unsigned int*)p_next_arg, tmp);
                memory_copy(tmp, buf + i_buf, n_digits);
                i_buf += n_digits;
                p_next_arg += 4; // every parameter occuped 4 bytes in stack
                break;
            case 's':
                // p_next_arg保存的是字符串的地址的地址，*(char*)p_next_arg是字符串的地址
                string = (*(uint32_t*)p_next_arg);
                memory_copy(string, buf + i_buf, strlen(string)); 
                i_buf += strlen(string);
                p_next_arg += 4;
                break;
            default:
                break;
        }
    }
    buf[i_buf] = '\0';
}