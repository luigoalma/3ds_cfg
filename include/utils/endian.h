#pragma once
#include <inttypes.h>

static const union {
	uint16_t foo;
	uint8_t is_little;
} endian_check = {.foo = 1};

static inline uint32_t getbe32(uint32_t value) {
	if(endian_check.is_little) {
		#if defined __clang__ || defined __GNUC__
		value = __builtin_bswap32(value);
		#elif defined _MSC_VER
		value = _byteswap_ulong(value);
		#else
		value = value << 24 | value << 16 >> 24 << 16 | value << 8 >> 24 << 8 | value >> 24;
		#endif
	}
	return value;
}
