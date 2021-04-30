#include <kernel/task.h>
#include <cpu/gdt.h>
#include <gui/vga.h>
#include <mm/memory.h>
#include <gui/window.h>
#include <gui/layer.h>
#include <gui/console.h>
#include <cpu/timer.h>
#include <drivers/keyboard.h>
#include <libc/string.h>
#include <drivers/ata.h>
#include <kernel/pci.h>
#include <fs/fs.h>
#include <libc/mem.h>
#include <fs/inode.h>

TSS tss_curr;
TSS tss_test;
extern LayerManager* layman;
static MemMan* memMan;
static TaskCtrl* tCtrl;


void console_task() {    
    console_run();
}

Task* task_init() {
    int i, taskSel;
    Task* task;

    memMan = (MemMan*)MEM_MAN_ADDR;
    tCtrl = (TaskCtrl*)mem_alloc_4k(memMan, sizeof(TaskCtrl));
    tCtrl->task_num = 0;
    for (i = 0; i < MAX_TASK_NUM; i++) {
        task = &(tCtrl->task_array[i]);
        task->sel = (3 + i) * 8; // the first task corresponds to the third GDT entry
        task->flags = TASK_NO_USE;
        /**
         * specifying the TSS' address as "base", 
         * TSS' size as "limit", 0x89 (Present|Executable|Accessed) as "access byte"
         * and 0x40 (Size-bit) as "flags"
         */
        gdt_set_gate(3+i, (uint32_t)&(task->tss), sizeof(TSS), 0x89, 0x40);
    }
    task = task_alloc();
    tCtrl->current = 0;
    taskSel = tCtrl->task_array[tCtrl->current].sel;
    load_task_register(taskSel);
    return task;
}

Task* task_alloc(void) {
    int i;
    Task* task = NULL;

    /* find an unused task */
    for (i = 0; i < MAX_TASK_NUM; i++) {
        if (tCtrl->task_array[i].flags == TASK_NO_USE) {
            task = &(tCtrl->task_array[i]);
            break;
        }
    }

    tCtrl->task_num++;

    task->flags = TASK_IN_USE;
    /* set the properties of TSS */
    task->tss.EFLAGS = EFLAGS_IF_BIT | EFLAGS_RE_BIT;
    task->tss.EAX = 0;
    task->tss.ECX = 0;
    task->tss.EDX = 0;
    task->tss.EBX = 0;
    task->tss.EBP = 0;
    task->tss.ESI = 0;
    task->tss.EDI = 0;
    
    task->tss.ES = DS_SEL;
    task->tss.CS = CS_SEL;
    task->tss.SS = DS_SEL;
    task->tss.DS = DS_SEL;
    task->tss.FS = DS_SEL;
    task->tss.GS = DS_SEL;
    task->tss.LDTR = 0;
    task->tss.IOPB = sizeof(TSS) << 16;

    return task;
}

void task_switch() {
    tCtrl->current = (tCtrl->current+1) % tCtrl->task_num;

    far_jmp((uint32_t)0, (uint16_t)tCtrl->task_array[tCtrl->current].sel);
}

