#include <stddef.h>
#include <stdbool.h>
#include <inttypes.h>
#include <utils/base64.h>

static const char encode_table[64] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
	'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
};

static uint32_t _div3_u32(uint32_t x) {
	return (uint32_t)(((uint64_t)x * 0xAAAAAAABllu) >> 33);
}

static uint32_t _divmod3_u32(uint32_t x, uint32_t* mod) {
	uint32_t div = _div3_u32(x);
	uint32_t _mod = x - (div * 3u);
	if(mod) *mod = _mod;
	return div;
}

size_t base64_size(size_t len) {
	uint32_t mod;
	uint32_t div = _divmod3_u32(len, &mod);
	size_t size = div * 4 + (mod ? 4 : 0);
	return (size < len) ? 0 : size; // overflow? 0
}

bool base64_encode(char* out, size_t* outsize, const void* data, size_t len) {
	if(!len) {
		*outsize = 0;
		return true;
	}

	size_t size = base64_size(len);
	if(!size) return false;

	const uint8_t* _data = (const uint8_t*)data;

	while(len >= 3) {
		uint32_t foo = (_data[0] << 16) | (_data[1] << 8) | _data[2];
		out[0] = encode_table[foo >> 18];
		out[1] = encode_table[(foo >> 12) & 0x3F];
		out[2] = encode_table[(foo >> 6) & 0x3F];
		out[3] = encode_table[foo & 0x3F];
		
		_data += 3;
		out += 4;
		len -= 3;
	}

	if(len == 2) {
		uint16_t foo = (_data[0] << 8) | _data[1];
		out[0] = encode_table[foo >> 10];
		out[1] = encode_table[(foo >> 4) & 0x3F];
		out[2] = encode_table[(foo << 2) & 0x3F];
		out[3] = '=';
	} else if(len == 1) {
		uint16_t foo = _data[0];
		out[0] = encode_table[foo >> 2];
		out[1] = encode_table[(foo << 4) & 0x3F];
		out[2] = '=';
		out[3] = '=';
	}

	*outsize = size;
	return true;
}
