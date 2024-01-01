#include<common.h>
#include<bootloader.h>
#include<uart.h>
#include<libopencm3/stm32/rcc.h>
#include<libopencm3/stm32/gpio.h>
#include<libopencm3/cm3/systick.h>
#include<libopencm3/cm3/scb.h>

#define LED_PORT GPIOC
#define LED_PIN GPIO13
#define DELAY (64000000/4)

static void vector_setup(void){
    SCB_VTOR = BOOTLOADER_SIZE;
}

static void systick_setup(void){
    systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8); // 48Mhz / 8 = 6Mhz
    systick_set_reload(rcc_ahb_frequency/8 - 1); // 6Mhz - 1 = 5999999 ticks
    systick_interrupt_enable();
    systick_counter_enable();
}

static void gpio_setup(void){
    rcc_periph_clock_enable(RCC_GPIOC);
    gpio_set_mode(LED_PORT, GPIO_MODE_OUTPUT_2_MHZ,GPIO_CNF_OUTPUT_PUSHPULL, LED_PIN);
}

static void wake_up(void){
    vector_setup();
    rcc_clock_setup_pll(&rcc_hsi_configs[RCC_CLOCK_HSI_48MHZ]);
    systick_setup();
    gpio_setup();
}


int main() {
    wake_up();
    while(true){
        if(uart_data_available()){
            uint8_t data = uart_read_byte();
            uart_write_byte(data + 1);
        }
    }
    return 0;
}

void sys_tick_handler(void){
    // switch gpio state every second.
    gpio_toggle(LED_PORT, LED_PIN);
}
