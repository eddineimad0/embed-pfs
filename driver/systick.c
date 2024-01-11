#include<libopencm3/cm3/systick.h>
#include<libopencm3/stm32/rcc.h>

/// keeps count of the number of miliseconds that passed
/// overflows after approx 1000 hours.
static volatile uint32_t ticks = 0;

void systick_setup(void){
    systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8); // 64MHZ / 8 = 8Mhz
    systick_set_reload(rcc_ahb_frequency/8000 - 1); // 8000 - 1 = 7999 ticks
    systick_counter_enable();
    systick_interrupt_enable();
}

void systick_shutdown(void){
    systick_interrupt_disable();
    systick_counter_disable();
    systick_clear();
}

void sys_tick_handler(void){
    ticks += 1;
}

uint32_t systick_get_ticks(void){
    return ticks;
}

void systick_delay(const uint32_t miliseconds){
    const uint32_t end_ticks = systick_get_ticks() + miliseconds;
    while(systick_get_ticks() < end_ticks){
        __asm__("nop");
    }
}
