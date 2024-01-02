#include<ring-buffer.h>

void RingBuffer_setup(RingBuffer* rb,uint8_t* buffer,uint32_t size){
    rb->buffer = buffer;
    rb->reader_index = 0;
    rb->writer_index = 0;
    rb->mask = size - 1;
}

bool RingBuffer_empty(const RingBuffer* rb){
    return rb->reader_index == rb->writer_index;
}

bool RingBuffer_read_byte(RingBuffer* rb,uint8_t* output){
    uint32_t rindex_copy = rb->reader_index;
    uint32_t windex_copy = rb->writer_index;

    if(rindex_copy == windex_copy ){
        return false;
    }

    *output = rb->buffer[rindex_copy];
    rindex_copy = (rindex_copy + 1) & rb->mask; // wraps around;
    rb->reader_index = rindex_copy;
    return true;
}

bool RingBuffer_write_byte(RingBuffer* rb,uint8_t byte){
    uint32_t rindex_copy = rb->reader_index;
    uint32_t windex_copy = rb->writer_index;
    uint32_t next_windex = (windex_copy + 1) & rb->mask;
    if(next_windex == rindex_copy){
        return false;
    }
    rb->buffer[windex_copy] = byte;
    rb->writer_index = next_windex;
    return true;
}
