#ifndef TASK_H
#define TASK_H

#include <cpu/type.h>
/* see structure of TSS: https://wiki.osdev.org/Task_State_Segment */
typedef struct {
    uint32_t LINK, ESP0, SS0, ESP1, SS1, ESP2, SS2, CR3;
    uint32_t EIP, EFLAGS, EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI;
    uint32_t ES, CS, SS, DS, FS, GS, LDTR, IOPB;
} __attribute__((packed)) TSS;

void task_test();

void task_switch();

extern void jmp_to_new_task(void);
#endif