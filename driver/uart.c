#include<uart.h>
#include<libopencm3/stm32/rcc.h>
#include<libopencm3/stm32/usart.h>
#include<libopencm3/cm3/nvic.h>
#include<libopencm3/stm32/gpio.h>

#define UART_PORT (GPIOA)
#define UART_TX_PIN (GPIO_USART2_TX)
#define UART_RX_PIN (GPIO_USART2_RX)

static uint8_t byte_received = 0U;
static bool data_available = false;

void usart2_isr(void){
    const bool overrun_occured = usart_get_flag(USART2,USART_FLAG_ORE) == 1;
    const bool data_received = usart_get_flag(USART2,USART_FLAG_RXNE) == 1;

    if(data_received || overrun_occured){
        byte_received = (uint8_t)usart_recv(USART2);
        data_available = true;
    }
}

void uart_setup(void){
    rcc_periph_clock_enable(RCC_GPIOA);
    gpio_set_mode(UART_PORT, GPIO_MODE_OUTPUT_2_MHZ,GPIO_CNF_OUTPUT_PUSHPULL, UART_RX_PIN | UART_TX_PIN);
    gpio_primary_remap(AFIO_MAPR_SWJ_CFG_FULL_SWJ,AFIO_MAPR_USART2_REMAP);

    rcc_periph_clock_enable(RCC_USART2);

    usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE);
    usart_set_databits(USART2, 8);
    usart_set_baudrate(USART2, BAUD_RATE);
    usart_set_parity(USART2, 0);
    usart_set_stopbits(USART2, 1);
    usart_set_mode(USART2, USART_MODE_TX_RX);
    usart_enable_rx_interrupt(USART2);
    nvic_enable_irq(NVIC_USART2_IRQ);

    usart_enable(USART2);
}

void uart_write_byte(uint8_t byte){
    usart_send_blocking(USART2,(uint16_t)byte);
}

void uart_write_buffer(uint8_t* data,uint32_t size){
    for(uint32_t i = 0; i<size; i+=1){
        uart_write_byte(data[i]);
    }
}

uint8_t uart_read_byte(void){
    data_available = false;
    return byte_received;
}

uint32_t uart_read_buffer(uint8_t* buffer,const uint32_t size){
    if(size > 0 && data_available){
        *buffer = byte_received;
        data_available = false;
        return 1;
    }
    return 0;
}

bool uart_data_available(void){
    return data_available;
}
