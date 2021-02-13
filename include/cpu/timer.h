#ifndef TIMER_H
#define TIMER_H

#define MAX_TIMER_COUNT 256
#define TIMER_IN_USE 0x01
#define TIMER_NO_USE 0x00

#define TIME_OUT 0x01
#define TIME_NO_OUT 0x00

#ifndef NULL
#define NULL 0
#endif

/* single timer structure */
typedef struct timer_t{
    unsigned int count;  // set by set_timer
    unsigned int systimePlusCount; // systime at the point of setting time add count;
    struct timer_t *nextNode;    // double   linked   lists
    struct timer_t *preNode;
    char useFlags;          // use or not
    char timeoutFlags;      // is time out?
}timer_t;

/* timer manager */
typedef struct {
    unsigned int sysTimeCount; // total time count from the beginning
    unsigned int timerNum;     // the number of timer
    timer_t* next;             // the latest timer which will time out
    timer_t timerArray[MAX_TIMER_COUNT];
}TIMER_MAN;


void init_timer();

timer_t* set_timer(unsigned int timeout);
void restart_timer(timer_t* timer);
#endif