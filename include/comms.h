#ifndef INC_COMMS_H
#define INC_COMMS_H
#include<common.h>

typedef struct packet_t{

#define PACKET_LENGTH_BYTES (1)
#define PACKET_PAYLOAD_BYTES (19)
#define PACKET_CRC_BYTES (4) // CRC32
#define PACKET_SIZE (PACKET_LENGTH_BYTES + PACKET_PAYLOAD_BYTES + PACKET_CRC_BYTES)
#define PACKET_ACK_BYTE0 (0x06)
#define PACKET_RET_BYTE0 (0x07)
#define PACKET_SYN_BYTE0 (0x16)
#define PACKET_NAK_BYTE0 (0x15)
#define PACKET_PAYLOAD_PADDING (0xFF)

    uint8_t length;
    uint8_t data[PACKET_PAYLOAD_BYTES];
    uint32_t crc;

}Packet;

uint32_t Packet_compute_crc32(const Packet* pkt);
bool Packet_is_cntrl(const Packet* pkt,uint8_t cntrl_byte);
void Packet_create_single_byte(Packet* pkt, uint8_t byte);

bool comms_is_packet_available(void);
void comms_setup(void);
void comms_shutdown(void);
void comms_update(void);
void comms_write(const Packet* pkt);
void comms_read(Packet* pkt);
#endif
