#include <mm/memory.h>
#include <cpu/register.h>
#include <cpu/type.h>

#include <gui/vga.h>
#include <libc/string.h>

uint32_t mem_check(uint32_t start, uint32_t end) {
    uint32_t eflags = load_eflags();
    uint32_t memtotal = 0;
    bool flg486 = TRUE;

    /* set AC(18) bit of EFLAGS, then reread it.
     * if AC bit of EFLAGS is still 0, it's 80386 or before
     * if AC bit of EFLAGS is 1 now, it's 80486 or later.
     * It hasn't cache until 80486, so we should disable cache if it's 80486 or later
     */
    eflags |= EFLAGS_AC_BIT;
    store_eflags(eflags);
    eflags = load_eflags();

    /* AC bit of EFLAGS is still 0, it's 80386 */
    if ((eflags & EFLAGS_AC_BIT) == 0) {
        flg486 = FALSE;
    }

    /* if 486, diable cache temporarily*/
    if (flg486) {
        disable_cache();
    }

    memtotal = mem_check_sub(start, end);

    if (flg486) {
        enable_cache();
    }
    return memtotal;

}

/**
 * memory check subroutine
 * write 0x55 to memory and then invert, check whether the result is 0xaa
 * check 4KB each time(only write to last byte of 4KB to check)
 */
uint32_t mem_check_sub(uint32_t start, uint32_t end) {
    uint8_t t1 = 0x55, t2 = 0xaa;
    uint8_t old, *p;
    uint32_t i;
    // check 4KB each time
    for (i = start; i < end; i=i+0x1000) {
        p = (uint8_t*)(i + 0x1000-1); // check the last byte of 4KB
        old = *p;
        *p = t1;
        *p ^= 0xff;
        if (*p != t2) {
            *p = old;
            return i;
        }

        *p ^= 0xff;
        if (*p != t1) {
            *p = old;
            return i;
        }

        *p = old;
    }
    return i;
}

/**
 * init memory manager
 */
void mem_init(MemMan* memman) {
    memman->cnt = 0;
}

/**
 * calculate total free memory(bytes)
 */
uint32_t mem_total(MemMan* memman) {
    uint32_t total = 0;
    int i;
    for (i = 0; i < memman->cnt; i++) {
        total += memman->frees[i].size;
    }
    return total;
}

/**
 * allocate size bytes memory and return the starting address of allocated memory if success.
 * return 0 if no satisfied free memory
 */
uint32_t mem_allocate(MemMan* memman, uint32_t size) {
    int i;
    uint32_t begin_addr;
    for (i = 0; i < memman->cnt; i++) {
        if (memman->frees[i].size >= size) {
            /* find enough free memory */
            begin_addr = memman->frees[i].addr;
            memman->frees[i].addr += size;
            memman->frees[i].size -= size;

            if (memman->frees[i].size == 0) {
                memman->cnt--;
            }
            return begin_addr;  // return the starting address of allocated memory
        }
    }
    return 0; // no satisfied memory space
}

bool mem_free(MemMan* memman, uint32_t addr, uint32_t size) {
    int i, j;
    for (i = 0; i < memman->cnt; i++) {
        if (memman->frees[i].addr >=  addr) break; // get insert point
    }
    /* if there is free memory */
    if (i > 0) {
        /* can merge with the previous */
        if (memman->frees[i-1].addr + memman->frees[i-1].size >= addr) {
            memman->frees[i-1].size = (addr - memman->frees[i-1].addr) + size;
            if (i < memman->cnt) {
                /* can merge withe the next */
                if (addr + size >= memman->frees[i].addr) {
                    memman->frees[i-1].size = (memman->frees[i].addr + memman->frees[i].size)
                                                - memman->frees[i-1].addr;
                    /* delete the next one, and move left one by one */
                    memman->cnt--;
                    for (j = i; j < memman->cnt - 1; j++) {
                        memman->frees[j] = memman->frees[j+1];
                    }

                    return TRUE; // success
                }
            }
        }
    }

    /* can't merge with the previous, but can merge with the next */
    if (i < memman->cnt) {
        if (addr + size >= memman->frees[i].addr) {
            memman->frees[i].size += memman->frees[i].addr - addr;
            memman->frees[i].addr = addr;
            return TRUE; // success
        }
    }

    /* it's the separate one, should insert into array */
    if (memman->cnt >= FREE_MEM_ITEM_CNT) { // there is no space to store free memory info
        return FALSE; // fail
    } else {
        // move right one by one
        for (j = memman->cnt; j > i; j--) {
            memman->frees[j] = memman->frees[j-1];
        }
        /* insert */
        memman->cnt++;
        memman->frees[i].addr = addr;
        memman->frees[i].size = size;
        return TRUE;
    }
}

uint32_t mem_alloc_4k(MemMan* memman, uint32_t size) {
    uint32_t a;
    size = (size + 0xfff) & 0xfffff000;  // round up by 0x1000(4K)
    a =  mem_allocate(memman, size);
    return a;
}


bool mem_free_4k(MemMan* memman, uint32_t addr, uint32_t size) {
    uint32_t a;
    size = (size + 0xfff) & 0xfffff000;  // round up by 0x1000(4K)
    a = mem_free(memman, addr, size);
    return a;
}