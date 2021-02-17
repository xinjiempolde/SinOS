#ifndef TASK_H
#define TASK_H

#include <cpu/type.h>
#include <mm/memory.h>

#define MAX_TASK_NUM 100
#define TASK_NO_USE  0x0
#define TASK_IN_USE  0x1
/* see structure of TSS: https://wiki.osdev.org/Task_State_Segment */
typedef struct {
    uint32_t LINK, ESP0, SS0, ESP1, SS1, ESP2, SS2, CR3;
    uint32_t EIP, EFLAGS, EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI;
    uint32_t ES, CS, SS, DS, FS, GS, LDTR, IOPB;
} __attribute__((packed)) TSS;

typedef struct {
    int sel;
    int flags;
    TSS tss;
}Task;

typedef struct {
    int task_num;
    int current;
    Task task_array[MAX_TASK_NUM]; 
}TaskCtrl;

void task_test();
void console_task();

Task* task_init(void);
Task* task_alloc(void);

void task_switch(void);
extern void load_task_register(uint16_t sel);
extern void far_jmp(uint32_t EIP, uint16_t CS);
#endif