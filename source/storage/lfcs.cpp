#include <cstddef>
#include <3ds/fs.h>
#include <3ds/os.h>
#include <3ds/ps.h>
#include <3ds/result.h>
#include <3ds/types.h>
#include <err.h>
#include <storage/lfcs.h>
#include "storage.hpp"
#include "managedsignedfile.hpp"

#define LFCSSIZE 272

union ALIGN(8) LfcsData_T {
	u8 Raw[LFCSSIZE];
	struct {
		u8 Signature[256];
		union {
			struct {
				u8 UnkOrPad1;
				u8 IsDevUnit;
				u8 UnkOrPad2[6];
				u64 Id;
			};
			u8 SignedData[LFCSSIZE-256];
		};
	};
};

static_assert(sizeof(LfcsData_T) == LFCSSIZE);
static_assert(offsetof(LfcsData_T, Raw) == 0);
static_assert(offsetof(LfcsData_T, Signature) == 0);
static_assert(offsetof(LfcsData_T, SignedData) == 256);
static_assert(offsetof(LfcsData_T, IsDevUnit) == 257);
static_assert(offsetof(LfcsData_T, Id) == 264);
static_assert(alignof(LfcsData_T) == 8);

static const FS_Path LfcsPaths[2] = {
	{PATH_ASCII, 27, "/sys/LocalFriendCodeSeed_A"},
	{PATH_ASCII, 27, "/sys/LocalFriendCodeSeed_B"}
};

static const FS_Path LfcsInvalidPaths[2] = {
	{PATH_ASCII, 35, "/sys/LocalFriendCodeSeed_A_invalid"},
	{PATH_ASCII, 35, "/sys/LocalFriendCodeSeed_B_invalid"}
};

static const PS_RSA_Context LfcsRsa[2] = {
	// Retail
	(PS_RSA_Context){
		.mod = {
			0xA3, 0x75, 0x9A, 0x35, 0x46, 0xCF, 0xA7, 0xFE, 0x30, 0xEC, 0x55, 0xA1, 0xB6, 0x4E, 0x08, 0xE9,
			0x44, 0x9D, 0x0C, 0x72, 0xFC, 0xD1, 0x91, 0xFD, 0x61, 0x0A, 0x28, 0x89, 0x75, 0xBC, 0xE6, 0xA9,
			0xB2, 0x15, 0x56, 0xE9, 0xC7, 0x67, 0x02, 0x55, 0xAD, 0xFC, 0x3C, 0xEE, 0x5E, 0xDB, 0x78, 0x25,
			0x9A, 0x4B, 0x22, 0x1B, 0x71, 0xE7, 0xE9, 0x51, 0x5B, 0x2A, 0x67, 0x93, 0xB2, 0x18, 0x68, 0xCE,
			0x5E, 0x5E, 0x12, 0xFF, 0xD8, 0x68, 0x06, 0xAF, 0x31, 0x8D, 0x56, 0xF9, 0x54, 0x99, 0x02, 0x34,
			0x6A, 0x17, 0xE7, 0x83, 0x74, 0x96, 0xA0, 0x5A, 0xAF, 0x6E, 0xFD, 0xE6, 0xBE, 0xD6, 0x86, 0xAA,
			0xFD, 0x7A, 0x65, 0xA8, 0xEB, 0xE1, 0x1C, 0x98, 0x3A, 0x15, 0xC1, 0x7A, 0xB5, 0x40, 0xC2, 0x3D,
			0x9B, 0x7C, 0xFD, 0xD4, 0x63, 0xC5, 0xE6, 0xDE, 0xB7, 0x78, 0x24, 0xC6, 0x29, 0x47, 0x33, 0x35,
			0xB2, 0xE9, 0x37, 0xE0, 0x54, 0xEE, 0x9F, 0xA5, 0x3D, 0xD7, 0x93, 0xCA, 0x3E, 0xAE, 0x4D, 0xB6,
			0x0F, 0x5A, 0x11, 0xE7, 0x0C, 0xDF, 0xBA, 0x03, 0xB2, 0x1E, 0x2B, 0x31, 0xB6, 0x59, 0x06, 0xDB,
			0x5F, 0x94, 0x0B, 0xF7, 0x6E, 0x74, 0xCA, 0xD4, 0xAB, 0x55, 0xD9, 0x40, 0x05, 0x8F, 0x10, 0xFE,
			0x06, 0x05, 0x0C, 0x81, 0xBB, 0x42, 0x21, 0x90, 0xBA, 0x4F, 0x5C, 0x53, 0x82, 0xE1, 0xE1, 0x0F,
			0xBC, 0x94, 0x9F, 0x60, 0x69, 0x5D, 0x13, 0x03, 0xAA, 0xE2, 0xE0, 0xC1, 0x08, 0x42, 0x4C, 0x20,
			0x0B, 0x9B, 0xAA, 0x55, 0x2D, 0x55, 0x27, 0x6E, 0x24, 0xE5, 0xD6, 0x04, 0x57, 0x58, 0x8F, 0xF7,
			0x5F, 0x0C, 0xEC, 0x81, 0x9F, 0x6D, 0x2D, 0x28, 0xF3, 0x10, 0x55, 0xF8, 0x3B, 0x76, 0x62, 0xD4,
			0xE4, 0xA6, 0x93, 0x69, 0xB5, 0xDA, 0x6B, 0x40, 0x23, 0xAF, 0x07, 0xEB, 0x9C, 0xBF, 0xA9, 0xC9
		},
		.small_exp = 0x10001,
		.rsa_bit_size = 2048,
		.is_full_exponent = 0,
		.padding = {0, 0, 0}
	},
	// Dev
	(PS_RSA_Context){
		.mod = {
			0x98, 0x59, 0x92, 0xB9, 0x65, 0x17, 0x68, 0xF5, 0xE7, 0x9F, 0x6A, 0xE5, 0x00, 0xCC, 0x57, 0x41,
			0x8E, 0x6B, 0x8D, 0xF9, 0xD4, 0x40, 0x9C, 0xEA, 0xFD, 0x96, 0x27, 0xC4, 0x25, 0xD6, 0x16, 0xC2,
			0xBC, 0x31, 0xCC, 0x23, 0x20, 0x6B, 0x0A, 0x49, 0x49, 0x31, 0xAA, 0x56, 0x7E, 0x6E, 0xAE, 0x1C,
			0x55, 0x06, 0x63, 0xA8, 0x72, 0x1D, 0xEB, 0x16, 0xC5, 0x1E, 0x00, 0x23, 0xE0, 0xBA, 0xBD, 0x26,
			0x16, 0x65, 0x19, 0xCE, 0xFE, 0xDB, 0x7F, 0x42, 0x99, 0xCE, 0xE5, 0xCE, 0xCC, 0xB2, 0xC5, 0x18,
			0x95, 0x7E, 0x4B, 0xDB, 0xB6, 0x56, 0x7D, 0x7D, 0x73, 0xED, 0x9C, 0xD5, 0x94, 0xCE, 0x26, 0x5D,
			0x8B, 0xCA, 0x66, 0x35, 0xCB, 0x60, 0xCA, 0xA3, 0xC1, 0xB6, 0x52, 0x21, 0x2F, 0xF4, 0xFF, 0x45,
			0xC7, 0x3B, 0xEB, 0xF4, 0xCD, 0xB6, 0x46, 0x3D, 0x07, 0x61, 0xD6, 0x61, 0xC3, 0x49, 0xAF, 0xBC,
			0xF5, 0x5B, 0xAC, 0x71, 0xCD, 0x06, 0x68, 0x46, 0x2F, 0xD1, 0x2E, 0x82, 0xC2, 0x3F, 0xEB, 0x57,
			0xCF, 0x62, 0x2E, 0xE7, 0xE2, 0x72, 0x1E, 0x73, 0x70, 0x7A, 0xF4, 0xF7, 0xB0, 0x1B, 0x8A, 0xDF,
			0xC2, 0x8D, 0x59, 0xC2, 0xD4, 0x40, 0xCC, 0xF1, 0x41, 0x50, 0xD3, 0x91, 0xA1, 0xF9, 0x5D, 0x9D,
			0x45, 0xBC, 0x5A, 0x74, 0x64, 0x1E, 0x22, 0xCE, 0x10, 0xD6, 0x2C, 0xE3, 0x5C, 0x7D, 0xD2, 0xB0,
			0xEB, 0xC5, 0x16, 0x64, 0x5C, 0x9A, 0xDC, 0x60, 0x93, 0xA4, 0x4E, 0x75, 0x00, 0xC1, 0xBE, 0x16,
			0xE9, 0xD7, 0xD3, 0x5F, 0xB8, 0x69, 0xE6, 0x2C, 0x14, 0x38, 0x51, 0xB3, 0x15, 0x15, 0x57, 0x7F,
			0xEA, 0xFE, 0x74, 0x50, 0xE3, 0x6D, 0x60, 0x27, 0xAE, 0x8C, 0x9C, 0xAA, 0xCA, 0xBE, 0x78, 0xEA,
			0xE4, 0xA4, 0x8A, 0x65, 0x5B, 0x39, 0x65, 0x0B, 0x7F, 0x0B, 0xDB, 0xC0, 0x73, 0xE0, 0x91, 0xBB
		},
		.small_exp = 0x10001,
		.rsa_bit_size = 2048,
		.is_full_exponent = 0,
		.padding = {0, 0, 0}
	}
};

struct ManagedLfcs_T {
	ManagedSignedFile_T<LfcsData_T,LfcsPaths,LfcsInvalidPaths,2,nullptr,nullptr,LfcsRsa> Manager;

	Result GetLfcsId(u64* id);
	void GetWholeLfcsNoChecks(void* data);

	Result SetSignature(const void* sig);
	Result SetData(const void* data, bool invalidateSignature);
	Result ResetToHardcodedId();
};

inline Result ManagedLfcs_T::GetLfcsId(u64* id) {
	if(!Manager.IsInit)
		return CFG_NOT_INITIALIZED;

	if(!Manager.WasChecked) {
		Manager.CheckSignature();
		Manager.WasChecked = true;
	}

	if(!Manager.IsSigned)
		return CFG_BAD_SIGNATURE;

	*id = Manager.Data.Id;

	return 0;
}

inline void ManagedLfcs_T::GetWholeLfcsNoChecks(void* data) {
	memcpy(data, Manager.Data.Raw, sizeof(Manager.Data.Raw));
}

inline Result ManagedLfcs_T::SetSignature(const void* sig) {
	if(!Manager.IsInit)
		return CFG_NOT_INITIALIZED;

	memcpy(Manager.Data.Signature, sig, sizeof(Manager.Data.Signature));

	Manager.WasChecked = false; // cfg didnt check later again, invalidated signed but not to check it again later, we do
	Manager.IsSigned = false;

	return 0;
}

inline Result ManagedLfcs_T::SetData(const void* data, bool invalidateSignature) {
	if(!Manager.IsInit)
		return CFG_NOT_INITIALIZED;

	memcpy(Manager.Data.SignedData, data, sizeof(Manager.Data.SignedData));

	if(invalidateSignature) {
		Manager.WasChecked = false; // same as SetSignature
		Manager.IsSigned = false;
	}

	return 0;
}

inline Result ManagedLfcs_T::ResetToHardcodedId() {
	if(!Manager.IsInit)
		return CFG_NOT_INITIALIZED;

	ALIGN(8) u8 newdata[16];

	Result res = psInit();
	if(R_SUCCEEDED(res)) {
		res = PS_GetLocalFriendCodeSeed((u64*)&newdata[8]);
		psExit();
	}
	Err_Panic(res);

	*(u64*)&newdata[0] = 0LLU;
	newdata[1] = osEnvInfoIsDev() ? 1 : 0;

	return SetData(&newdata[0], true);
}

ManagedLfcs_T ManagedLfcsFile = {
	.Manager = (ManagedSignedFile_T<LfcsData_T,LfcsPaths,LfcsInvalidPaths,2,nullptr,nullptr,LfcsRsa>){
		.Data = (LfcsData_T){},
		.IsInit = false,
		.IsSigned = false,
		.WasChecked = false,
		.Overriden = false,
		.Index = -1
	}
};

extern "C" void Lfcs_Init() {
	ManagedLfcsFile.Manager.Init();
}

extern "C" void Lfcs_SaveToNextSlot() {
	ManagedLfcsFile.Manager.SaveToNextSlot();
}

extern "C" Result Lfcs_CheckSignature() {
	return ManagedLfcsFile.Manager.CheckSignature();
}

extern "C" Result Lfcs_GetId(u64* id) {
	return ManagedLfcsFile.GetLfcsId(id);
}

extern "C" Result Lfcs_GetWholeData(void* data, size_t size) {
	if(size < sizeof(ManagedLfcsFile.Manager.Data)) // size checks were not originally done
		return CFG_INVALID_SIZE;

	ManagedLfcsFile.GetWholeLfcsNoChecks(data);
	return 0;
}

extern "C" Result Lfcs_SetSignature(const void* sig, size_t size) {
	if(size < sizeof(ManagedLfcsFile.Manager.Data.Signature)) // size checks were not originally done
		return CFG_INVALID_SIZE;

	return ManagedLfcsFile.SetSignature(sig);
}

extern "C" Result Lfcs_SetData(const void* data, size_t size, bool reset_hardcoded) {
	if(reset_hardcoded)
		return ManagedLfcsFile.ResetToHardcodedId();

	if(size < sizeof(ManagedLfcsFile.Manager.Data.SignedData)) // size checks were not originally done
		return CFG_INVALID_SIZE;

	return ManagedLfcsFile.SetData(data, false); // I guess for IPC data, we dont invalidate signature?
}
