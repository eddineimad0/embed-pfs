#ifndef INC_BOOTLOADER_H
#define INC_BOOTLOADER_H
#include<common.h>
#include<libopencm3/stm32/memorymap.h>

#define BOOTLOADER_SIZE (0x4000U)
#define FIRMWARE_START_PAGE (16U)
#define FIRMWARE_END_PAGE (24U)

typedef struct firmware_info_t{
#define FWINFO_VERSION_OFFSET (0U)
#define FWINFO_SIZE_OFFSET (4U)
#define FWINFO_SIGNATURE_OFFSET (8U)
#define FWINFO_SIGNATURE_LENGTH (64U)
#define FWINFO_SIZE (FWINFO_SIGNATURE_OFFSET + FWINFO_SIGNATURE_LENGTH)

    uint8_t signature[FWINFO_SIGNATURE_LENGTH];
    uint32_t version;
    uint32_t size;
}FirmwareInfo;

#define FIRMWARE_VECTOR_START (FLASH_BASE + BOOTLOADER_SIZE)


#endif // !INC_BOOTLOADER_H
