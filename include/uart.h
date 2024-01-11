#ifndef INC_UART_H
#define INC_UART_H

#include<common.h>

#define BAUD_RATE (115200U)
#define UART_RING_BUFFER_SIZE (256)

void uart_setup(void);
void uart_shutdown(void);
void uart_write_byte(uint8_t byte);
void uart_write_buffer(const uint8_t* data,uint32_t size);
uint8_t uart_read_byte(void);
uint32_t uart_read_buffer(uint8_t* buffer,const uint32_t size);
bool uart_data_available(void);

#endif // !INC_UART_H
