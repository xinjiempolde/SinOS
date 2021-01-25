#include <cpu/type.h>
/**
 * Read a byte from the specified port
 */
unsigned char port_byte_in(uint16_t port) {
    unsigned char result;
    // "a"(result): put e'a'x to result when asm finished
    // "d"(port): load e'd'x with port
    __asm__("in %%dx, %%al": "=a"(result) :"d"(port));
    return result;
}

/**
 * Write a byte to the specified port
 */
void port_byte_out(uint16_t port, unsigned char data) {
    __asm__("out %%al, %%dx": :"d"(port), "a"(data));
}