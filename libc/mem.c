#include <libc/mem.h>
void memory_copy(uint8_t* src, uint8_t* dst, int nbytes) {
    int i;
    for (i = 0; i < nbytes; i++) {
        dst[i] = src[i];
    }
}

void memset(uint8_t* buf, int nbytes) {
    int i;
    for (i = 0; i < nbytes; i++) {
        buf[i] = 0;
    }
}

/* 把二维数组中的内容转换为线性的一维数组 */
void transfer(char* content, int col_size, int line_num, char* buf) {
    int i, offset = 0;
    char* p;
    for (i = 0; i < line_num; i++) {
        p = content + i*col_size;
        strcp(p, buf + offset, strlen(p));
        offset += strlen(p);
    }
}