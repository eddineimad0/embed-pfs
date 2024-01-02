#ifndef INC_SYSTEM_H
#define INC_SYSTEM_H
#include<common.h>
void system_wake_up(void);
uint32_t system_get_ticks(void);
void system_delay(const uint32_t seconds);
#endif
