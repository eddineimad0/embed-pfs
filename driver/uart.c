#include<uart.h>
#include<libopencm3/stm32/rcc.h>
#include<libopencm3/stm32/usart.h>
#include<libopencm3/cm3/nvic.h>
#include<libopencm3/stm32/gpio.h>
#include<ring-buffer.h>

#define UART_PORT (GPIOA)
#define UART_ID (USART2)
#define UART_TX_PIN (GPIO_USART2_TX)
#define UART_RX_PIN (GPIO_USART2_RX)


static RingBuffer rb = {0U};
static uint8_t data_buffer[UART_RING_BUFFER_SIZE] = {0U};


void usart2_isr(void){
    const bool data_received = usart_get_flag(UART_ID,USART_FLAG_RXNE) == 1;
    const bool overrun = usart_get_flag(UART_ID,USART_FLAG_ORE) == 1;
    if(data_received || overrun){
        if(!RingBuffer_write_byte(&rb, (uint8_t)usart_recv(UART_ID))){
            //TODO: Failure? 
        }
    }
}

void uart_setup(void){
    RingBuffer_setup(&rb, data_buffer, UART_RING_BUFFER_SIZE);
    // Enable clock for GPIO port A (USART2)
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_USART2);
    
    // Enable interrupt
    nvic_enable_irq(NVIC_USART2_IRQ);

    // Configure IO Port
    gpio_set_mode(UART_PORT, GPIO_MODE_OUTPUT_50_MHZ,GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, UART_TX_PIN);
    gpio_set_mode(UART_PORT, GPIO_MODE_INPUT,GPIO_CNF_INPUT_FLOAT, UART_RX_PIN);

    // USART parameters.
    usart_set_baudrate(UART_ID, BAUD_RATE);
    usart_set_databits(UART_ID, 8);
    usart_set_stopbits(UART_ID, USART_STOPBITS_1);
    usart_set_parity(UART_ID, USART_PARITY_NONE);
    usart_set_flow_control(UART_ID, USART_FLOWCONTROL_NONE);
    usart_set_mode(UART_ID, USART_MODE_TX_RX);
    usart_enable_rx_interrupt(UART_ID);

    // Enable USART
    usart_enable(UART_ID);
}

void uart_shutdown(void){
    usart_disable(UART_ID);
    usart_disable_rx_interrupt(UART_ID);
    nvic_disable_irq(NVIC_USART2_IRQ);
    rcc_periph_clock_disable(RCC_USART2);
    rcc_periph_clock_disable(RCC_GPIOA);
}

void uart_write_byte(uint8_t byte){
    usart_send_blocking(UART_ID,(uint16_t)byte);
}

void uart_write_buffer(const uint8_t* data, uint32_t size){
    for(uint32_t i = 0; i<size; i+=1){
        uart_write_byte(data[i]);
    }
}

uint8_t uart_read_byte(void){
    uint8_t byte;
    (void)uart_read_buffer(&byte, 1);
    return byte;
}

uint32_t uart_read_buffer(uint8_t* buffer,const uint32_t size){
    if(size == 0){
        return 0;
    }

    for(uint32_t i = 0;i<size;i+=1){
        if(!RingBuffer_read_byte(&rb,&buffer[i])){
            return i;
        }
    }

    return size;
}

bool uart_data_available(void){
    return RingBuffer_empty(&rb) == false;
}
