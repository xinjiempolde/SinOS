#ifndef TYPE_H
#define TYPE_H

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

typedef char* va_list;

typedef int bool;

#define NULL 0
#define TRUE 1
#define FALSE 0

#define CS_SEL 8
#define DS_SEL 16

#define EFLAGS_CF_BIT 0x00000001
#define EFLAGS_RE_BIT 0x00000002
#define EFLAGS_IF_BIT 0x00000200
#define EFLAGS_AC_BIT 0x00040000
#define low_16(address) (uint16_t)(address & 0xffff)
#define high_16(address) (uint16_t)((address >> 16) & 0xffff)
#endif