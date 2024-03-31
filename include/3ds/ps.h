#pragma once

#include <3ds/types.h>
#include <stddef.h>
#include <assert.h>
#include <assert_helpers.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	u8 mod[0x100];
	union {
		u32 small_exp; // = this member if is_full_exponent is 0, e.g. = 0x10001
		u8 exp[0x100]; // memcpy big endian number to this member if is_full_exponent is 1
	};
	s32 rsa_bit_size;
	u8 is_full_exponent; // 1 if big endian full exponent, 0 if small 4 byte little endian exponent
	u8 padding[3];
} PS_RSA_Context;

static_assert(sizeof(PS_RSA_Context) == 0x208, "Invalid PS_RSA_Context size compiled");
static_assert(offsetof(PS_RSA_Context, mod) == 0);
static_assert(sizeofmember(PS_RSA_Context, mod) == 256);
static_assert(offsetof(PS_RSA_Context, small_exp) == 256);
static_assert(sizeofmember(PS_RSA_Context, small_exp) == 4);
static_assert(offsetof(PS_RSA_Context, exp) == 256);
static_assert(sizeofmember(PS_RSA_Context, exp) == 256);
static_assert(offsetof(PS_RSA_Context, rsa_bit_size) == 512);
static_assert(sizeofmember(PS_RSA_Context, rsa_bit_size) == 4);
static_assert(offsetof(PS_RSA_Context, is_full_exponent) == 516);
static_assert(sizeofmember(PS_RSA_Context, is_full_exponent) == 1);

typedef enum {
	CBC_Encrypt = 0,
	CBC_Decrypt = 1,
	CTR_Encrypt = 2,
	CTR_Decrypt = 3,
	CCM_Encrypt = 4,
	CCM_Decrypt = 5
} PS_AES_AlgoTypes;

/// Initializes PS.
Result psInit(void);

/// Exits PS.
void psExit(void);

/**
 * @brief Verifies a RSA signature.
 * @param hash SHA256 hash to compare with.
 * @param ctx RSA context.
 * @param signature RSA signature.
 */
Result PS_VerifyRsaSha256(u8 *hash, const PS_RSA_Context *ctx, u8 *signature);

/**
 * @brief Gets the 64-bit console friend code seed.
 * @param seed Pointer to write the friend code seed to.
 */
Result PS_GetLocalFriendCodeSeed(u64* seed);

/**
 * @brief Generates cryptographically secure random bytes.
 * @param out Pointer to the buffer to write the bytes to.
 * @param len Number of bytes to write.
 */
Result PS_GenerateRandomBytes(void* out, size_t len);

#ifdef __cplusplus
}
#endif
