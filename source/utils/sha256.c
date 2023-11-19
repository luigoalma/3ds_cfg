#include <inttypes.h>
#include <utils/sha256.h>
#include <utils/endian.h>
#include <string.h>

// NIST FIPS 180-4
// https://dx.doi.org/10.6028/NIST.FIPS.180-4

static inline uint32_t ror32(uint32_t x, int n) {
	// checks not needed here as they are safe, n never 0 or bigger than 32 on this file
	// normally, shifts over the bit length of type are UB
	//n &= 0x1F;
	//if(!n) return x;
	return (x >> n) | (x << (32-n));
}

// σ0
static inline uint32_t lsigma0(uint32_t x) {
	return ror32(x, 7) ^ ror32(x, 18) ^ (x >> 3);
}

// σ1
static inline uint32_t lsigma1(uint32_t x) {
	return ror32(x, 17) ^ ror32(x, 19) ^ (x >> 10);
}

// Σ0
static inline uint32_t usigma0(uint32_t x) {
	return ror32(x, 2) ^ ror32(x, 13) ^ ror32(x, 22);
}

// Σ1
static inline uint32_t usigma1(uint32_t x) {
	return ror32(x, 6) ^ ror32(x, 11) ^ ror32(x, 25);
}

static inline uint32_t ch(uint32_t x, uint32_t y, uint32_t z) {
	return (x & y) ^ (~x & z);
}

static inline uint32_t maj(uint32_t x, uint32_t y, uint32_t z) {
	return (x & y) ^ (x & z) ^ (y & z);
}

static const uint32_t K[64] = {
	0x428A2F98, 0x71374491, 0xB5C0FBCF, 0xE9B5DBA5,
	0x3956C25B, 0x59F111F1, 0x923F82A4, 0xAB1C5ED5,
	0xD807AA98, 0x12835B01, 0x243185BE, 0x550C7DC3,
	0x72BE5D74, 0x80DEB1FE, 0x9BDC06A7, 0xC19BF174,
	0xE49B69C1, 0xEFBE4786, 0x0FC19DC6, 0x240CA1CC,
	0x2DE92C6F, 0x4A7484AA, 0x5CB0A9DC, 0x76F988DA,
	0x983E5152, 0xA831C66D, 0xB00327C8, 0xBF597FC7,
	0xC6E00BF3, 0xD5A79147, 0x06CA6351, 0x14292967,
	0x27B70A85, 0x2E1B2138, 0x4D2C6DFC, 0x53380D13,
	0x650A7354, 0x766A0ABB, 0x81C2C92E, 0x92722C85,
	0xA2BFE8A1, 0xA81A664B, 0xC24B8B70, 0xC76C51A3,
	0xD192E819, 0xD6990624, 0xF40E3585, 0x106AA070,
	0x19A4C116, 0x1E376C08, 0x2748774C, 0x34B0BCB5,
	0x391C0CB3, 0x4ED8AA4A, 0x5B9CCA4F, 0x682E6FF3,
	0x748F82EE, 0x78A5636F, 0x84C87814, 0x8CC70208,
	0x90BEFFFA, 0xA4506CEB, 0xBEF9A3F7, 0xC67178F2
};

// sha256
void sha256_init(sha256_t* ctx) {
	ctx->H[0] = 0x6A09E667;
	ctx->H[1] = 0xBB67AE85;
	ctx->H[2] = 0x3C6EF372;
	ctx->H[3] = 0xA54FF53A;
	ctx->H[4] = 0x510E527F;
	ctx->H[5] = 0x9B05688C;
	ctx->H[6] = 0x1F83D9AB;
	ctx->H[7] = 0x5BE0CD19;
	ctx->MSize = 0;
}

static void sha256_process512bitblock(sha256_t* ctx) {
	uint32_t W[64];
	uint32_t a, b, c, d, e, f, g, h;

	for(int i = 0; i < 16; ++i) {
		W[i] = getbe32(ctx->M32[i]);
	}

	for(int i = 16; i < 64; ++i) {
		W[i] = lsigma1(W[i-2]) + W[i-7] + lsigma0(W[i-15]) + W[i-16];
	}

	a = ctx->H[0];
	b = ctx->H[1];
	c = ctx->H[2];
	d = ctx->H[3];
	e = ctx->H[4];
	f = ctx->H[5];
	g = ctx->H[6];
	h = ctx->H[7];

	for(int i = 0; i < 64; ++i) {
		uint32_t T1 = h + usigma1(e) + ch(e, f, g) + K[i] + W[i];
		uint32_t T2 = usigma0(a) + maj(a, b, c);

		h = g;
		g = f;
		f = e;
		e = d + T1;
		d = c;
		c = b;
		b = a;
		a = T1 + T2;
	}

	ctx->H[0] += a;
	ctx->H[1] += b;
	ctx->H[2] += c;
	ctx->H[3] += d;
	ctx->H[4] += e;
	ctx->H[5] += f;
	ctx->H[6] += g;
	ctx->H[7] += h;
}

void sha256_update(sha256_t* ctx, const void* ptr, size_t len) {
	while(len) {
		size_t off = ctx->MSize & (sizeof(ctx->M)-1);
		size_t max = sizeof(ctx->M) - off;
		size_t _len = len > max ? max : len;
		memcpy(&ctx->M[off], ptr, _len);
		ctx->MSize += _len;
		len -= _len;
		if(off + _len == 64) {
			sha256_process512bitblock(ctx);
		}
	}
}

void sha256_finish(sha256_t* ctx) {
	size_t off = ctx->MSize & (sizeof(ctx->M)-1);
	size_t max = sizeof(ctx->M) - off - 1;
	ctx->M[off] = 0x80;
	uint64_t bitlength = ctx->MSize * 8;
	if(max >= 8) {
		if(max > 8) memset(&ctx->M[off+1], 0, max-8);
		ctx->M32[14] = getbe32(bitlength >> 32);
		ctx->M32[15] = getbe32(bitlength & 0xFFFFFFFF);
	} else {
		memset(&ctx->M[off+1], 0, max);
		sha256_process512bitblock(ctx);
		memset(&ctx->M[0], 0, sizeof(ctx->M)-8);
		ctx->M32[14] = getbe32(bitlength >> 32);
		ctx->M32[15] = getbe32(bitlength & 0xFFFFFFFF);
	}
	sha256_process512bitblock(ctx);
}

void sha256_hash(sha256_t* ctx, void* hash) {
	if((uintptr_t)hash & (alignof(uint32_t)-1)) {
		uint8_t* ptr = (uint8_t*)hash;
		for(int i = 0; i < 8; ++i) {
			union {
				uint32_t word32;
				uint8_t word8[4];
			} h = {.word32 = getbe32(ctx->H[i])};
			for(int j = 0; j < 4; ++j) {
				ptr[i*4+j] = h.word8[j];
			}
		}
	} else {
		uint32_t* ptr = (uint32_t*)hash;
		for(int i = 0; i < 8; ++i) {
			ptr[i] = getbe32(ctx->H[i]);
		}
	}
}

void sha256_full(const void* ptr, size_t len, void* hash) {
	sha256_t ctx;
	sha256_init(&ctx);
	sha256_update(&ctx, ptr, len);
	sha256_finish(&ctx);
	sha256_hash(&ctx, hash);
}
