#ifndef INC_SYSTICK_H
#define INC_SYSTICK_H
#include<common.h>
void systick_setup(void);
uint32_t systick_get_ticks(void);
void systick_delay(const uint32_t miliseconds);
void systick_shutdown(void);
#endif
