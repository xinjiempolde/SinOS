#ifndef MEM_H
#define MEM_H
#include <cpu/type.h>
#include <libc/string.h>

void memory_copy(uint8_t *src, uint8_t *dst, int nbytes);
void memset(uint8_t* buf, int nbytes);
void transfer(char* content, int col_size, int line_num, char* buf);
#endif