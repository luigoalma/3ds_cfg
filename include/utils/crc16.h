#pragma once
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

uint16_t crc16(uint16_t crc, const void* data, size_t len);

#ifdef __cplusplus
}
#endif