#ifndef INC_FW_UPDATE_H
#define INC_FW_UPDATE_H
#include<common.h>

// Choosen randomly
#define FU_PACKET_SYN_OBSERVED_BYTE0 (0x01)
#define FU_PACKET_UP_REQ_BYTE0 (0x1A)
#define FU_PACKET_UP_RESP_BYTE0 (0x1B)
#define FU_PACKET_FW_LENGTH_REQ_BYTE0 (0x0F)
#define FU_PACKET_FW_LENGTH_RESP_BYTE0 (0x0E)
#define FU_PACKET_READY_FOR_FW_BYTE0 (0x02)
#define FU_PACKET_UPDATE_SUCCESS_BYTE0 (0x04)

#define FU_DEFAULT_TIMEOUT (5000)

#define MAX_FW_LENGTH  (5*1024U)
#define FW_HASH_LENGTH  (32U)

#define SYNC_SEQ_0 (0xDE)
#define SYNC_SEQ_1 (0xAD)
#define SYNC_SEQ_2 (0xBA)
#define SYNC_SEQ_3 (0xBE)

// Verifying key
#include "key/verify.inc"

#endif // !INC_FW_UPDATE_H
