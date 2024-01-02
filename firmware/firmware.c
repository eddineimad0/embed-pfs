#include<common.h>
#include<uart.h>
#include<libopencm3/stm32/rcc.h>
#include<libopencm3/stm32/gpio.h>
#include"system.h"

#define LED_PORT GPIOC
#define LED_PIN GPIO13



static void gpio_setup(void){
    rcc_periph_clock_enable(RCC_GPIOC);
    gpio_set_mode(LED_PORT, GPIO_MODE_OUTPUT_2_MHZ,GPIO_CNF_OUTPUT_PUSHPULL, LED_PIN);
}


int main() {
    system_wake_up();
    uart_setup();
    gpio_setup();

    while(true){
        if(uart_data_available()){
            uart_write_byte(uart_read_byte());
        }
    }
    return 0;
}

