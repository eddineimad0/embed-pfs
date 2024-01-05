#include<common.h>
#include<bootloader.h>
#include<systick.h>
#include<comms.h>
#include<uart.h>
#include<libopencm3/stm32/memorymap.h>
#include<libopencm3/stm32/crc.h>
#include<libopencm3/stm32/rcc.h>
#include<libopencm3/cm3/scb.h>

static void setup_firmware_vector(void){
    SCB_VTOR = BOOTLOADER_SIZE;
}

static void boot_wake_up(void){
    rcc_clock_setup_pll(&rcc_hsi_configs[RCC_CLOCK_HSI_64MHZ]);
    systick_setup();
    uart_setup();
    comms_setup();
}

static void jump_to_firmware(void){
    typedef void(*reset_fn)(void);
    uint32_t** firmware_reset_vector_entry = (uint32_t**)(FIRMWARE_VECTOR_START + sizeof(uint32_t));
    uint32_t* firmware_reset_fn_address = (*firmware_reset_vector_entry);
    reset_fn firware_reset = (reset_fn)(firmware_reset_fn_address);
    setup_firmware_vector();
    firware_reset();
}

int main(void){
    boot_wake_up();
    Packet pkt = {
        .length = 9,
        .data = {'h','e','l','l','o','i','m','a','d',0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF},
    };
    pkt.crc = Packet_compute_crc32(&pkt);
    /* crc_reset(); */
    /* pkt.crc = crc_calculate(0xFF); */
    while(true){
        // bootloader loop
        comms_write(&pkt);
        systick_delay(3);
    }

    // TODO: unregister everything
    jump_to_firmware();
    return 0; 
}
