#include <kernel/task.h>
#include <cpu/gdt.h>
#include <gui/vga.h>
#include <mm/memory.h>
#include <gui/window.h>
#include <gui/layer.h>
#include <gui/console.h>
#include <cpu/timer.h>
#include <drivers/keyboard.h>
TSS tss_curr;
TSS tss_test;
extern LayerManager* layman;
extern char key_buff[256];
static MemMan* memMan;
static TaskCtrl* tCtrl;

Layer* console_layer;
void console_task() {
    int kb_idx = 0; // keyboard buffuer index
    timer_t* cursorTimer = set_timer(50);
    int cursorColor = WHITE;
    int cursorX = 12, cursorY = 24;
    uint8_t* console_buf = mem_alloc_4k(memMan, DFT_CSL_H*DFT_CSL_W);
    init_console_buf(console_buf, DFT_CSL_W, DFT_CSL_H);
    Layer* console_layer = alloc_layer(layman, console_buf, DFT_CSL_H, DFT_CSL_W, 6);
    add_layer(layman, console_layer);
    move_layer(layman, console_layer, 100, 100);
    while (1) {
        if (key_buff[kb_idx] != '\0') {
            if (key_buff[kb_idx] == KEY_BACKSPACE) {
                if (cursorX >= 4 + CHAR_W) { // can backspace
                    cursorX -= 8;
                    fill_rect(console_buf, DFT_CSL_W, cursorX, cursorY, 2*CHAR_W, CHAR_H, BLACK);
                    repaint_single_layer(layman, console_layer, cursorX, cursorY, 2*CHAR_W, CHAR_H);
                }
            } else if (key_buff[kb_idx] == KEY_ENTER) {
                fill_rect(console_buf, DFT_CSL_W, cursorX, cursorY, CHAR_W, CHAR_H, BLACK); // cover the char of the end of line
                repaint_single_layer(layman, console_layer, cursorX, cursorY, CHAR_W, CHAR_H);
                put_char(console_buf, DFT_CSL_W, 4, cursorY+CHAR_H, '#', BRIGHT_GREEN);
                repaint_single_layer(layman, console_layer, 4, cursorY+CHAR_H, CHAR_W, CHAR_H);
                cursorX = 12;
                cursorY += CHAR_H;
            } else {        
                fill_rect(console_buf, DFT_CSL_W, cursorX, cursorY, CHAR_W, CHAR_H, BLACK);
                put_char(console_buf, DFT_CSL_W, cursorX, cursorY, key_buff[kb_idx], WHITE);
                repaint_single_layer(layman, console_layer, cursorX, cursorY, CHAR_W, CHAR_H);
                cursorX += CHAR_W;
            }
            kb_idx++;
        }
        if (cursorTimer->timeoutFlags == TIME_OUT) {
            if (cursorColor == WHITE) {
                cursorColor = BLACK;
            } else {
                cursorColor = WHITE;
            }
            restart_timer(cursorTimer);
        }
        fill_rect(console_buf, DFT_CSL_W, cursorX, cursorY, 8, 16, cursorColor);
        repaint_single_layer(layman, console_layer, cursorX, cursorY, CHAR_W, CHAR_H);
    }
}
Task* task_init() {
    int i, taskSel;
    Task* task;

    memMan = (MemMan*)MEM_MAN_ADDR;
    tCtrl = mem_alloc_4k(memMan, sizeof(TaskCtrl));
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
        gdt_set_gate(3+i, &(task->tss), sizeof(TSS), 0x89, 0x40);
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
    task->tss.LDTR = 0;
    task->tss.IOPB = sizeof(TSS) << 16;

    return task;
}

void task_switch() {
    tCtrl->current = (tCtrl->current+1) % tCtrl->task_num;

    far_jmp((uint32_t)0, (uint16_t)tCtrl->task_array[tCtrl->current].sel);
}