#ifndef INC_FLASH_CONTROLLER_H
#define INC_FLASH_CONTROLLER_H
#include<common.h>
#include<libopencm3/stm32/memorymap.h>

#define FLASH_BASE_ADDRESS (FLASH_BASE)
#define FLASH_LAST_ADDRESS (FLASH_BASE_ADDRESS + (64 * FLASH_PAGE_SIZE) )
#define FLASH_PAGE_SIZE (1024U)

void flash_erase_section(const uint8_t start_page,const uint8_t end_page);
uint32_t flash_write(uint32_t address, const uint8_t* data, const uint32_t size);

#endif // !INC_FLASH_CONTROLLER_H
