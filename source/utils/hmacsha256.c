#include <inttypes.h>
#include <utils/hmacsha256.h>
#include <utils/sha256.h>
#include <string.h>

typedef struct hmacsha256_t {
	sha256_t sha256_ctx;
	union {
		uint8_t key[64];
		uint32_t key_u32[16];
	};
} hmacsha256_t;

static void hmacsha256_init(hmacsha256_t* ctx, const void* key, size_t keylen) {
	size_t remainder;
	uint32_t round0[16]; 

	if(keylen > 64) {
		sha256_full(key, keylen, &ctx->key[0]);
		remainder = 32;
	} else {
		memcpy(&ctx->key[0], key, keylen);
		remainder = 64 - keylen;
	}

	memset(&ctx->key[64 - remainder], 0, remainder);

	for(int i = 0; i < 16; ++i)
		round0[i] = ctx->key_u32[i] ^ 0x36363636;

	sha256_init(&ctx->sha256_ctx);
	sha256_update(&ctx->sha256_ctx, round0, sizeof(round0));
}

void calculate_hmacsha256(const void* ptr, size_t len, void* hmac, const void* key, size_t keylen) {
	hmacsha256_t ctx;
	uint32_t data_u32[24];

	hmacsha256_init(&ctx, key, keylen);

	sha256_update(&ctx.sha256_ctx, ptr, len);
	sha256_finish(&ctx.sha256_ctx);
	sha256_hash(&ctx.sha256_ctx, &data_u32[16]);

	for(int i = 0; i < 16; ++i)
		data_u32[i] = ctx.key_u32[i] ^ 0x5C5C5C5C;

	sha256_init(&ctx.sha256_ctx);
	sha256_update(&ctx.sha256_ctx, data_u32, sizeof(data_u32));
	sha256_finish(&ctx.sha256_ctx);
	sha256_hash(&ctx.sha256_ctx, hmac);
}
