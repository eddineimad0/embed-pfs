#include<libopencm3/stm32/rcc.h>

void system_wake_up(void){
    rcc_clock_setup_pll(&rcc_hsi_configs[RCC_CLOCK_HSI_64MHZ]);
}
