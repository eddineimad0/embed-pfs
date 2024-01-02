#include<common.h>
#include<bootloader.h>
#include<libopencm3/stm32/memorymap.h>
#include<libopencm3/cm3/scb.h>

static void setup_firmware_vector(void){
    SCB_VTOR = BOOTLOADER_SIZE;
}

static void jump_to_main(void){
    typedef void(*reset_fn)(void);
    uint32_t** firmware_reset_vector_entry = (uint32_t**)(FIRMWARE_VECTOR_START + sizeof(uint32_t));
    uint32_t* firmware_reset_fn_address = (*firmware_reset_vector_entry);
    reset_fn firware_reset = (reset_fn)(firmware_reset_fn_address);
    setup_firmware_vector();
    firware_reset();
}

int main(void){
    jump_to_main();
    return 0; 
}
