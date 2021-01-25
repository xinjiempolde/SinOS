#ifndef TYPE_H
#define TYPE_H

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

typedef char* va_list;

typedef int bool;
#define TRUE 1
#define FALSE 0

#define low_16(address) (uint16_t)(address & 0xffff)
#define high_16(address) (uint16_t)((address >> 16) & 0xffff)
#endif