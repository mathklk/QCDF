#ifndef CRC32_H
#define CRC32_H

#include <stdint.h>
#include <stddef.h>

static uint32_t crc32(const uint8_t *data, size_t len) {
    uint32_t crc = 0xFFFFFFFFu;
    for (size_t i = 0; i < len; ++i) {
        crc ^= data[i];
        for (uint8_t bit = 0; bit < 8; ++bit) {
            if (crc & 1u) {
                crc = (crc >> 1) ^ 0xEDB88320u;  // reversed poly
            } else {
                crc >>= 1;
            }
        }
    }
    return crc ^ 0xFFFFFFFFu;
}

#endif // CRC32_H
