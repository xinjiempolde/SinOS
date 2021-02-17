#ifndef REGISTER_H
#define REGISTER_H

#include <cpu/type.h>


#define CRO_CD_BIT 0x40000000
#define CR0_NW_BIT 0x20000000
uint32_t load_eflags(void);
void store_eflags(uint32_t eflags);
uint32_t load_cr0(void);
void store_cr0(uint32_t cr0);

#endif