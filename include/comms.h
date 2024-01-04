#ifndef INC_COMMS_H
#define INC_COMMS_H
#include<common.h>

typedef struct packet_t{
#define PACKET_PAYLOAD_BYTES (19)
#define PACKET_LENGTH_BYTES (1)
#define PACKET_CRC_BYTES (4) // CRC32
#define PACKET_SIZE (PACKET_LENGTH_BYTES + PACKET_PAYLOAD_BYTES + PACKET_CRC_BYTES)
#define PACKET_ACK_BYTE0 (0xAC)
#define PACKET_RET_BYTE0 (0xAB)
    uint8_t length;
    uint8_t data[PACKET_PAYLOAD_BYTES];
    uint32_t crc;
}Packet;

bool Packet_is_available(void);
uint32_t Packet_compute_crc32(Packet* pkt);

void comms_setup(void);
void comms_update(void);
void comms_write(Packet* pkt);
void comms_read(Packet* pkt);
#endif
