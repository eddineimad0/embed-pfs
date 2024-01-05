#ifndef INC_COMMS_H
#define INC_COMMS_H
#include<common.h>

typedef struct packet_t{

#define PACKET_LENGTH_BYTES (1)
#define PACKET_PAYLOAD_BYTES (19)
#define PACKET_CRC_BYTES (4) // CRC32
#define PACKET_ACK_BYTE0 (0xAC)
#define PACKET_RET_BYTE0 (0xAB)
#define PACKET_PAYLOAD_PADDING (0xFF)

    uint8_t length;
    uint8_t data[PACKET_PAYLOAD_BYTES];
    uint32_t crc;
}Packet;

uint32_t Packet_compute_crc32(Packet* pkt);

bool comms_is_packet_available(void);
void comms_setup(void);
void comms_update(void);
void comms_write(Packet* pkt);
void comms_read(Packet* pkt);
#endif
