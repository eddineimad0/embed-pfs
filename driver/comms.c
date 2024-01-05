#include<comms.h>
#include<uart.h>
#include<mem.h>
#include<libopencm3/stm32/crc.h>
#include<libopencm3/stm32/rcc.h>

#define PACKET_BUFFER_SIZE (8)
#define PACKET_BUFFER_MASK PACKET_BUFFER_SIZE 

typedef enum comms_state_t{
    Await_Length,
    Await_Paylod,
    Await_CRC
} CommsState;

static CommsState state = Await_Length;
static uint8_t payloads_bytes_count = 0;
static uint8_t crc_bytes_count = 0;

static Packet temp = {.length = 0,.data = {0},.crc = 0};
static Packet ret = {.length = 0,.data = {0},.crc = 0};
static Packet ack = {.length = 0,.data = {0},.crc = 0};
static Packet last_received = {.length = 0,.data = {0},.crc = 0};

static Packet buffer[PACKET_BUFFER_SIZE];
static uint32_t buffer_read_idx = 0;
static uint32_t buffer_write_idx = 0;

bool comms_is_packet_available(void){
    return (buffer_write_idx != buffer_read_idx);
}

static bool Packet_is_cntrl(Packet* pkt,uint8_t cntrl_byte){
    if(pkt->length != 1){
        return false;
    }

    if(pkt->data[0] != cntrl_byte){
        return false;
    }

    for(uint8_t i = 0; i< PACKET_PAYLOAD_BYTES;i+=1){
        if(pkt->data[i] != PACKET_PAYLOAD_PADDING){
            return false;
        }
    }

    return true;
}

void comms_setup(void){
    rcc_periph_clock_enable(RCC_CRC);
    ret.length = 1;
    ret.data[0] = PACKET_RET_BYTE0;
    for(uint8_t i = 1; i< PACKET_PAYLOAD_BYTES; i+=1){
        ret.data[i] = PACKET_PAYLOAD_PADDING;
    }
    ret.crc = Packet_compute_crc32(&ret);

    ack.length = 1;
    ack.data[0] = PACKET_ACK_BYTE0;
    for(uint8_t i = 1; i< PACKET_PAYLOAD_BYTES; i+=1){
        ret.data[i] = PACKET_PAYLOAD_PADDING;
    }
    ack.crc = Packet_compute_crc32(&ret);
}

void comms_update(void){
    while(uart_data_available()){

        switch (state) {

            case Await_Length: {
                temp.length = uart_read_byte();
                state = Await_Paylod;
            }break;

            case Await_Paylod:{
                temp.data[payloads_bytes_count] = uart_read_byte();
                payloads_bytes_count += 1;
                if(payloads_bytes_count >= PACKET_PAYLOAD_BYTES){
                    payloads_bytes_count = 0;
                    state = Await_CRC;
                }
            }break;

            case Await_CRC:{
                temp.crc |= ((uint32_t)(uart_read_byte())) << (8*(3 - crc_bytes_count));
                crc_bytes_count += 1;
                if(crc_bytes_count >= PACKET_CRC_BYTES){
                    crc_bytes_count = 0;
                    uint32_t computed_crc = Packet_compute_crc32(&temp);
                    if(temp.crc != computed_crc){
                        comms_write(&ret);
                    }else if(Packet_is_cntrl(&temp,PACKET_RET_BYTE0)){
                        // Retransmit last received packet
                        comms_write(&last_received);
                    }else if(Packet_is_cntrl(&temp,PACKET_ACK_BYTE0)){
                        // Drop ACK packets
                        state = Await_Length;
                        break;
                    }

                    uint32_t next_write_index = (buffer_write_idx + 1) & PACKET_BUFFER_MASK;
                    if(next_write_index == buffer_read_idx){
                        // Debug check
                        __asm__("BKPT #0");
                    }
                    custom_memcpy(&temp, &buffer[buffer_write_idx], sizeof(Packet));
                    buffer_write_idx = next_write_index;
                    comms_write(&ack);
                    state = Await_Length;
                }
            }break;

            default:{
                // Unreachable code.
            }
        }
    }
}

void comms_write(Packet* pkt){
    uart_write_buffer((uint8_t*)pkt, sizeof(Packet));
}

void comms_read(Packet* pkt){
    custom_memcpy(&buffer[buffer_read_idx], pkt, sizeof(Packet));
    buffer_read_idx = (buffer_read_idx + 1) & PACKET_BUFFER_MASK;
}

// This function uses the CRC32/BZIP2 algorithme
uint32_t Packet_compute_crc32(Packet* pkt){
    uint8_t* temp_ptr = (uint8_t*)pkt;
    uint8_t reversed_packet[sizeof(Packet) - sizeof(uint32_t)] = {0};

    // stm32 CRC32 peripheral reads the data in big endian while the 
    // cortex m3 processor uses little endian so to get accurate results
    // we will change each 32-bits word to use big endian
    for(uint32_t i=0; i<(sizeof(Packet) - sizeof(uint32_t)); i+=4){
        reversed_packet[i+3] = temp_ptr[i];
        reversed_packet[i+2] = temp_ptr[i+1];
        reversed_packet[i+1] = temp_ptr[i+2];
        reversed_packet[i] = temp_ptr[i+3];
    }
    crc_reset();
    uint32_t crc_checksum = crc_calculate_block((uint32_t*)reversed_packet, (sizeof(Packet) - sizeof(uint32_t))/sizeof(uint32_t));
    // Necessary to filp the bits since the hardware doesn't
    return (crc_checksum ^ 0xFFFFFFFF);
}
