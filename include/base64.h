#ifndef INC_BASE64_H
#define INC_BASE64_H 
#include<common.h>

bool b64decode(const void* data, const uint32_t len, uint8_t* output, const uint32_t output_len, uint32_t* count);

#endif // !INC_BASE64_H
