#ifndef _CRC32_
#define _CRC32_

#include <stdint.h>

uint32_t
calculate_crc32c(uint32_t crc32c,
    const unsigned char *buffer,
    unsigned int length);

#endif
