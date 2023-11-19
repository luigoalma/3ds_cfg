#include <inttypes.h>
#include <utils/crc16.h>

static const uint16_t table[16] = {
	0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401,
	0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400
};

uint16_t crc16(uint16_t crc, const void* data, size_t len) {
	const uint8_t* _data = (const uint8_t*)data;

	for(; len; --len) {
		uint8_t b = *_data++;
		crc = (crc >> 4) ^ table[crc & 0xF] ^ table[b & 0xF];
		crc = (crc >> 4) ^ table[crc & 0xF] ^ table[(b >> 4) & 0xF];
	}

	return crc;
}
