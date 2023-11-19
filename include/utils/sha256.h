#pragma once
#include <inttypes.h>
#include <stddef.h>
#include <stdalign.h>
#include <assert.h>
#include <assert_helpers.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sha256_t {
	uint32_t H[8];
	union {
		uint8_t  M[64];
		uint32_t M32[64/4];
	};
	uint64_t MSize; // total length
} sha256_t;

static_assert(sizeof(sha256_t) == 0x68);
static_assert(alignof(sha256_t) == 8);
static_assert(offsetof(sha256_t, H) == 0);
static_assert(offsetof(sha256_t, M) == 32);
static_assert(offsetof(sha256_t, M32) == 32);
static_assert(offsetof(sha256_t, MSize) == 96);
static_assert(sizeofmember(sha256_t, H) == 32);
static_assert(sizeofmember(sha256_t, M) == 64);
static_assert(sizeofmember(sha256_t, M32) == 64);
static_assert(sizeofmember(sha256_t, MSize) == 8);

#define SHA256_HASH_LENGTH 32

void sha256_init(sha256_t* ctx);
void sha256_update(sha256_t* ctx, const void* ptr, size_t len);
void sha256_finish(sha256_t* ctx);
void sha256_hash(sha256_t* ctx, void* hash);
void sha256_full(const void* ptr, size_t len, void* hash);

#ifdef __cplusplus
}
#endif
