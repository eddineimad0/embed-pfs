#ifndef INC_MEM_H
#define INC_MEM_H
#include <common.h>
void m_memcpy(const void* restrict src, void* restrict dst, uint32_t nbytes);
void m_memset(void* dst, uint8_t value, uint32_t size);
bool m_memcmp(const void* restrict left, const void* restrict right, uint32_t size);
uint32_t read32_from_le_bytes(const uint8_t* bytes);
#endif // !INC_MEM_H
