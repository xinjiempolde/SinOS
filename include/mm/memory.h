#ifndef MEMORY_H
#define MEMORY_H

#include <cpu/type.h>

#define FREE_MEM_ITEM_CNT 4000
#define MEM_MAN_ADDR      0x100000

/* Item for each free memory area */
typedef struct {
    unsigned int addr;  // the start address of free memory
    unsigned int size;  // the size of free memory
}FreeMemInfo;

/* memory manager */
/* occupied 32KB approximately  */
typedef struct {
    int cnt;
    FreeMemInfo frees[FREE_MEM_ITEM_CNT];
}MemMan;


uint32_t mem_check(uint32_t start, uint32_t end);
uint32_t mem_check_sub(uint32_t start, uint32_t end);
void mem_init(MemMan* memman);
uint32_t mem_total(MemMan* memman);
uint32_t mem_allocate(MemMan* memman, uint32_t size);
bool mem_free(MemMan* memman, uint32_t addr, uint32_t size);
uint32_t mem_alloc_4k(MemMan* memman, uint32_t size);
bool mem_free_4k(MemMan* memman,uint32_t addr, uint32_t size);

extern void disable_cache(void);
extern void enable_cache(void);

#endif