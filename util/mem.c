#include<mem.h>

void m_memcpy(const void* restrict src, void* restrict  dst, uint32_t nbytes){
    for(uint32_t i = 0; i< nbytes; i+=1){
        ((uint8_t* restrict)dst)[i] = ((uint8_t* restrict)src)[i];
    }
}

void m_memset(void* dst, uint8_t value, uint32_t size){
    for(uint32_t i = 0; i< size; i += 1){
        ((uint8_t*)dst)[i] = value;
    }
}

bool m_memcmp(const void* restrict left, const void* restrict right, uint32_t size){
    for(uint32_t i = 0; i<size; i+=1){
        if(((uint8_t* restrict)left)[i] != ((uint8_t* restrict)right)[i]){
            return false;
        }
    }
    return true;
}

uint32_t read32_from_le_bytes(const uint8_t* bytes){
    return (
                (uint32_t)(bytes[3]) << 24 |
                (uint32_t)(bytes[2]) << 16 |
                (uint32_t)(bytes[1]) <<  8 |
                (uint32_t)(bytes[0])       
            );
}

uint32_t bytes_len(const uint8_t* bytes){
    uint32_t count = 0;

    while(*bytes != 0x00){
        bytes += 1;
        count += 1;
    }

    return count;
}
