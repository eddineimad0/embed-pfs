#ifndef INC_RING_BUFFER_H
#define INC_RING_BUFFER_H
#include<common.h>

typedef struct ring_buffer_t{
    uint8_t* buffer;
    uint32_t mask;
    uint32_t reader_index;
    uint32_t writer_index;
}RingBuffer;

void RingBuffer_setup(RingBuffer* rb,uint8_t* buffer,uint32_t size);
bool RingBuffer_write_byte(RingBuffer* rb,uint8_t byte);
bool RingBuffer_read_byte(RingBuffer* rb,uint8_t* output);
bool RingBuffer_empty(const RingBuffer* rb);
#endif
