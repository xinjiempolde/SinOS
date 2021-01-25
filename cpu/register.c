#include <cpu/register.h>
#include <cpu/type.h>

/**
 * load eflags from cpu and return eflags
 */
uint32_t load_eflags(void) {
    uint32_t eflags = 0;
    asm volatile ("pushf");
    asm volatile ("pop %%ebx": "=b"(eflags) :);
    return eflags;
}

/**
 * store eflags into cpu
 */
void store_eflags(uint32_t eflags) {
    asm volatile ("pushl %%ebx" : : "b"(eflags)); // mov ebx, eflags; push ebx;
    asm volatile ("popf");
}

uint32_t load_cr0(void) {
    uint32_t cr0;
    asm volatile ("movl %%cr0, %%eax": "=a"(cr0) :);
    return cr0;
}

void store_cr0(uint32_t cr0) {
    asm volatile ("movl %0, %%cr0":: "r"(cr0));
}