#include <kernel/task.h>
#include <cpu/gdt.h>
#include <gui/vga.h>

TSS tss_curr;
TSS tss_test;

void task_test() {
    unsigned int i = 0;
    while(1) {
        i++;
        if (i == 4294967295) {
            printf("yes, it works!");
        }
    };
}

void task_switch() {
    /* current task status segment, it's necessary for switching task */
    tss_curr.LDTR = 0;
    tss_curr.IOPB = sizeof(TSS) << 16;

    tss_test.EIP = (uint32_t)&task_test;
    tss_test.EFLAGS = 0x202;
    tss_test.CS = 8;
    tss_test.DS = 16;
    tss_test.ES = 16;
    tss_test.SS = 16;

    tss_test.ESI = 0;
    tss_test.EDI = 0;
    tss_test.ESP = 0x2000000;
    tss_test.IOPB = sizeof(TSS) << 16;
    tss_test.LDTR = 0;

    gdt_set_gate(3, &tss_curr, sizeof(TSS), 0x89, 0x40);
    gdt_set_gate(4, &tss_test, sizeof(TSS), 0x89, 0x40);

    asm volatile("mov $0x18, %%ax\n\t"
                "ltr %%ax\n\t"
                "ljmp $0x20, $0x00"
                :
                :
                : "%ax" );

}