#include <drivers/vga.h>
#include <drivers/ports.h>
#include <drivers/font16.h>
#include <libc/mem.h>
#include <libc/string.h>
void set_palette(int start, int end, uint8_t* rgb);
void put_pixel(int row, int col, uint8_t color);
void fill_rect(int x0, int y0, int w, int h, uint8_t color);


void init_palette() {
    static uint8_t table_rgb[16 * 3] = {
        0x00, 0x00, 0x00,       /* 0:黑 */
        0xff, 0x00, 0x00,       /* 1:亮红 */
        0x00, 0xff, 0x00,       /* 2:亮绿 */
        0xff, 0xff, 0x00,       /* 3:亮黄 */
        0x00, 0x00, 0xff,       /* 4:亮蓝 */
        0xff, 0x00, 0xff,       /* 5:亮紫 */
        0x00, 0xff, 0xff,       /* 6:浅亮蓝 */
        0xff, 0xff, 0xff,       /* 7:白 */
        0xd6, 0xd6, 0xd6,       /* 8:亮灰 */
        0x84, 0x00, 0x66,       /* 9:暗红 */
        0x00, 0x84, 0x00,       /* 10:暗绿 */
        0x84, 0x84, 0x00,       /* 11:暗黄 */
        0x00, 0x00, 0x84,       /* 12:暗蓝 */
        0x84, 0x00, 0x84,       /* 13:暗紫 */
        0x00, 0x84, 0x84,       /* 14:浅暗蓝 */
        0xa4, 0xa4, 0xa4        /* 15:暗灰 */
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

void fill_rect(int x0, int y0, int w, int h, uint8_t color) {
    uint8_t* vram = (uint8_t*)0xa0000;
    int x, y;
    for (y = y0; y < y0 + h; y++) {
        for (x = x0; x < x0 + w; x++) {
            vram[x + y * SCREEN_W] = color;
        }
    }
}

void put_pixel(int x, int y, uint8_t color) {
    char* vram = (char*)0xa0000;
    vram[x + SCREEN_W * y] = color;
}


void put_char(int x0, int y0, char c, uint8_t color) {
    int i, j;
    for (i = 0; i < CHAR_H; i++) {
        uint8_t mask = 0x01;
        for (j = 0; j < CHAR_W; j++) {
            // this postion has pixel
            if ((asc1608[(uint8_t)c][i] & mask) != 0) {
                put_pixel(x0+j, y0+i, color);
            }
            mask = mask << 1;
        }
    }
}

void put_string(int x0, int y0, char* str, uint8_t color) {
    int i = 0;
    int x = x0, y = y0;
    while (str[i] != '\0') {
        put_char(x, y, str[i], color);
        x += CHAR_W;
        if (x >= SCREEN_W) {
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
    put_string(0, 0, buf, BLACK);
}

void vsprintf(char* buf, const char* fmt, va_list args) {
    va_list p_next_arg = args;
    int i_buf = 0,  i_fmt = 0;
    int n_digits = 0; // number of digits
    char tmp[11]; // for decimal number, unsigned int ranges 0～4294967295, the last bit for '\0'
                  // for hex number, 0x12345678, 11 bits, the last bit for '\0'
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
            default:
                break;
        }
    }
    buf[i_buf] = '\0';
}