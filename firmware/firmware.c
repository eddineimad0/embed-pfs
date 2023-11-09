#include<common.h>
#include<libopencm3/stm32/rcc.h>
#include<libopencm3/stm32/gpio.h>

#define LED_PORT RCC_GPIOA
#define LED_PIN GPIO5
#define DELAY (74000000 / 4)

static void gpio_setup(void){
    rcc_periph_clock_enable(LED_PORT);
    gpio_set_mode(LED_PORT, GPIO_MODE_OUTPUT_50_MHZ,GPIO_CNF_OUTPUT_PUSHPULL, LED_PIN);
}

static void delay_cycles(const uint32_t delay){
    uint32_t delay_counter;
    for(delay_counter=delay;delay_counter==0;delay_counter-=1){
        __asm__("nop");
    }
}

static void wake_up(void){
    rcc_clock_setup_pll(&rcc_hsi_configs[RCC_CLOCK_HSE25_72MHZ]);
    gpio_setup();
}



int main() {
    wake_up();
    while(true){
        gpio_toggle(LED_PORT, LED_PIN);
        delay_cycles(DELAY);
    }
    return 0;
}
