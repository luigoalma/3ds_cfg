#pragma once

#include <3ds/types.h>
#include <stddef.h>
#include <assert.h>

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
static_assert(sizeof(((PS_RSA_Context*)NULL)->mod) == 256);
static_assert(offsetof(PS_RSA_Context, small_exp) == 256);
static_assert(sizeof(((PS_RSA_Context*)NULL)->small_exp) == 4);
static_assert(offsetof(PS_RSA_Context, exp) == 256);
static_assert(sizeof(((PS_RSA_Context*)NULL)->exp) == 256);
static_assert(offsetof(PS_RSA_Context, rsa_bit_size) == 512);
static_assert(sizeof(((PS_RSA_Context*)NULL)->rsa_bit_size) == 4);
static_assert(offsetof(PS_RSA_Context, is_full_exponent) == 516);
static_assert(sizeof(((PS_RSA_Context*)NULL)->is_full_exponent) == 1);

typedef enum {
	CBC_Encrypt = 0,
	CBC_Decrypt = 1,
	CTR_Encrypt = 2,
	CTR_Decrypt = 3,
	CCM_Encrypt = 4,
	CCM_Decrypt = 5
} PS_AES_AlgoTypes;

Result psInit(void);
void psExit(void);
Result PS_VerifyRsaSha256(u8 *hash, PS_RSA_Context *ctx, u8 *signature);
Result PS_GetLocalFriendCodeSeed(u64* seed);
Result PS_GenerateRandomBytes(void* out, size_t len);

#ifdef __cplusplus
}
#endif
