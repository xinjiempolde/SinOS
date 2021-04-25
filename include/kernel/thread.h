#include <cpu/type.h>
typedef struct {
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edi;
    uint32_t esi;
    void (*eip)(void*, void*);
}ThreadStack;

typedef struct {

}PCB;