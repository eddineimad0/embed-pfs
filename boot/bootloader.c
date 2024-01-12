#include<common.h>
#include<systick.h>
#include<comms.h>
#include<uart.h>
#include<flash-controller.h>
#include<timer.h>
#include<mem.h>
#include<sha256.h>
#include<base64.h>
#include<uECC.h>
#include<libopencm3/stm32/rcc.h>
#include<libopencm3/cm3/scb.h>

#include"bootloader.h"
#include"fw-update.h"


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

static FirmwareInfo fwinfo;

static void setup_firmware_vector(void){
    SCB_VTOR = (BOOTLOADER_SIZE);
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

static uint32_t bl_write_firmware(const uint8_t* firmware_data, const uint32_t size){
    return flash_write(FIRMWARE_VECTOR_START, firmware_data, size);
}

static void jump_to_firmware(void){
    typedef void(*reset_fn)(void);
    uint32_t** firmware_reset_vector_entry = (uint32_t**)(FIRMWARE_VECTOR_START + sizeof(uint32_t));
    uint32_t* firmware_reset_fn_address = (*firmware_reset_vector_entry);
    reset_fn firware_reset = (reset_fn)(firmware_reset_fn_address);
    setup_firmware_vector();
    firware_reset();
}


static void read_firmware_info(const uint8_t* restrict firmware_data, const uint32_t data_len, FirmwareInfo* info){
    const uint32_t fw_info_offset = data_len - FWINFO_SIZE;
    info->version = read32_from_le_bytes(&firmware_data[fw_info_offset + FWINFO_VERSION_OFFSET]);
    info->size = read32_from_le_bytes(&firmware_data[fw_info_offset + FWINFO_SIZE_OFFSET]);
    m_memcpy(&firmware_data[fw_info_offset + FWINFO_SIGNATURE_OFFSET], info->signature, FWINFO_SIGNATURE_LENGTH);
}

static void calculate_firmware_hash(const uint8_t* firmware_data,const uint32_t firmware_size, uint8_t* output){
    ecdsa_sha256_context_t ctx;
    ecdsa_sha256_init(&ctx);
    ecdsa_sha256_update(&ctx,firmware_data,firmware_size);
    ecdsa_sha256_final(&ctx,output);
}

static bool verify_firmware_signature(const FirmwareInfo* info, const uint8_t* fw_hash){
    uint8_t binary_pub_key[VERIFYING_KEY_LENGTH]; 
    uint32_t key_length = bytes_len((const uint8_t*)VERIFYING_KEY);
    uint32_t output_length = 0;

    if(!b64decode(VERIFYING_KEY, key_length , binary_pub_key, VERIFYING_KEY_LENGTH, &output_length)){
        // Failed to decode public key fail the verification.
        return false;
    }

    uECC_Curve curve = uECC_secp256k1();
    return uECC_verify(binary_pub_key, fw_hash, FW_HASH_LENGTH, info->signature, curve) == 1;
}


static void check_for_firmware_update(){
    Timer t;
    Packet pkt;
    BLState state = BL_Sync;
    uint8_t fw_buffer[MAX_FW_LENGTH] = {0};
    uint8_t fw_hash[FW_HASH_LENGTH] = {0};
    uint8_t sync_seq[4] = {0};
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
                    if(fw_length <= MAX_FW_LENGTH && fw_length >= FWINFO_SIZE ){
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
                    m_memcpy(pkt.data, &fw_buffer[bytes_written], pkt.length);
                    bytes_written += pkt.length;
                    if(bytes_written >= fw_length){
                        state = BL_CheckSignature;
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
                read_firmware_info(fw_buffer,bytes_written,&fwinfo);
                if(fwinfo.size >= (MAX_FW_LENGTH - FWINFO_SIZE)){
                    fw_update_fail(&pkt);
                    state = BL_Timeout;
                }
                calculate_firmware_hash(fw_buffer,fwinfo.size,fw_hash);

                if(verify_firmware_signature(&fwinfo,fw_hash)){
                    Packet_create_single_byte(&pkt, FU_PACKET_UPDATE_SUCCESS_BYTE0);
                    comms_write(&pkt);
                    state = BL_UpdateFW;
                }else{
                    fw_update_fail(&pkt);
                    state = BL_Timeout;
                }
                continue;
            }break;
            case BL_UpdateFW:{
                bl_erase_firmware();
                bl_write_firmware(fw_buffer, bytes_written);
                state = BL_Done;
                continue;
            }break;
            case BL_Done:{
                return;
            }break;
            default:{
                return;
            }break;
        }
    }
}

/* static void infinite_loop(){ */
/*     while(true){ */
/*         __asm__("nop"); */
/*     } */
/* } */


int main(void){
    boot_wake_up();
    check_for_firmware_update();

    systick_delay(200);
    boot_cleanup();
    jump_to_firmware();
    return 0; 
}
