#include<mem.h>
#include<flash-controller.h>
#include<libopencm3/stm32/flash.h>

void flash_erase_section(const uint8_t start_page, const uint8_t end_page){
    flash_unlock();
    for(uint8_t i = start_page; i < end_page; i += 1){
        flash_erase_page(FLASH_BASE_ADDRESS + (FLASH_PAGE_SIZE * i));
    }
    flash_lock();
}

uint32_t flash_write(uint32_t base_address, const uint8_t* data, const uint32_t size){
    uint32_t bytes_written = 0;
    uint32_t word_to_write = 0;
    uint32_t address = base_address;
    uint32_t word_aligned_reads = size/4;
    uint32_t read_indx = 0;

    if(base_address >= FLASH_LAST_ADDRESS){
        return 0;
    }

    flash_unlock();

    for(uint32_t i = 0; i < word_aligned_reads; i += 1){
        read_indx = 4*i;
        word_to_write = read32_from_le_bytes(&data[read_indx]);
        flash_program_word(address, word_to_write);
        address += 4;
        bytes_written += 4;
        if(address >= FLASH_LAST_ADDRESS){
            break;
        }
    }

    uint32_t bytes_left = size - bytes_written;

    if(bytes_left != 0){
        uint8_t padd[4] = {0xFF};
        for(uint32_t i = 0; i<bytes_left; i += 1){
            read_indx = bytes_written + i;
            padd[i]=data[read_indx];
        }
        word_to_write = read32_from_le_bytes(padd);
        flash_program_word(address, word_to_write);
        bytes_written += 4;
    }

    flash_lock();
    return bytes_written;
}
