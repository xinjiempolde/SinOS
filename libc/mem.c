#include <libc/mem.h>
void memory_copy(char* src, char* dst, int nbytes) {
    int i;
    for (i = 0; i < nbytes; i++) {
        dst[i] = src[i];
    }
}

void memset(char* buf, int nbytes) {
    int i;
    for (i = 0; i < nbytes; i++) {
        buf[i] = 0;
    }
}
