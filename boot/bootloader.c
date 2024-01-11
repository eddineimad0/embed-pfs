#include<common.h>
#include<bootloader.h>
#include<systick.h>
#include<comms.h>
#include<uart.h>
#include<flash-controller.h>
#include<timer.h>
#include<mem.h>
#include<sha1.h>
#include<hmac.h>
#include<libopencm3/stm32/rcc.h>
#include<libopencm3/cm3/scb.h>

#include "fw-update.h"


#define SYNC_SEQ_0 (0xDE)
#define SYNC_SEQ_1 (0xAD)
#define SYNC_SEQ_2 (0xBA)
#define SYNC_SEQ_3 (0xBE)

typedef enum bl_state_t{
    BL_Sync,
    BL_WaitingForUpdateReq,
    BL_FWLengthReq,
    BL_FWLengthRes,
    BL_ReceiveFW,
    BL_CheckSignature,
    BL_UpdateFW,
    BL_Timeout,
    BL_Done,
    BL_FWSwitch,
}BLState;

static uint8_t sync_seq[4] = {0};
static uint8_t calculated_firmware_hash[FW_HASH_LENGTH] = {0};
static uint8_t hmac_result[FW_HASH_LENGTH] = {0};
static uint8_t pub_key[4] = {0xAD,0xDE,0xDE,0x0D};

static void setup_firmware_vector(void){
    SCB_VTOR = BOOTLOADER_SIZE;
}

static void boot_wake_up(void){
    rcc_clock_setup_pll(&rcc_hsi_configs[RCC_CLOCK_HSI_64MHZ]);
    systick_setup();
    uart_setup();
    comms_setup();
}

static void boot_cleanup(void){
    comms_shutdown();
    uart_shutdown();
    systick_shutdown();
}

static void fw_update_fail(Packet* pkt){
    Packet_create_single_byte(pkt, PACKET_NAK_BYTE0);
    comms_write(pkt);
}

static bool check_for_timeout(Timer* t,Packet* pkt){
    if(Timer_has_elapsed(t)){
        fw_update_fail(pkt);
        return true;
    }
    return false;
}

static void bl_erase_firmware(void){
    flash_erase_section(FIRMWARE_START_PAGE, FIRMWARE_END_PAGE + 1);
}

static uint32_t bl_write_firmware(const uint8_t* firmware_data, const uint32_t size,const uint32_t offset){
    return flash_write(FIRMWARE_VECTOR_START + offset, firmware_data, size);
}

static void jump_to_firmware(void){
    typedef void(*reset_fn)(void);
    uint32_t** firmware_reset_vector_entry = (uint32_t**)(FIRMWARE_VECTOR_START + sizeof(uint32_t));
    uint32_t* firmware_reset_fn_address = (*firmware_reset_vector_entry);
    reset_fn firware_reset = (reset_fn)(firmware_reset_fn_address);
    setup_firmware_vector();
    firware_reset();
}

static void calculate_sha1(const uint8_t* msg, unsigned nbytes, uint8_t* output)
{
  struct sha1 ctx;
  sha1_reset(&ctx);
  sha1_input(&ctx, msg, nbytes);
  sha1_result(&ctx, output);
}

static void check_for_firmware_update(){
    Timer t;
    Packet pkt;
    BLState state = BL_Sync;
    uint8_t fw_buffer[MAX_FW_LENGTH] = {0};
    uint32_t fw_length = 0;
    uint32_t bytes_written = 0;
    Timer_setup(&t, FU_DEFAULT_TIMEOUT, false);
    while(true){
        if(state == BL_Sync){
            if(uart_data_available()){
                sync_seq[0] = sync_seq[1];
                sync_seq[1] = sync_seq[2];
                sync_seq[2] = sync_seq[3];
                sync_seq[3] = uart_read_byte();

                bool is_sync_seq = sync_seq[0] == SYNC_SEQ_0;
                is_sync_seq = is_sync_seq && sync_seq[1] == SYNC_SEQ_1;
                is_sync_seq = is_sync_seq && sync_seq[2] == SYNC_SEQ_2;
                is_sync_seq = is_sync_seq && sync_seq[3] == SYNC_SEQ_3;

                if(is_sync_seq){
                    while(uart_data_available()){
                        uart_read_byte();
                    }
                    Packet_create_single_byte(&pkt, FU_PACKET_SYN_OBSERVED_BYTE0);
                    comms_write(&pkt);
                    state = BL_WaitingForUpdateReq;
                    Timer_reset(&t);
                    continue;
                }else{
                    if(check_for_timeout(&t,&pkt)){
                        state = BL_Timeout;
                    }
                }
            }else{
                if(check_for_timeout(&t,&pkt)){
                    state = BL_Timeout;
                }
            }
            continue;
        }

        comms_update();
        switch(state){
            case BL_WaitingForUpdateReq:{
                if(comms_is_packet_available()){
                    comms_read(&pkt);
                    if(Packet_is_cntrl(&pkt, FU_PACKET_UP_REQ_BYTE0)){
                        Packet_create_single_byte(&pkt, FU_PACKET_UP_RESP_BYTE0);
                        comms_write(&pkt);
                        state = BL_FWLengthReq;
                        /* Timer_reset(&t); */
                        continue;
                    }else{
                        fw_update_fail(&pkt);
                    }
                }else{
                    if(check_for_timeout(&t,&pkt)){
                        state = BL_Timeout;
                    }
                }
            }break;
            case BL_FWLengthReq:{
                Packet_create_single_byte(&pkt, FU_PACKET_FW_LENGTH_REQ_BYTE0);
                comms_write(&pkt);
                state = BL_FWLengthRes;
                Timer_reset(&t);
                continue;
            }break;
            case BL_FWLengthRes:{
                if(comms_is_packet_available()){
                    comms_read(&pkt);
                    fw_length = read32_from_le_bytes(&pkt.data[0]); 
                    if(fw_length <= MAX_FW_LENGTH){
                        Packet_create_single_byte(&pkt, FU_PACKET_READY_FOR_FW_BYTE0);
                        comms_write(&pkt);
                        state = BL_ReceiveFW;
                        Timer_reset(&t);
                        continue;
                    }else{
                        fw_update_fail(&pkt);
                        state = BL_Done;
                    }
                }else{
                    if(check_for_timeout(&t,&pkt)){
                        state = BL_Timeout;
                    }
                }
                
            }break;
            case BL_ReceiveFW:{
                if(comms_is_packet_available()){
                    comms_read(&pkt);
                    // Signature check.
                    //
                    /* bytes_written = bl_write_firmware(pkt.data,pkt.length,bytes_written); */
                    m_memcpy(pkt.data, &fw_buffer[bytes_written], pkt.length);
                    bytes_written += pkt.length;
                    if(bytes_written >= fw_length){
                        /* state = BL_CheckSignature; */
                        state = BL_UpdateFW;
                    }else{
                        Packet_create_single_byte(&pkt, FU_PACKET_READY_FOR_FW_BYTE0);
                        comms_write(&pkt);
                    }
                    Timer_reset(&t);
                    continue;
                }else{
                    if(check_for_timeout(&t,&pkt)){
                        state = BL_Timeout;
                    }
                }
            }break;
            case BL_CheckSignature:{
                calculate_sha1(&fw_buffer[FW_HASH_LENGTH],bytes_written - FW_HASH_LENGTH,calculated_firmware_hash);
                hmac_sha1(pub_key, 4,fw_buffer, FW_HASH_LENGTH, hmac_result);
                if(m_memcmp(calculated_firmware_hash,hmac_result , FW_HASH_LENGTH) == 0){
                    state = BL_UpdateFW;
                }else{
                    fw_update_fail(&pkt);
                }
            }break;
            case BL_UpdateFW:{
                bl_erase_firmware();
                bl_write_firmware(fw_buffer, bytes_written,0);
                state = BL_Done;
                continue;
            }break;
            case BL_Done:{
                Packet_create_single_byte(&pkt, FU_PACKET_UPDATE_SUCCESS_BYTE0);
                comms_write(&pkt);
                return;
            }break;
            default:{
                return;
            }break;
        }
    }
}


int main(void){
    boot_wake_up();
    check_for_firmware_update();

    systick_delay(200);
    boot_cleanup();
    jump_to_firmware();
    return 0; 
}
