#include<mem.h>

void custom_memcpy(const uint8_t* src,uint8_t* dst,uint32_t nbytes){
    for(uint32_t i = 0; i< nbytes;i+=1){
        dst[i] = src[i];
    }
}
