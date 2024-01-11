#ifndef INC_TIMER_H
#define INC_TIMER_H

#include<common.h>

typedef struct timer_t{
    uint32_t wait_time;
    uint32_t end_time;
    bool auto_reset;
    bool elapsed;
}Timer;

void Timer_setup(Timer* t,uint32_t wait_time,bool auto_reset);
void Timer_reset(Timer* t);
bool Timer_has_elapsed(Timer* t);

#endif // !INC_TIMER_H
