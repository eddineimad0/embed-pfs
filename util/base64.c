#include<base64.h>
#include<mem.h>

static const uint8_t B64index[256] = { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 62, 63, 62, 62, 63, 52, 53, 54, 55,
56, 57, 58, 59, 60, 61,  0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  3,  4,  5,  6,
7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,  0,
0,  0,  0, 63,  0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51 };

bool b64decode(const void* data, const uint32_t len, uint8_t* output, const uint32_t output_len, uint32_t* count){
    *count = 0;

    if (len == 0) return false;

    uint8_t* p = (uint8_t*)data;
    uint32_t j = 0;
    uint32_t len_mod_4 = len & 3;
    bool pad1 = (len_mod_4 != 0 || p[len - 1] == '=');
    bool pad2 = pad1 && (len_mod_4 > 2 || (len_mod_4 == 0 && p[len - 2] != '='));
    const uint32_t last_index = ((len - pad1) >> 2) << 2;
    /* const uint32_t result_len = ((len + 3) >> 2) * 3; */
    /* if(output_len < last_index || output_len < result_len){ */
    /*     return false; */
    /* } */
    m_memset(output, 0x00, output_len);

    for (uint32_t i = 0; i < last_index; i += 4)
    {
        uint32_t n = B64index[p[i]] << 18 | B64index[p[i + 1]] << 12 | B64index[p[i + 2]] << 6 | B64index[p[i + 3]];
        output[j++] = (uint8_t)(n >> 16);
        output[j++] = (uint8_t)(n >> 8 & 0xFF);
        output[j++] = (uint8_t)(n & 0xFF);
    }
    if (pad1)
    {
        uint32_t n = B64index[p[last_index]] << 18 | B64index[p[last_index + 1]] << 12;
        output[j++] = (uint8_t)(n >> 16);

        if (pad2)
        {
            n |= B64index[p[last_index + 2]] << 6;
            output[j++] = (uint8_t)(n >> 8 & 0xFF);
        }
    }

    *count = j;
    return true;
}
