#include<mem.h>

void custom_memcpy(const void* src,void* dst,uint32_t nbytes){
    for(uint32_t i = 0; i< nbytes;i+=1){
        ((uint8_t*)dst)[i] = ((uint8_t*)src)[i];
    }
}
