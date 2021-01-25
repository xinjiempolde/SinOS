#include <mm/memory.h>
#include <cpu/register.h>
#include <cpu/type.h>

#include <drivers/vga.h>
#include <libc/string.h>
uint32_t memcheck(uint32_t start, uint32_t end) {
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
    
    memtotal = memcheck_sub(start, end) / (1024 * 1024);
    printf("mem:%dMB", memtotal);

}

uint32_t memcheck_sub(uint32_t start, uint32_t end) {
    uint8_t t1 = 0x55, t2 = 0xaa;
    uint8_t old;
    uint8_t* i;
    // check 4KB each time
    for (i = start; i < end; i=i+0x1000) {
        old = *(i + 0x1000-1); // check the last byte of 4KB
        *i = t1;
        *i ^= 0xff;
        if (*i != t2) {
            *i = old;
            return i;
        }

        *i ^= 0xff;
        if (*i != t1) {
            *i = old;
            return i;
        }

        *i = old;
    }
    return i;
}