#pragma once
#include "sha256.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HMAC_SHA256_HASH_LENGTH SHA256_HASH_LENGTH

void calculate_hmacsha256(const void* ptr, size_t len, void* hmac, const void* key, size_t keylen);

#ifdef __cplusplus
}
#endif
