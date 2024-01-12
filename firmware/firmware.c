#include<common.h>
#include<uart.h>
#include<systick.h>
#include<libopencm3/stm32/rcc.h>
#include<libopencm3/stm32/gpio.h>

#define LED_PORT GPIOC
#define LED_PIN GPIO13

static void gpio_setup(void){
    rcc_periph_clock_enable(RCC_GPIOC);
    gpio_set_mode(LED_PORT, GPIO_MODE_OUTPUT_2_MHZ,GPIO_CNF_OUTPUT_PUSHPULL, LED_PIN);
}

static void firmware_init(){
    rcc_clock_setup_pll(&rcc_hsi_configs[RCC_CLOCK_HSI_64MHZ]);
    systick_setup();
    gpio_setup();
}

int main() {
    firmware_init();

    while(true){
        systick_delay(1000);
        gpio_toggle(LED_PORT,LED_PIN);
    }

    return 0;
}

