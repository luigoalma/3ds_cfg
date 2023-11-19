extern "C" {
	#include <3ds/errf.h>
	#include <3ds/fs.h>
	#include <3ds/i2c.h>
	#include <3ds/os.h>
	#include <3ds/ps.h>
	#include <3ds/result.h>
	#include <3ds/srv.h>
	#include <3ds/svc.h>
	#include <3ds/types.h>
}
#include <utils/sha256.h>
#include <utils/hmacsha256.h>
#include <utils/endian.h>
#include <cfg.h>
#include <internal_types.h>
#include <hwcal_dummy_defaults.h>
#include <err.h>
#include <cstddef>

#define FS_NOT_MOUNTED           MAKERESULT(RL_STATUS, RS_NOTFOUND, RM_FS, 101)
#define UTIL_INVALID_COMBINATION MAKERESULT(RL_USAGE, RS_INVALIDARG, RM_UTIL, RD_INVALID_COMBINATION)

#define CONFIGSIZE 32768

static const FS_Path ConfigPath = {PATH_ASCII, 8, "/config"};

static const FS_Path[2] LfcsPaths = {
	{PATH_ASCII, 27, "/sys/LocalFriendCodeSeed_A"},
	{PATH_ASCII, 27, "/sys/LocalFriendCodeSeed_B"}
};

static const FS_Path[2] LfcsInvalidPaths = {
	{PATH_ASCII, 35, "/sys/LocalFriendCodeSeed_A_invalid"},
	{PATH_ASCII, 35, "/sys/LocalFriendCodeSeed_B_invalid"}
};

static const FS_Path[2] SecInfoPaths = {
	{PATH_ASCII, 18, "/sys/SecureInfo_A"},
	{PATH_ASCII, 18, "/sys/SecureInfo_B"}
};

static const FS_Path[2] SecInfoInvalidPaths = {
	{PATH_ASCII, 26, "/sys/SecureInfo_A_invalid"},
	{PATH_ASCII, 26, "/sys/SecureInfo_B_invalid"}
};

static const FS_Path SecInfoOverride = {PATH_ASCII, 18, "/sys/SecureInfo_C"};
static const FS_Path SecInfoOverrideTemp = {PATH_ASCII, 23, "/sys/SecureInfo_C_temp"};

static const FS_Path[2] HwcalPaths = {
	{PATH_ASCII, 16, "/sys/HWCAL0.dat"},
	{PATH_ASCII, 16, "/sys/HWCAL1.dat"}
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

static const PS_RSA_Context SecInfoRsa[2] = {
	// Retail
	(PS_RSA_Context){
		.mod = {
			0xB1, 0x79, 0x1A, 0x6D, 0x1E, 0xAD, 0xD4, 0x29, 0xBA, 0x89, 0xA1, 0xCD, 0x43, 0x36, 0x30, 0x17,
			0x4B, 0xC6, 0x87, 0x30, 0xC5, 0xE7, 0x05, 0x60, 0x19, 0x7B, 0x50, 0xD8, 0xC4, 0x54, 0x67, 0x10,
			0xA6, 0xE8, 0xA1, 0x01, 0xBC, 0x2C, 0xEB, 0x03, 0x76, 0xF0, 0x05, 0xC7, 0x0C, 0xE0, 0xB6, 0xD6,
			0xDF, 0xFD, 0x26, 0xDF, 0x33, 0x46, 0x8B, 0xDB, 0xB2, 0x39, 0x1E, 0x7E, 0xC0, 0x1A, 0xA1, 0xA5,
			0xA0, 0x91, 0xE8, 0x07, 0xDA, 0x37, 0x86, 0x76, 0xBA, 0x39, 0x0A, 0x25, 0x42, 0x9D, 0x59, 0x61,
			0xE1, 0x61, 0xD4, 0x04, 0x85, 0xA7, 0x4B, 0xB2, 0x01, 0x86, 0xBE, 0xB1, 0x1A, 0x35, 0x72, 0xC1,
			0xC2, 0xEA, 0x28, 0xAB, 0x7A, 0x10, 0x15, 0x32, 0x5C, 0x9E, 0x71, 0x2B, 0x7D, 0xF9, 0x65, 0xEA,
			0xE6, 0xC6, 0xFB, 0x8B, 0xAE, 0xD7, 0x6C, 0x2A, 0x94, 0xA6, 0xC5, 0xEC, 0xE4, 0x0E, 0xAF, 0x98,
			0x7E, 0x06, 0xF2, 0x0F, 0x88, 0x4F, 0xD2, 0x06, 0x35, 0xA4, 0x76, 0xE9, 0xF7, 0x0A, 0xBA, 0x5C,
			0x5B, 0x14, 0x61, 0x52, 0x00, 0x54, 0x04, 0x45, 0x93, 0xE4, 0x68, 0x27, 0x04, 0x35, 0x35, 0x5A,
			0xAD, 0x58, 0x09, 0xD1, 0x19, 0x3F, 0x5A, 0x07, 0x28, 0xD6, 0xDB, 0x6B, 0x55, 0x1F, 0x77, 0x94,
			0x5D, 0xC3, 0xBE, 0x6F, 0xAE, 0x5B, 0xCC, 0x08, 0x63, 0xE4, 0x76, 0xDF, 0xA2, 0x9B, 0x36, 0xEA,
			0x85, 0x34, 0x03, 0xE6, 0x16, 0xEA, 0xA9, 0x05, 0xE0, 0x7F, 0x3A, 0x3E, 0x7E, 0x70, 0x77, 0xCF,
			0x16, 0x6A, 0x61, 0xD1, 0x7E, 0x4D, 0x35, 0x4C, 0x74, 0x44, 0x85, 0xD4, 0xF6, 0x7B, 0x0E, 0xEE,
			0x32, 0xF1, 0xC2, 0xD5, 0x79, 0x02, 0x48, 0xE9, 0x62, 0x1A, 0x33, 0xBA, 0xA3, 0x9B, 0x02, 0xB0,
			0x22, 0x94, 0x05, 0x7F, 0xF6, 0xB4, 0x38, 0x88, 0xE3, 0x01, 0xE5, 0x5A, 0x23, 0x7C, 0x9C, 0x0B
		},
		.small_exp = 0x10001,
		.rsa_bit_size = 2048,
		.is_full_exponent = 0,
		.padding = {0, 0, 0}
	},
	// Dev
	(PS_RSA_Context){
		.mod = {
			0xB1, 0xAA, 0x6C, 0x55, 0x3C, 0xA8, 0x4D, 0x83, 0x3C, 0x2E, 0x97, 0x56, 0xB5, 0x2B, 0xD3, 0x70,
			0x1D, 0x0F, 0xD4, 0xD1, 0xEE, 0xF1, 0x71, 0xF4, 0xFD, 0x95, 0x96, 0x1D, 0x52, 0xBF, 0x75, 0x63,
			0xB8, 0x9D, 0x2F, 0xF5, 0xF8, 0x15, 0xE4, 0x0A, 0x76, 0xE2, 0x0F, 0x55, 0x11, 0x63, 0xE9, 0xE9,
			0x85, 0x68, 0x41, 0x5A, 0x28, 0x31, 0x22, 0xE1, 0x99, 0xDE, 0xEC, 0x77, 0x17, 0x12, 0xC6, 0x78,
			0xDA, 0x0B, 0xB4, 0xDD, 0x50, 0xF3, 0x0C, 0x61, 0x5F, 0xA5, 0x7D, 0xEA, 0x74, 0xD7, 0x1D, 0x11,
			0x87, 0xBF, 0xEB, 0xC3, 0x33, 0xD7, 0x35, 0x0E, 0xDD, 0x45, 0x98, 0x1B, 0xEF, 0x6F, 0xB9, 0x73,
			0xE8, 0x35, 0x9C, 0xE5, 0xB0, 0xC8, 0xFF, 0x5C, 0x42, 0x9B, 0xA7, 0x90, 0xAE, 0xF9, 0xB7, 0x62,
			0x56, 0x04, 0xB1, 0xB0, 0xB2, 0x44, 0xD6, 0x86, 0x34, 0xE6, 0x2F, 0x79, 0x4D, 0x9C, 0xAF, 0xB5,
			0x9A, 0x3B, 0xFA, 0xC8, 0x81, 0x03, 0x96, 0x6F, 0x9B, 0xDC, 0x87, 0x8B, 0x32, 0x3C, 0x37, 0xEB,
			0xCD, 0x21, 0xC8, 0xB9, 0x27, 0x6F, 0xFC, 0xC8, 0x47, 0x02, 0xFF, 0x87, 0xD1, 0xD0, 0x2F, 0x64,
			0xD4, 0x36, 0xD4, 0x85, 0x01, 0xAD, 0x70, 0xF3, 0xA2, 0xB1, 0x0D, 0x13, 0xEF, 0x55, 0x94, 0xA0,
			0x23, 0x81, 0x71, 0xF9, 0x4A, 0xD2, 0x01, 0x58, 0x90, 0x60, 0x13, 0xFB, 0x6D, 0xB6, 0x18, 0x38,
			0x31, 0xDF, 0x11, 0x44, 0xB5, 0x96, 0x49, 0xA3, 0x53, 0x08, 0xB2, 0x64, 0xC1, 0xEF, 0x11, 0x9E,
			0x1D, 0x17, 0x17, 0x9A, 0x87, 0x44, 0x17, 0x3A, 0x73, 0xA2, 0xF7, 0xD9, 0x96, 0x1A, 0x79, 0xE1,
			0xF9, 0x86, 0x6E, 0xEE, 0x6F, 0xBB, 0xD2, 0xDC, 0xCF, 0x3B, 0x0D, 0xC4, 0xE2, 0x76, 0xD1, 0xD0,
			0xC0, 0x37, 0x98, 0xBE, 0xC1, 0xBC, 0xD9, 0x64, 0x6F, 0xC4, 0xCB, 0x46, 0xBB, 0x5F, 0xF5, 0x55
		},
		.small_exp = 0x10001,
		.rsa_bit_size = 2048,
		.is_full_exponent = 0,
		.padding = {0, 0, 0}
	}
};

static ALIGN(4) const u8 HwcalHMACKey[32] = {
	0x0D, 0x36, 0xF9, 0xC8, 0xEA, 0xED, 0x15, 0xA7, 0xB9, 0x22, 0xE3, 0xC7, 0x70, 0xE7, 0x53, 0x48,
	0xDA, 0x2D, 0x57, 0x35, 0xC2, 0x59, 0x9F, 0x93, 0x5D, 0xE1, 0x81, 0x4D, 0xE6, 0x65, 0x40, 0x8B
};

struct ConfigBlkEntry_T
{
	u32 Id;
	union {
		u32 RelOffset;
		u8 Data[4];
	};
	u16 DataSize;
	u16 Flags;

	void* GetPtr(void* BasePtr);
};

static_assert(sizeof(ConfigBlkEntry_T) == 12);
static_assert(offsetof(ConfigBlkEntry_T, Id) == 0);
static_assert(offsetof(ConfigBlkEntry_T, RelOffset) == 4);
static_assert(offsetof(ConfigBlkEntry_T, Data) == 4);
static_assert(offsetof(ConfigBlkEntry_T, DataSize) == 8);
static_assert(offsetof(ConfigBlkEntry_T, Flags) == 10);
static_assert(alignof(ConfigBlkEntry_T) == 4);

union ALIGN(8) ConfigData_T {
	u8 Raw[CONFIGSIZE];
	struct {
		u16 TotalEntries;
		u16 DataEntryOffset;
		ConfigBlkEntry_T BlkEntries[];
	};

	ConfigBlkEntry_T* FindBlkId(u32 Id);
	bool SanityCheck() const; // Something cfg does not do.
	void Reset();
	void Save() const;
	void Load(); // cfg doesn't care if load fails
	void SaveFixData() const;
	bool LoadFixData();
	void DeleteAndReset();
	Result GetBlkPtr(void*& ptr, u32 blkId, size_t size, CFG_BlkFlags flags, CFG_BlkFlags bitmask);
	Result GetBlkPtrForReading(const void*& ptr, u32 blkId, size_t size, CFG_BlkFlags flags);
	Result GetBlkPtrForWriting(void*& ptr, u32 blkId, size_t size, CFG_BlkFlags flags);
	Result CreateBlk(void*& ptr, u32 blkId, size_t size, CFG_BlkFlags flags);
	Result ReadBlk(void* ptr, u32 blkId, size_t size, bool system);
	Result WriteBlk(const void* ptr, u32 blkId, size_t size, bool system);
};

static_assert(sizeof(ConfigData_T) == CONFIGSIZE);
static_assert(offsetof(ConfigData_T, TotalEntries) == 0);
static_assert(offsetof(ConfigData_T, DataEntryOffset) == 2);
static_assert(offsetof(ConfigData_T, BlkEntries) == 4);
static_assert(alignof(ConfigData_T) == 8);

// override is a special feature to keep the SecureInfo_C consistency from Luma
template<typename T, const FS_Path* Paths, const FS_Path* InvalidPaths, int PathCount, const FS_Path* OverridePath, const FS_Path* OverridePathTemp, const PS_RSA_Context RsaPair[2]>
struct ManagedSignedFile_T {
	T Data;
	bool IsInit:1;
	bool IsSigned:1;
	bool WasChecked:1;
	bool Overriden:1;
	int Index;

	bool HasDefinedOverride();
	Result CheckSignature();
	bool OpenDeleteFail(Handle& file, FS_Path& rwNandPath);
	void Init();
	void SaveToNextSlot();
};

namespace SystemSave {
	static FS_Archive NormalFSArchive = 0LLU;
	static FS_Archive FixDataFSArchive = 0LLU;
	const static FS_SystemSaveDataInfo NormalFSInfo = {MEDIATYPE_NAND, 0, 0, 0x10017};
	const static FS_SystemSaveDataInfo FixDataFSInfo = {MEDIATYPE_NAND, 1, 0, 0x10017};
	const static FS_Path NormalFSPath = {PATH_BINARY, sizeof(NormalFSInfo), reinterpret_cast<const void*>(&NormalFSInfo)};
	const static FS_Path FixDataFSPath = {PATH_BINARY, sizeof(FixDataFSInfo), reinterpret_cast<const void*>(&FixDataFSInfo)};

	static Result Create(FS_SystemSaveDataInfo& FSInfo) {
		// Same creation parameters for both normal syssave and fixdata syssave
		return FSUSER_CreateSystemSaveData(FSInfo, 0x40000, 0x1000, 10, 10, 11, 11, true);
	}

	static Result Delete(FS_SystemSaveDataInfo& FSInfo) {
		return FSUSER_DeleteSystemSaveData(FSInfo);
	}

	static Result SimpleOpen(FS_Archive& FSArchive, FS_Path& FSPath) {
		return FSUSER_OpenArchive(&FSArchive, ARCHIVE_SYSTEM_SAVEDATA, FSPath);
	}

	static Result Close(FS_Archive& FSArchive) {
		if(!FSArchive) return 0;
		Result res = FSUSER_CloseArchive(FSArchive);
		FSArchive = 0LLU;
		return res;
	}

	static Result Format(FS_Archive& FSArchive, FS_SystemSaveDataInfo& FSInfo, FS_Path& FSPath) {
		Close(FSArchive);
		Result res = Delete(FSInfo);
		if(R_FAILED(res) && !R_MODULEDESCRANGE(res, RM_FS, 100, 179)) return res;
		if(R_FAILED(res = Create(FSInfo))) return res;
		if(R_FAILED(res = SimpleOpen(FSArchive, FSPath))) return res;
		return 0;
	}

	static Result OpenWithCreation() {
		if(NormalFSArchive) return 0;
		Result res = SimpleOpen(NormalFSArchive, NormalFSPath);
		if(R_SUCCEEDED(res)) return res;
		if(R_MODULEDESCRANGE(res, RM_FS, 100, 179) || R_MODULEDESCRANGE(res, RM_FS, 340, 359)) {
			if(R_FAILED(res = Create(NormalFSInfo))) return res;
			res = SimpleOpen(NormalFSArchive, NormalFSPath);
		} else if(R_MODULEDESCRANGE(res, RM_FS, 360, 399)) {
			res = Format(NormalFSArchive, NormalFSInfo, NormalFSPath);
		}
		return res;
	}

	// fixdata save is treated more loosely
	static Result FixDataOpenWithCreation() {
		if(FixDataFSArchive) return 0;
		Result res = SimpleOpen(FixDataFSArchive, FixDataFSPath);
		if(R_SUCCEEDED(res)) return res;
		if(!R_MODULEDESCRANGE(res, RM_FS, 100, 179)) {
			Delete(FixDataFSInfo); // no check
		if(R_FAILED(res = Create(FixDataFSInfo))) return res;
		if(R_FAILED(res = SimpleOpen(FixDataFSArchive, FixDataFSPath))) {
			Delete(FixDataFSInfo); // no check
		}
		return res;
	}

	static Result Commit(FS_Archive& FSArchive) {
		if(!FSArchive) return FS_NOT_MOUNTED; //FS not mounted
		char a[2];
		return FSUSER_ControlArchive(FSArchive, ARCHIVE_ACTION_COMMIT_SAVE_DATA, &a[0], 1, &a[1], 1);
	}
};

namespace NandAccess {
	static FS_Archive NandRWFSArchive = 0LLU;
	static FS_Archive NandROFSArchive = 0LLU;
	const static FS_Path NullPath = {PATH_EMPTY, 1, reinterpret_cast<const void*>(&NullPath.data)};

	static void Open() {
		if(!NandRWFSArchive) FSUSER_OpenArchive(&NandRWFSArchive, ARCHIVE_NAND_RW, NullPath);
		if(!NandROFSArchive) FSUSER_OpenArchive(&NandROFSArchive, ARCHIVE_NAND_RO, NullPath);
	}
}

extern "C" Result Cfg_FormatSysSave() {
	return ::SystemSave::Format(
		::SystemSave::NormalFSArchive,
		::SystemSave::NormalFSInfo,
		::SystemSave::NormalFSPath
	);
}

extern "C" Result Cfg_OpenSysSave() {
	return ::SystemSave::OpenWithCreation();
}

extern "C" Result Cfg_CommitSysSave() {
	return ::SystemSave::Commit(::SystemSave::NormalFSArchive);
}

extern "C" void Cfg_OpenNandAccess() {
	::NandAccess::Open();
}

extern "C" void Cfg_DeleteFixData() {
	Err_FailedThrow(::SystemSave::Delete(::SystemSave::FixDataFSInfo))
}

inline void* ConfigBlkEntry_T::GetPtr(void* BasePtr) {
	if(DataSize <= 4) return reinterpret_cast<void*>(&Data[0]);
	return reinterpret_cast<void*>(reinterpret_cast<u8*>(BasePtr) + RelOffset);
}

inline ConfigBlkEntry_T* ConfigData_T::FindBlkId(u32 Id) {
	ConfigBlkEntry_T* ptr = nullptr;

	for(int i = 0; i < TotalEntries; ++i) {
		if(BlkEntries[i].Id == id) {
			ptr = &BlkEntries[i];
		}
	}

	return ptr;
}

// Comment note: comment written September 23rd, 2023
// importing a config from citra to real save would fail sanity check
// citra's implementation is incorrect as of this writing
bool ConfigData_T::SanityCheck() const {
	if(DataEntryOffset > CONFIGSIZE) // beyond size
		return false;

	uptr header_roof = reinterpret_cast<uptr>(&BlkEntries[TotalEntries]) - reinterpret_cast<uptr>(&Raw[0]);
	if(header_roof > DataEntryOffset) // overlap data
		return false;

	u32 offset = CONFIGSIZE;
	bool pass = true;

	for(int i = 0; i < TotalEntries; ++i) {
		const ConfigBlkEntry_T& blk = BlkEntries[i];

		// search for repetition
		for(int j = i+1; j < TotalEntries; ++j) {
			if(blk.Id == BlkEntries[j].Id) {
				pass = false;
				break;
			}
		}
		if(!pass)
			break;

		// blks with 4 or less bytes have data self contained
		// in the entry header itself
		if(blk.DataSize <= 4)
			continue;

		offset -= blk.DataSize;
		if(offset != blk.RelOffset) {
			pass = false;
			break;
		}
	}

	return pass;
}

inline void ConfigData_T::Reset() {
	TotalEntries = 0;
	DataEntryOffset = CONFIGSIZE;
}

inline void ConfigData_T::Save() const {
	Handle file = 0;
	u32 filesize = 0;
	Result res = 0;

	res = FSUSER_OpenFile(&file, ::SystemSave::NormalFSArchive, ConfigPath, FS_OPEN_WRITE | FS_OPEN_CREATE, 0);
	if(R_FAILED(res))
		return;

	res = FSFILE_Write(file, &filesize, 0LLU, &Raw[0], CONFIGSIZE, FS_WRITE_FLUSH);

	FSFILE_Close(file);
	svcCloseHandle(file);

	if(R_SUCCEEDED(res)) 
		::SystemSave::Commit(::SystemSave::NormalFSArchive);
}

inline void ConfigData_T::Load() {
	Handle file = 0;
	u32 filesize = 0;
	Result res = 0;

	res = FSUSER_OpenFile(&file, ::SystemSave::NormalFSArchive, ConfigPath, FS_OPEN_READ, 0);

	if(R_SUCCEEDED(res))
		res = FSFILE_Read(file, &filesize, 0LLU, &Raw[0], CONFIGSIZE);

	FSFILE_Close(file);
	svcCloseHandle(file);

	if(R_SUCCEEDED(res) && filesize == CONFIGSIZE && SanityCheck())
		return;

	Reset(); // fail? reset
}

inline void ConfigData_T::SaveFixData() const {
	Handle file = 0;
	u32 filesize = 0;
	Result res = 0;

	Err_FailedThrow(::SystemSave::FixDataOpenWithCreation());
	FSUSER_DeleteFile(::SystemSave::FixDataFSArchive, ConfigPath);

	res = FSUSER_OpenFile(&file, ::SystemSave::FixDataFSArchive, ConfigPath, FS_OPEN_WRITE | FS_OPEN_CREATE, 0);

	if(R_SUCCEEDED(res)) {
		res = FSFILE_Write(file, &filesize, 0LLU, &Raw[0], CONFIGSIZE, FS_WRITE_FLUSH);

		FSFILE_Close(file);
		svcCloseHandle(file);
	}

	if(R_SUCCEEDED(res))
		res = ::SystemSave::Commit(::SystemSave::FixDataFSArchive);

	if(R_FAILED(res)) {
		::SystemSave::Close(::SystemSave::FixDataFSArchive);
		::SystemSave::Delete(::SystemSave::FixDataFSInfo);
		Err_Throw(res);
	}

	::SystemSave::Close(::SystemSave::FixDataFSArchive);
}

inline bool ConfigData_T::LoadFixData() {
	Handle file = 0;
	u32 filesize = 0;
	Result res = 0;
	bool pass = false;

	if(R_FAILED(::SystemSave::SimpleOpen(FixDataFSArchive, FixDataFSPath)))
		return false;

	res = FSUSER_OpenFile(&file, ::SystemSave::FixDataFSArchive, ConfigPath, FS_OPEN_READ, 0)

	if(R_SUCCEEDED(res)) {
		res = FSFILE_Read(file, &filesize, 0LLU, &Raw[0], CONFIGSIZE);

		FSFILE_Close(file);
		svcCloseHandle(file);

		if(R_SUCCEEDED(res) && filesize == CONFIGSIZE && SanityCheck()) {
			pass = true;
			Save();
		}
	}

	::SystemSave::Close(::SystemSave::FixDataFSArchive);

	if(pass) // if successful read, delete must happen it seems?
		Err_FailedThrow(::SystemSave::Delete(::SystemSave::FixDataFSInfo));
	else // else, ignore any issues deleting it??
		::SystemSave::Delete(::SystemSave::FixDataFSInfo);

	return pass;
}

inline void ConfigData_T::DeleteAndReset() {
	FSUSER_DeleteFile(::SystemSave::NormalFSArchive, ConfigPath);
	TotalEntries = 0;
	DataEntryOffset = CONFIGSIZE;
}

inline Result ConfigData_T::GetBlkPtr(void*& ptr, u32 blkId, size_t size, CFG_BlkFlags accessFlags, CFG_BlkFlags bitmask) {
	ptr = nullptr;

	if(size == 0 || size > CONFIGSIZE)
		return CFG_INVALID_SIZE;

	ConfigBlkEntry_T* blk = FindBlkId(blkId);

	if(!blk)
		return CFG_NOT_FOUND;

	if(!(blk->Flags & accessFlags & bitmask))
		return CFG_NOT_AUTHORIZED;

	if(blk->DataSize != size)
		return CFG_INVALID_SIZE;

	ptr = blk->GetPtr(&Raw[0]);

	return 0;
}

inline Result ConfigData_T::GetBlkPtrForReading(const void*& ptr, u32 blkId, size_t size, CFG_BlkFlags accessFlags) {
	return GetBlkPtr(ptr, blkId, size, accessFlags, BLK_READ_PERM_BITMASK);
}

inline Result ConfigData_T::GetBlkPtrForWriting(void*& ptr, u32 blkId, size_t size, CFG_BlkFlags accessFlags) {
	return GetBlkPtr(ptr, blkId, size, accessFlags, BLK_READ_WRITE_BITMASK);
}

inline Result ConfigData_T::CreateBlk(void*& ptr, u32 blkId, size_t size, CFG_BlkFlags flags) {
	ptr = nullptr;

	if(size == 0 || size > CONFIGSIZE)
		return CFG_INVALID_SIZE;

	s32 new_header_roof = reinterpret_cast<uptr>(&BlkEntries[TotalEntries + 1]) - reinterpret_cast<uptr>(&Raw[0]);
	s32 new_data_bottom = (s32)DataEntryOffset - ((size <= 4) ? 0 : size);

	if(new_header_roof > new_data_bottom) // overlap data
		return CFG_OUT_OF_MEMORY;

	if(FindBlkId(blkId))
		return CFG_ALREADY_EXISTS;

	ConfigBlkEntry_T& newblk = BlkEntries[TotalEntries];

	newblk.Id = blkId;
	if(size > 4) newblk.RelOffset = (u16)new_data_bottom;
	newblk.DataSize = size;
	newblk.Flags = flags;

	++TotalEntries;
	DataEntryOffset = (u16)new_data_bottom;

	ptr = newblk.GetPtr(&Raw[0]);

	return 0;
}

inline Result ConfigData_T::UpdateBlkFlags(u32 blkId, CFG_BlkFlags flags) {
	ConfigBlkEntry_T* blk = FindBlkId(blkId);

	if(!blk)
		return CFG_NOT_FOUND;

	blk->Flags = flags;

	return 0;
}

inline Result ConfigData_T::ReadBlk(void* ptr, u32 blkId, size_t size, bool system) {
	const void* _ptr;

	Result res = ConfigSave.GetBlkPtrForReading(_ptr, blkId, size, system ? BLK_SYSTEM_READ_PERM : BLK_USER_READ_PERM);
	if(R_FAILED(res)) return res;

	if(size == 4 && (reinterpret_cast<uptr>(ptr) & 0x3) == 0) { // size 4, aligned by 4 
		*reinterpret_cast<u32*>(ptr) = *reinterpret_cast<const u32*>(_ptr);
	} else if((reinterpret_cast<uptr>(ptr) & 0x1) == 0) {
		if(size == 3) {
			*reinterpret_cast<u16*>(ptr) = *reinterpret_cast<const u16*>(_ptr);
			*(reinterpret_cast<u8*>(ptr)+2) = *(reinterpret_cast<const u8*>(_ptr)+2);
		} else if(size == 2) {
			*reinterpret_cast<u16*>(ptr) = *reinterpret_cast<const u16*>(_ptr);
		}
	} else if(size == 1) {
		*reinterpret_cast<u8*>(ptr) = *reinterpret_cast<const u8*>(_ptr);
	} else {
		memcpy(ptr, _ptr, size);
	}

	return 0;
}

inline Result ConfigData_T::WriteBlk(const void* ptr, u32 blkId, size_t size, bool system) {
	void* _ptr;

	Result res = ConfigSave.GetBlkPtrForWriting(_ptr, blkId, size, system ? BLK_SYSTEM_WRITE_PERM : BLK_USER_WRITE_PERM);
	if(R_FAILED(res)) return res;

	// returned blk pointer for size <= 4 should be always 4 byte aligned with a space of 4 bytes
	// however argument pointer may be a different case in alignment, we check
	if(size <= 4) {
		if((reinterpret_cast<uptr>(ptr) & 0x3) == 0) {
			*reinterpret_cast<u32*>(_ptr) = *reinterpret_cast<const u32*>(ptr);
		} else if((reinterpret_cast<uptr>(ptr) & 0x1) == 0) {
			*reinterpret_cast<u16*>(_ptr) = *reinterpret_cast<const u16*>(ptr);
			if(size > 2) *(reinterpret_cast<u16*>(_ptr)+1) = *(reinterpret_cast<const u16*>(ptr)+1);
		} else {
			*reinterpret_cast<u8*>(_ptr) = *reinterpret_cast<const u8*>(ptr);
			if(size > 1) *(reinterpret_cast<u8*>(_ptr)+1) = *(reinterpret_cast<const u8*>(ptr)+1);
			if(size > 2) *(reinterpret_cast<u8*>(_ptr)+2) = *(reinterpret_cast<const u8*>(ptr)+2);
			if(size > 3) *(reinterpret_cast<u8*>(_ptr)+3) = *(reinterpret_cast<const u8*>(ptr)+3);
		}
	} else {
		memcpy(_ptr, ptr, size);
	}

	return 0;
}

ConfigData_T ConfigSave = {.TotalEntries = 0, .DataEntryOffset = CONFIGSIZE};

extern "C" void Cfg_AtBootConfigLoad() {
	// cfg actually always loads normal save first, then tries to read fixdata
	// but why spend loading on normal save if fixdata will succeed?
	// we will be trying both anyway
	if(!ConfigSave.LoadFixData())
		ConfigSave.Load();
}

extern "C" void Cfg_SaveToFixData() {
	ConfigSave.SaveFixData();
}

extern "C" void Cfg_SaveConfig() {
	ConfigSave.Save();
}

extern "C" void Cfg_DeleteAndResetConfig() {
	ConfigSave.DeleteAndReset();
}

extern "C" Result Cfg_User_ReadBlk(void* ptr, u32 blkId, size_t size) {
	return ConfigSave.ReadBlk(ptr, blkId, size, false);
}

extern "C" Result Cfg_System_ReadBlk(void* ptr, u32 blkId, size_t size) {
	return ConfigSave.ReadBlk(ptr, blkId, size, true);
}

extern "C" Result Cfg_System_WriteBlk(const void* ptr, u32 blkId, size_t size) {
	return ConfigSave.WriteBlk(ptr, blkId, size, true);
}

extern "C" Result Cfg_System_WriteBlkWithPublish(const void* ptr, u32 blkId, size_t size) {
	if(blkId == 0x50001 || blkId == 0x50009)
		srvPublishToSubscriber(0x109, 0);
	return ConfigSave.WriteBlk(ptr, blkId, size, true);
}

extern "C" Result Cfg_System_GetBlkPtr(void** ptr, u32 blkId, size_t size) {
	*ptr = nullptr;

	void* _ptr;

	Result res = ConfigSave.GetBlkPtrForWriting(_ptr, blkId, size, BLK_SYSTEM_WRITE_PERM);
	if(R_FAILED(res)) return res;

	*ptr = _ptr;

	return 0;
}

extern "C" Result Cfg_CreateBlk(void** ptr, u32 blkId, size_t size, CFG_BlkFlags flags) {
	if(blkId != BLK_RO_SYSTEM && blkId != BLK_RO_ANY && blkId != BLK_RW_SYSTEM && blkId != BLK_RW_ANY)
		return UTIL_INVALID_COMBINATION; // util??
	return ConfigSave.CreateBlk(*ptr, blkId, size, flags);
}

extern "C" Result Cfg_CreateBlkWithData(const void* ptr, u32 blkId, size_t size, CFG_BlkFlags flags) {
	void* _ptr;
	Result res = Cfg_CreateBlk(&_ptr, blkId, size, flags);
	if(R_FAILED(res)) return res;

	if(size <= 4) {
		if((reinterpret_cast<uptr>(ptr) & 0x3) == 0) {
			*reinterpret_cast<u32*>(_ptr) = *reinterpret_cast<const u32*>(ptr);
		} else if((reinterpret_cast<uptr>(ptr) & 0x1) == 0) {
			*reinterpret_cast<u16*>(_ptr) = *reinterpret_cast<const u16*>(ptr);
			if(size > 2) *(reinterpret_cast<u16*>(_ptr)+1) = *(reinterpret_cast<const u16*>(ptr)+1);
		} else {
			*reinterpret_cast<u8*>(_ptr) = *reinterpret_cast<const u8*>(ptr);
			if(size > 1) *(reinterpret_cast<u8*>(_ptr)+1) = *(reinterpret_cast<const u8*>(ptr)+1);
			if(size > 2) *(reinterpret_cast<u8*>(_ptr)+2) = *(reinterpret_cast<const u8*>(ptr)+2);
			if(size > 3) *(reinterpret_cast<u8*>(_ptr)+3) = *(reinterpret_cast<const u8*>(ptr)+3);
		}
	} else {
		memcpy(_ptr, ptr, size);
	}

	return 0;
}

template<typename T, const FS_Path* Paths, const FS_Path* InvalidPaths, int PathCount, const FS_Path* OverridePath, const FS_Path* OverridePathTemp, const PS_RSA_Context RsaPair[2]>
inline bool ManagedSignedFile_T<T,Paths,InvalidPaths,PathCount,OverridePath,OverridePathTemp,RsaPair>::HasDefinedOverride() {
	return OverridePath != nullptr && OverridePathTemp != nullptr;
}

template<typename T, const FS_Path* Paths, const FS_Path* InvalidPaths, int PathCount, const FS_Path* OverridePath, const FS_Path* OverridePathTemp, const PS_RSA_Context RsaPair[2]>
inline Result ManagedSignedFile_T<T,Paths,InvalidPaths,PathCount,OverridePath,OverridePathTemp,RsaPair>::CheckSignature() {
	if(!IsInit)
		return CFG_NOT_INITIALIZED;

	ALIGN(4) u8 hash[SHA256_HASH_LENGTH];
	sha256_full(Data.SignedData, sizeof(Data.SignedData), hash);

	PS_RSA_Context* ctx = &RsaPair[osEnvInfoIsDev() ? 1 : 0];
	Result res = psInit();
	if(R_SUCCEEDED(res)) {
		res = PS_VerifyRsaSha256(hash, ctx, Data.Signature);
		psExit();
	}

	IsSigned = R_SUCCEEDED(res);

	return R_FAILED(res) ? CFG_FAILED_SIGNATURE_CHECK : 0;
}

template<typename T, const FS_Path* Paths, const FS_Path* InvalidPaths, int PathCount, const FS_Path* OverridePath, const FS_Path* OverridePathTemp, const PS_RSA_Context RsaPair[2]>
inline bool ManagedSignedFile_T<T,Paths,InvalidPaths,PathCount,OverridePath,OverridePathTemp,RsaPair>::OpenDeleteFail(Handle& file, FS_Path& rwNandPath) {
	file = 0;

	Result res = FSUSER_OpenFile(&file, ::NandAccess::NandRWFSArchive, rwNandPath, FS_OPEN_READ, 0);

	if(R_FAILED(res))
		FSUSER_DeleteFile(::NandAccess::NandRWFSArchive, rwNandPath);

	return R_SUCCEEDED(res);
}

template<typename T, const FS_Path* Paths, const FS_Path* InvalidPaths, int PathCount, const FS_Path* OverridePath, const FS_Path* OverridePathTemp, const PS_RSA_Context RsaPair[2]>
inline void ManagedSignedFile_T<T,Paths,InvalidPaths,PathCount,OverridePath,OverridePathTemp,RsaPair>::Init() {
	Index = -1;
	IsSigned = false;
	WasChecked = false;
	Overriden = false;

	for(int i = 0; i < PathCount; ++i)
		FSUSER_DeleteFile(::NandAccess::NandRWFSArchive, InvalidPaths[i]);

	Handle file = 0;
	u32 filesize = 0;

	if(HasDefinedOverride()) {
		Result res = FSUSER_OpenFile(&file, ::NandAccess::NandRWFSArchive, OverridePath, FS_OPEN_READ, 0);
		if(R_SUCCEEDED(res)) res = FSFILE_Read(file, &filesize, 0LLU, &Data.Raw[0], sizeof(T));

		FSFILE_Close(file);
		svcCloseHandle(file);

		if(R_SUCCEEDED(res) && filesize == sizeof(T)) {
			IsInit = true;
			Overriden = true;
			return;
		}

		file = 0;
		filesize = 0;
	}

	for(int i = 0; i < PathCount; ++i) {
		if(OpenDeleteFail(file, Paths[i])) {
			Index = i;
		}
	}

	if(Index >= 0) {
		Result res = FSFILE_Read(file, &filesize, 0LLU, &Data.Raw[0], sizeof(T));
		if(R_FAILED(res) || filesize != sizeof(T)) {
			// checks that cfg didnt do
			memset(&Data.Raw[0], 0, sizeof(T));
		}
	} else {
		Index = 0;
		memset(&Data.Raw[0], 0, sizeof(T));

		static const FS_Path SysPath = {PATH_ASCII, 5, "/sys"};
		FSUSER_CreateDirectory(::NandAccess::NandRWFSArchive, SysPath, 0);

		file = 0;

		Result res = FSUSER_OpenFile(&file, ::NandAccess::NandRWFSArchive, InvalidPaths[0], FS_OPEN_WRITE | FS_OPEN_CREATE, 0);
		if(R_SUCCEEDED(res)) res = FSFILE_SetSize(file, sizeof(T));
		if(R_SUCCEEDED(res)) res = FSFILE_Write(file, &filesize, 0LLU, &Data.Raw[0], sizeof(T), FS_WRITE_FLUSH);
		if(R_SUCCEEDED(res) && filesize == sizeof(T)) FSUSER_RenameFile(::NandAccess::NandRWFSArchive, InvalidPaths[0], ::NandAccess::NandRWFSArchive, Paths[0]);
		else FSUSER_DeleteFile(::NandAccess::NandRWFSArchive, InvalidPaths[0]);
	}

	FSFILE_Close(file);
	svcCloseHandle(file);

	IsInit = true;
}

template<typename T, const FS_Path* Paths, const FS_Path* InvalidPaths, int PathCount, const FS_Path* OverridePath, const FS_Path* OverridePathTemp, const PS_RSA_Context RsaPair[2]>
inline void ManagedSignedFile_T<T,Paths,InvalidPaths,PathCount,OverridePath,OverridePathTemp,RsaPair>::SaveToNextSlot() {
	const FS_Path* target = nullptr;
	Handle file = 0;

	int last_index = Index;
	Index = (++Index >= PathCount) ? 0 : Index;

	if(Overriden) {
		target = OverridePathTemp;
	} else {
		target = &InvalidPaths[Index];
		if(OpenDeleteFail(file, Paths[Index])) {
			FSFILE_Close(file);
			svcCloseHandle(file);
			file = 0;
			FSUSER_DeleteFile(::NandAccess::NandRWFSArchive, InvalidPaths[Index]); // config didnt ensure path was clear to use, and not sure if FS errors on rename but
			FSUSER_RenameFile(::NandAccess::NandRWFSArchive, Paths[Index], ::NandAccess::NandRWFSArchive, InvalidPaths[Index]);
		}
	}

	Result res = FSUSER_OpenFile(&file, ::NandAccess::NandRWFSArchive, *target, FS_OPEN_WRITE | FS_OPEN_CREATE, 0);
	if(R_SUCCEEDED(res)) res = FSFILE_SetSize(file, sizeof(T));
	if(R_SUCCEEDED(res)) res = FSFILE_Write(file, &filesize, 0LLU, &Data.Raw[0], sizeof(T), FS_WRITE_FLUSH);

	FSFILE_Close(file);
	svcCloseHandle(file);

	if(!Overriden) {
		if(R_SUCCEEDED(res) && filesize == sizeof(T)) {
			FSUSER_RenameFile(::NandAccess::NandRWFSArchive, InvalidPaths[Index], ::NandAccess::NandRWFSArchive, Paths[Index]);
			FSUSER_RenameFile(::NandAccess::NandRWFSArchive, Paths[last_index], ::NandAccess::NandRWFSArchive, InvalidPaths[last_index]);
		} else {
			FSUSER_DeleteFile(::NandAccess::NandRWFSArchive, InvalidPaths[Index]);
			Index = last_index; // cfg doesn't check if error and roleback
		}
	} else {
		if(R_SUCCEEDED(res) && filesize == sizeof(T)) {
			FSUSER_DeleteFile(::NandAccess::NandRWFSArchive, OverridePath);
			FSUSER_RenameFile(::NandAccess::NandRWFSArchive, OverridePathTemp, ::NandAccess::NandRWFSArchive, OverridePath);
		} else {
			FSUSER_DeleteFile(::NandAccess::NandRWFSArchive, OverridePathTemp);
		}
	}
}

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

	Manager.WasChecked = false; // cfg didnt check later again, invalidated signed but not to check it again later
	Manager.IsSigned = false;

	return 0;
}

inline Result ManagedLfcs_T::SetData(const void* data, bool invalidateSignature) {
	if(!Manager.IsInit)
		return CFG_NOT_INITIALIZED;

	memcpy(Manager.Data.SignedData, data, sizeof(Manager.Data.SignedData));

	if(invalidateSignature) {
		Manager.WasChecked = false;
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

extern "C" void Cfg_Lfcs_Init() {
	ManagedLfcsFile.Manager.Init();
}

extern "C" void Cfg_Lfcs_SaveToNextSlot() {
	ManagedLfcsFile.Manager.SaveToNextSlot();
}

extern "C" Result Cfg_Lfcs_CheckSignature() {
	return ManagedLfcsFile.Manager.CheckSignature();
}

extern "C" Result Cfg_Lfcs_GetId(u64* id) {
	return ManagedLfcsFile.GetLfcsId(id);
}

extern "C" Result Cfg_Lfcs_GetWholeData(void* data, size_t size) {
	if(size < sizeof(Manager.Data)) // size checks were not originally done
		return CFG_INVALID_SIZE;

	ManagedLfcsFile.GetWholeLfcsNoChecks(data);
}

extern "C" Result Cfg_Lfcs_SetSignature(const void* sig, size_t size) {
	if(size < sizeof(Manager.Data.Signature)) // size checks were not originally done
		return CFG_INVALID_SIZE;

	return ManagedLfcsFile.SetSignature(sig);
}

extern "C" Result Cfg_Lfcs_SetData(const void* data, size_t size, bool reset_hardcoded) {
	if(reset_hardcoded)
		return ManagedLfcsFile.ResetToHardcodedId();

	if(size < sizeof(Manager.Data.SignedData)) // size checks were not originally done
		return CFG_INVALID_SIZE;

	return ManagedLfcsFile.SetData(data, false); // I guess for IPC data, we dont invalidate signature?
}

struct ManagedSecInfo_T {
	ManagedSignedFile_T<SecInfoData_T,SecInfoPaths,SecInfoInvalidPaths,2,&SecInfoOverride,&SecInfoOverrideTemp,SecInfoRsa> Manager;

	Result GetSignature(void* sig);
	Result GetRegionByte(u8* region);
	Result GetByte0x101(u8* unk);
	Result GetSerialNumber(char* serial);
	Result GetData(void* data);

	Result SetWholeSecInfo(const void* sig, const void* data);
};

inline Result ManagedSecInfo_T::GetSignature(void* sig) {
	if(!Manager.IsInit)
		return CFG_NOT_INITIALIZED;

	memcpy(sig, Manager.Data.Signature, sizeof(Manager.Data.Signature));

	return 0;
}

inline Result ManagedSecInfo_T::GetRegionByte(u8* region) {
	if(!Manager.IsInit)
		return CFG_NOT_INITIALIZED;

	if(!Manager.WasChecked) {
		Manager.CheckSignature();
		Manager.WasChecked = true;
	}

	if(!Manager.IsSigned)
		return CFG_BAD_SIGNATURE;

	*region = Manager.Data.Region;

	return 0;
}

inline Result ManagedSecInfo_T::GetByte0x101(u8* unk) {
	if(!Manager.IsInit)
		return CFG_NOT_INITIALIZED;

	if(!Manager.WasChecked) {
		Manager.CheckSignature();
		Manager.WasChecked = true;
	}

	if(!Manager.IsSigned)
		return CFG_BAD_SIGNATURE;

	*unk = Manager.Data.Byte0x101;

	return 0;
}

inline Result ManagedSecInfo_T::GetSerialNumber(char* serial) {
	if(!Manager.IsInit)
		return CFG_NOT_INITIALIZED;

	if(!Manager.WasChecked) {
		Manager.CheckSignature();
		Manager.WasChecked = true;
	}

	if(!Manager.IsSigned)
		return CFG_BAD_SIGNATURE;

	memcpy(serial, Manager.Data.Serial, sizeof(Manager.Data.Serial));

	return 0;
}

inline Result ManagedSecInfo_T::GetData(void* data) {
	if(!Manager.IsInit)
		return CFG_NOT_INITIALIZED;

	memcpy(data, Manager.Data.SignedData, sizeof(Manager.Data.SignedData));

	return 0;
}

inline Result ManagedSecInfo_T::SetWholeSecInfo(const void* sig, const void* data) {
	if(!Manager.IsInit)
		return CFG_NOT_INITIALIZED;

	memcpy(Manager.Data.Signature,  sig,  sizeof(Manager.Data.Signature));
	memcpy(Manager.Data.SignedData, data, sizeof(Manager.Data.SignedData));

	return 0;
}

ManagedSecInfo_T ManagedSecInfoFile = {
	.Manager = (ManagedSignedFile_T<SecInfoData_T,SecInfoPaths,SecInfoInvalidPaths,2,&SecInfoOverride,&SecInfoOverrideTemp,SecInfoRsa>){
		.Data = (SecInfoData_T){},
		.IsInit = false,
		.IsSigned = false,
		.WasChecked = false,
		.Overriden = false,
		.Index = -1
	}
};

extern "C" void Cfg_SecInfo_Init() {
	ManagedSecInfoFile.Manager.Init();
}

extern "C" void Cfg_SecInfo_SaveToNextSlot() {
	ManagedSecInfoFile.Manager.SaveToNextSlot();
}

extern "C" Result Cfg_SecInfo_CheckSignature() {
	return ManagedSecInfoFile.Manager.CheckSignature();
}

extern "C" Result Cfg_SecInfo_GetSignature(void* sig, size_t size) {
	if(size < sizeof(Manager.Data.Signature)) // size checks were not originally done
		return CFG_INVALID_SIZE;

	return ManagedSecInfoFile.GetSignature(sig);
}

extern "C" Result Cfg_SecInfo_GetRegionByte(u8* region) {
	return ManagedSecInfoFile.GetRegionByte(region);
}

extern "C" Result Cfg_SecInfo_GetByte0x101(u8* unk) {
	return ManagedSecInfoFile.GetByte0x101(unk);
}

extern "C" Result Cfg_SecInfo_GetSerialNumber(char* serial, size_t size) {
	if(size < sizeof(Manager.Data.Serial)) // size checks were not originally done
		return CFG_INVALID_SIZE;

	return ManagedSecInfoFile.GetSerialNumber(serial);
}

extern "C" Result Cfg_SecInfo_GetData(void* data, size_t size) {
	if(size < sizeof(Manager.Data.SignedData)) // size checks were not originally done
		return CFG_INVALID_SIZE;

	return ManagedSecInfoFile.GetData(data);
}

extern "C" Result Cfg_SecInfo_SetWholeSecInfo(const void* sig, size_t sigsize, const void* data, size_t datasize) {
	if(sigsize < sizeof(Manager.Data.Signature) || datasize < sizeof(Manager.Data.SignedData)) // size checks were not originally done
		return CFG_INVALID_SIZE;

	return ManagedSecInfoFile.SetWholeSecInfo(sig, data);
}

template<typename T>
static inline bool CheckHwcalChecksum(const T& data) {
	return crc16(0x55AA, &data, offsetof(T, Checksum)) == data.Checksum;
}

template<typename T>
static bool inline CheckHwcalByteFlipper(const T& data) {
	if(sizeof(data.FlippedBytes) == 1) {
		const u8* ptr1 = reinterpret_cast<const u8*>(&data);
		const u8* ptr2 = &data.FlippedBytes[0];
		return (ptr1[0] ^ ptr2[0]) == 0xFF;
	} else if(sizeof(data.FlippedBytes) == 2) {
		const u16* ptr1 = reinterpret_cast<const u16*>(&data);
		const u16* ptr2 = reinterpret_cast<const u16*>(&data.FlippedBytes[0]);
		return (ptr1[0] ^ ptr2[0]) == 0xFFFF;
	} else {
		return false;
	}
}

static size_t Hwcali2cReadLoop(Handle i2cEEPHandle, void* out, u16 offset, size_t size) {
	if(size & 0x1) return 0; // don't even

	svcSleepThread(5000000LLU); // seems that cfg sleeps here, because..? maybe i2c needs time?
	// loop detected in binary would accept an index to a table to determine the size to loop with
	// table would go 0x10, 0x20, 0x20, 0x800, 0x1000, 0x2000
	// I do not know what the index means, likely an enum but I know nothing about it's members
	// I do know, however, that cfg only uses value 1, which indexes at the table to the size of 0x20
	// so I'll optimize it for that
	size_t total_read = 0;
	size_t num_blocks = size >> 5;
	size_t remainder_bytes = size & 0x1F;

	u8* _out = reinterpret_cast<u8*>(out);

	for(size_t i = 0; i < num_blocks; ++i) {
		if(R_FAILED(I2C_ReadEEPROM(i2cEEPHandle, 14, reinterpret_cast<void*>(_out), offset, 0x20))) {
			total_read = 0;
			break;
		}

		offset += 0x20;
		_out += 0x20;
	}

	return total_read;
}

enum CALIndexes : u8 {
	CAL_INDEX_RTCCOMPENSATION   = 0,
	CAL_INDEX_SCREENFLICKER     = 1,
	CAL_INDEX_OUTERCAMS1        = 2,
	CAL_INDEX_TOUCH             = 3,
	CAL_INDEX_CIRCLEPAD1        = 4,
	CAL_INDEX_CODEC             = 5,
	CAL_INDEX_GYRO              = 6,
	CAL_INDEX_RTCCORRECTION     = 7,
	CAL_INDEX_ACCELEROMETER     = 8,
	CAL_INDEX_SOUND3DFILTER     = 9,
	CAL_INDEX_LCDPOWERSAVE      = 10,
	CAL_INDEX_LCDSTEREOSCOPIC   = 11,
	CAL_INDEX_BACKLIGHTPWM      = 12,
	CAL_INDEX_CIRCLEPAD2        = 13,
	CAL_INDEX_OUTERCAMS2        = 14,
	CAL_INDEX_LCDPOWERSAVELGY   = 15,
	CAL_INDEX_VOLUMESLIDER      = 16,
	CAL_INDEX_LCDMODEDELAY      = 17,
	CAL_INDEX_MICECHOCANCEL     = 18,
	CAL_INDEX_CSTICK            = 19,
	CAL_INDEX_DEADINDEX20       = 20,
	CAL_INDEX_LCDPOWERSAVEEXTRA = 21,
	CAL_INDEX_PIT               = 22,
	CAL_INDEX_QTM               = 23
};

struct ManagedHwcal_T {
	HWCAL_T Hwcal;
	int Index;
	bool SystemUsesEEP:1;
	bool SystemIsDev:1;

	bool CheckAgingFlag(CALIndexes index) const;
	bool ReadCalIndex(void* ptr, CALIndexes index) const;
	bool WriteCalIndex(const void* ptr, CALIndexes index, u16 agingFlag);
	void GenerateDummyHeader();
	void GenerateDummy();
	bool WriteDummy(int index);
	bool CheckHeaderAndHash() const;
	bool ReadFromI2C(int index);
	bool ReadFromFile(int index);
	bool ReadIndex(int index);
	void Load();
};

bool ManagedHwcal_T::CheckAgingFlag(CALIndexes index) const {
	if(index < 16) {
		if(Hwcal.Header.Version != 0 && (index == CAL_INDEX_SCREENFLICKER || index == CAL_INDEX_TOUCH))
			return true;
		return (Hwcal.Header.AgingSuccessBitmask & (1u << index)) != 0;
	}

	u16 AgingFlag = 0;

	switch(index) {
	case CAL_INDEX_VOLUMESLIDER:
		if(!CheckHwcalChecksum<HWCALVolumeSlider_T>(Hwcal.Body.VolumeSlider))
			break;
		AgingFlag = Hwcal.Body.VolumeSlider.AgingFlag;
		break;
	case CAL_INDEX_LCDMODEDELAY:
		if(!CheckHwcalChecksum<HWCALLcdModeDelay_T>(Hwcal.Body.LcdModeDelay))
			break;
		AgingFlag = Hwcal.Body.LcdModeDelay.AgingFlag;
		break;
	case CAL_INDEX_MICECHOCANCEL:
		if(!CheckHwcalChecksum<HWCALMicrophoneEchoCancellation_T>(Hwcal.Body.MicEchoCancel))
			break;
		AgingFlag = Hwcal.Body.MicEchoCancel.AgingFlag;
		break;
	case CAL_INDEX_CSTICK:
		if(!CheckHwcalChecksum<HWCALCStick_T>(Hwcal.Body.CStick))
			break;
		AgingFlag = Hwcal.Body.CStick.AgingFlag;
		break;
	case CAL_INDEX_DEADINDEX20:
		AgingFlag = 1;
		break;
	case CAL_INDEX_LCDPOWERSAVEEXTRA:
		if(!CheckHwcalChecksum<HWCALLcdPowerSaveExtra_T>(Hwcal.Body.LcdPowersaveExtra))
			break;
		AgingFlag = Hwcal.Body.LcdPowersaveExtra.AgingFlag;
		break;
	case CAL_INDEX_PIT:
		if(!CheckHwcalChecksum<HWCALPit_T>(Hwcal.Body.Pit))
			break;
		AgingFlag = Hwcal.Body.Pit.AgingFlag;
		break;
	case CAL_INDEX_QTM:
		if(!CheckHwcalChecksum<HWCALQtm_T>(Hwcal.Body.Qtm))
			break;
		AgingFlag = Hwcal.Body.Qtm.AgingFlag;
		break;
	default:
		break;
	}

	return AgingFlag != 0;
}

template<typename T>
static void CAL_COPY(void* dst, const void* src) {
	uptr dst_cast = reinterpret_cast<uptr>(dst);
	uptr src_cast = reinterpret_cast<uptr>(src);
	uptr alignmask = (alignof(T)-1);

	if(alignof(T) > 1 && ((dst_cast & alignmask) || (src_cast & alignmask)))
		memcpy(dst, src, sizeof(T));
	else
		*reinterpret_cast<T*>(dst) = *reinterpret_cast<const T*>(src);
}

template<typename T>
static bool CAL_COPY_FLIPPERCHECK(void* dst, const void* src) {
	T* _dst = reinterpret_cast<T*>(dst);
	const T* _src = reinterpret_cast<const T*>(src);

	if(!CheckHwcalByteFlipper<T>(*_src))
		return false;

	CAL_COPY<decltype(_dst->Data)>(&_dst->Data, &_src->Data);
	return true;
}

template<typename T>
static bool CAL_COPY_CRCCHECK(void* dst, const void* src) {
	T* _dst = reinterpret_cast<T*>(dst);
	const T* _src = reinterpret_cast<const T*>(src);

	if(!CheckHwcalChecksum<T>(*_src))
		return false;

	CAL_COPY<decltype(_dst->Data)>(&_dst->Data, &_src->Data);
	return true;
}

template<typename T>
static bool CAL_COPY_FLIPPERMAKE(void* dst, const void* src) {
	T* _dst = reinterpret_cast<T*>(dst);

	if(sizeof(_dst.Data) > 2
	  || sizeof(_dst.FlippedBytes) > 2
	  || sizeof(_dst.Data) != sizeof(_dst.FlippedBytes)
	  || offsetof(T, FlippedBytes) != sizeof(_dst.Data)) {
		return false;
	}

	if(sizeof(_dst.Data) == 1) {
		u8 data = *reinterpret_cast<const u8*>(&src);
		u8* ptr1 = reinterpret_cast<u8*>(&_dst);
		u8* ptr2 = &_dst.FlippedBytes[0];
		*ptr1 = data;
		*ptr2 = data ^ 0xFF;
	} else if(sizeof(_dst.Data) == 2) {
		u16 data = *reinterpret_cast<const u16*>(&src);
		u16* ptr1 = reinterpret_cast<u16*>(&_dst);
		u16* ptr2 = reinterpret_cast<u16*>(&_dst.FlippedBytes[0]);
		*ptr1 = data;
		*ptr2 = data ^ 0xFFFF;
	}

	return true;
}

template<typename T>
static bool CAL_COPY_CRCMAKE(void* dst, const void* src) {
	T* _dst = reinterpret_cast<T*>(dst);
	const decltype(_dst.Data)* _src = reinterpret_cast<const decltype(_dst.Data)*>(src);

	CAL_COPY<decltype(_src)>(&_dst->Data, _src);
	_dst.Checksum = crc16(0x55AA, dst, offsetof(T, Checksum));

	return true;
}

template<typename T>
static bool CAL_COPY_CRCMAKE_AGING(void* dst, const void* src, u16 agingFlag) {
	T* _dst = reinterpret_cast<T*>(dst);

	_dst.AgingFlag |= agingFlag;

	return CAL_COPY_CRCMAKE<T>(dst, src);
}

bool ManagedHwcal_T::ReadCalIndex(void* ptr, CALIndexes index) const {
	bool ret = false;

	switch(index) {
	case CAL_INDEX_RTCCOMPENSATION:
		ret = CAL_COPY_FLIPPERCHECK<HWCALRtcCompensation_T>(ptr, Hwcal.Body.RtcCompensation);
		break;
	case CAL_INDEX_SCREENFLICKER:
		ret = CAL_COPY_FLIPPERCHECK<HWCALScreenFlicker_T>(ptr, Hwcal.Body.ScreenFlicker);
		break;
	case CAL_INDEX_OUTERCAMS1:
		ret = CAL_COPY_CRCCHECK<HWCALOuterCamarasPart1_T>(ptr, Hwcal.Body.OuterCams1);
		break;
	case CAL_INDEX_TOUCH:
		ret = CAL_COPY_CRCCHECK<HWCALTouch_T>(ptr, Hwcal.Body.Touch);
		break;
	case CAL_INDEX_CIRCLEPAD1:
		ret = CAL_COPY_CRCCHECK<HWCALCirclePadPart1_T>(ptr, Hwcal.Body.CirclePad1);
		break;
	case CAL_INDEX_CODEC:
		ret = CAL_COPY_CRCCHECK<HWCALCodec_T>(ptr, Hwcal.Body.Codec);
		break;
	case CAL_INDEX_GYRO:
		ret = CAL_COPY_CRCCHECK<HWCALGyroscope_T>(ptr, Hwcal.Body.Gyro);
		break;
	case CAL_INDEX_RTCCORRECTION:
		ret = CAL_COPY_FLIPPERCHECK<HWCALRtcCorrection_T>(ptr, Hwcal.Body.RtcCorrection);
		break;
	case CAL_INDEX_ACCELEROMETER:
		ret = CAL_COPY_CRCCHECK<HWCALAccelerometer_T>(ptr, Hwcal.Body.Accelerometer);
		break;
	case CAL_INDEX_SOUND3DFILTER:
		ret = CAL_COPY_CRCCHECK<HWCALSound3DFilter_T>(ptr, Hwcal.Body.Sound3DFilter);
		break;
	case CAL_INDEX_LCDPOWERSAVE:
		ret = CAL_COPY_CRCCHECK<HWCALLcdPowerSave_T>(ptr, Hwcal.Body.LcdPowersave);
		break;
	case CAL_INDEX_LCDSTEREOSCOPIC:
		ret = CAL_COPY_CRCCHECK<HWCALLcdStereoscopic_T>(ptr, Hwcal.Body.LcdStereoscopic);
		break;
	case CAL_INDEX_BACKLIGHTPWM:
		ret = CAL_COPY_CRCCHECK<HWCALBacklightPwm_T>(ptr, Hwcal.Body.BlPwn);
		break;
	case CAL_INDEX_CIRCLEPAD2:
		ret = CAL_COPY_CRCCHECK<HWCALCirclePadPart2_T>(ptr, Hwcal.Body.CirclePad2);
		break;
	case CAL_INDEX_OUTERCAMS2:
		ret = CAL_COPY_CRCCHECK<HWCALOuterCamarasPart2_T>(ptr, Hwcal.Body.OuterCams2);
		break;
	case CAL_INDEX_LCDPOWERSAVELGY:
		ret = CAL_COPY_CRCCHECK<HWCALLcdPowerSave_T>(ptr, Hwcal.Body.LcdPowersaveLgy);
		break;
	case CAL_INDEX_VOLUMESLIDER:
		ret = CAL_COPY_CRCCHECK<HWCALVolumeSlider_T>(ptr, Hwcal.Body.VolumeSlider);
		break;
	case CAL_INDEX_LCDMODEDELAY:
		ret = CAL_COPY_CRCCHECK<HWCALLcdModeDelay_T>(ptr, Hwcal.Body.LcdModeDelay);
		break;
	case CAL_INDEX_MICECHOCANCEL:
		ret = CAL_COPY_CRCCHECK<HWCALMicrophoneEchoCancellation_T>(ptr, Hwcal.Body.MicEchoCancel);
		break;
	case CAL_INDEX_CSTICK:
		ret = CAL_COPY_CRCCHECK<HWCALCStick_T>(ptr, Hwcal.Body.CStick);
		break;
	case CAL_INDEX_DEADINDEX20:
		ret = true;
		break;
	case CAL_INDEX_LCDPOWERSAVEEXTRA:
		ret = CAL_COPY_CRCCHECK<HWCALLcdPowerSaveExtra_T>(ptr, Hwcal.Body.LcdPowersaveExtra);
		break;
	case CAL_INDEX_PIT:
		ret = CAL_COPY_CRCCHECK<HWCALPit_T>(ptr, Hwcal.Body.Pit);
		break;
	case CAL_INDEX_QTM:
		ret = CAL_COPY_CRCCHECK<HWCALQtm_T>(ptr, Hwcal.Body.Qtm);
		break;
	default:
		break;
	}

	return ret;
}

bool ManagedHwcal_T::WriteCalIndex(const void* ptr, CALIndexes index, u16 agingFlag) {
	bool ret = false;

	switch(index) {
	case CAL_INDEX_RTCCOMPENSATION:
		ret = CAL_COPY_FLIPPERMAKE<HWCALRtcCompensation_T>(Hwcal.Body.RtcCompensation, ptr);
		break;
	case CAL_INDEX_SCREENFLICKER:
		ret = CAL_COPY_FLIPPERMAKE<HWCALScreenFlicker_T>(Hwcal.Body.ScreenFlicker, ptr);
		break;
	case CAL_INDEX_OUTERCAMS1:
		ret = CAL_COPY_CRCMAKE<HWCALOuterCamarasPart1_T>(Hwcal.Body.OuterCams1, ptr);
		break;
	case CAL_INDEX_TOUCH:
		ret = CAL_COPY_CRCMAKE<HWCALTouch_T>(Hwcal.Body.Touch, ptr);
		break;
	case CAL_INDEX_CIRCLEPAD1:
		ret = CAL_COPY_CRCMAKE<HWCALCirclePadPart1_T>(Hwcal.Body.CirclePad1, ptr);
		break;
	case CAL_INDEX_CODEC:
		ret = CAL_COPY_CRCMAKE<HWCALCodec_T>(Hwcal.Body.Codec, ptr);
		break;
	case CAL_INDEX_GYRO:
		ret = CAL_COPY_CRCMAKE<HWCALGyroscope_T>(Hwcal.Body.Gyro, ptr);
		break;
	case CAL_INDEX_RTCCORRECTION:
		ret = CAL_COPY_FLIPPERMAKE<HWCALRtcCorrection_T>(Hwcal.Body.RtcCorrection, ptr);
		break;
	case CAL_INDEX_ACCELEROMETER:
		ret = CAL_COPY_CRCMAKE<HWCALAccelerometer_T>(Hwcal.Body.Accelerometer, ptr);
		break;
	case CAL_INDEX_SOUND3DFILTER:
		ret = CAL_COPY_CRCMAKE<HWCALSound3DFilter_T>(Hwcal.Body.Sound3DFilter, ptr);
		break;
	case CAL_INDEX_LCDPOWERSAVE:
		ret = CAL_COPY_CRCMAKE<HWCALLcdPowerSave_T>(Hwcal.Body.LcdPowersave, ptr);
		break;
	case CAL_INDEX_LCDSTEREOSCOPIC:
		ret = CAL_COPY_CRCMAKE<HWCALLcdStereoscopic_T>(Hwcal.Body.LcdStereoscopic, ptr);
		break;
	case CAL_INDEX_BACKLIGHTPWM:
		ret = CAL_COPY_CRCMAKE<HWCALBacklightPwm_T>(Hwcal.Body.BlPwn, ptr);
		break;
	case CAL_INDEX_CIRCLEPAD2:
		ret = CAL_COPY_CRCMAKE<HWCALCirclePadPart2_T>(Hwcal.Body.CirclePad2, ptr);
		break;
	case CAL_INDEX_OUTERCAMS2:
		ret = CAL_COPY_CRCMAKE<HWCALOuterCamarasPart2_T>(Hwcal.Body.OuterCams2, ptr);
		break;
	case CAL_INDEX_LCDPOWERSAVELGY:
		ret = CAL_COPY_CRCMAKE<HWCALLcdPowerSave_T>(Hwcal.Body.LcdPowersaveLgy, ptr);
		break;
	case CAL_INDEX_VOLUMESLIDER:
		ret = CAL_COPY_CRCMAKE_AGING<HWCALVolumeSlider_T>(Hwcal.Body.VolumeSlider, ptr, agingFlag);
		break;
	case CAL_INDEX_LCDMODEDELAY:
		ret = CAL_COPY_CRCMAKE_AGING<HWCALLcdModeDelay_T>(Hwcal.Body.LcdModeDelay, ptr, agingFlag);
		break;
	case CAL_INDEX_MICECHOCANCEL:
		ret = CAL_COPY_CRCMAKE_AGING<HWCALMicrophoneEchoCancellation_T>(Hwcal.Body.MicEchoCancel, ptr, agingFlag);
		break;
	case CAL_INDEX_CSTICK:
		ret = CAL_COPY_CRCMAKE_AGING<HWCALCStick_T>(Hwcal.Body.CStick, ptr, agingFlag);
		break;
	case CAL_INDEX_DEADINDEX20:
		ret = true;
		break;
	case CAL_INDEX_LCDPOWERSAVEEXTRA:
		ret = CAL_COPY_CRCMAKE_AGING<HWCALLcdPowerSaveExtra_T>(Hwcal.Body.LcdPowersaveExtra, ptr, agingFlag);
		break;
	case CAL_INDEX_PIT:
		ret = CAL_COPY_CRCMAKE_AGING<HWCALPit_T>(Hwcal.Body.Pit, ptr, agingFlag);
		break;
	case CAL_INDEX_QTM:
		ret = CAL_COPY_CRCMAKE_AGING<HWCALQtm_T>(Hwcal.Body.Qtm, ptr, agingFlag);
		break;
	default:
		break;
	}

	if(index < 16 && ret && agingFlag)
		Hwcal.Header.AgingSuccessBitmask |=  (1u << index);
	else if(index < 16)
		Hwcal.Header.AgingSuccessBitmask &= ~(1u << index);

	return ret;
}

void ManagedHwcal_T::GenerateDummyHeader() {
	Hwcal.Header.Magic = getbe32(0x4343414C) // 'CCAL'
	Hwcal.Header.Version = 18;
	Hwcal.Header.DataSize = sizeof(HWCALBody_T);
	Hwcal.Header.ModelVersion = 0;
	Hwcal.Header.Revision = 0;
	Hwcal.Header.AgingSuccessBitmask = 0;
	if(SystemIsDev || SystemUsesEEP) {
		sha256_full(&Hwcal.Body, sizeof(Hwcal.Body), Hwcal.Header.Sha256);
	} else {
		calculate_hmacsha256(&Hwcal.Body, sizeof(HWCALBody_T), Hwcal.Header.HmacSha256, HwcalHMACKey, sizeof(HwcalHMACKey));
	}
}

void ManagedHwcal_T::GenerateDummy() {
	union {
		HWCALGyroscopeData_T a,
		HWCALRtcCorrectionData_T b,
		HWCALOuterCamarasPart2Data_T c,
		HWCALCStickData_T d,
		HWCALLcdPowerSaveExtraData_T e
	} dummies;

	memset(&Hwcal, 0, sizeof(HWCAL_T));
	memset(&dummies, 0, sizeof(dummies));

	WriteCalIndex(&DummyRtcComp, CAL_INDEX_RTCCOMPENSATION, 0);
	WriteCalIndex(&DummyScreenFlicker, CAL_INDEX_SCREENFLICKER, 0);
	WriteCalIndex(&DummyOuterCams1, CAL_INDEX_OUTERCAMS1, 0);
	WriteCalIndex(&DummyTouch, CAL_INDEX_TOUCH, 0);
	WriteCalIndex(&DummyCirclePad1, CAL_INDEX_CIRCLEPAD1, 0);
	WriteCalIndex(&DummyCodec, CAL_INDEX_CODEC, 0);
	WriteCalIndex(&dummies, CAL_INDEX_GYRO, 0);
	WriteCalIndex(&dummies, CAL_INDEX_RTCCORRECTION, 0);
	WriteCalIndex(&DummyAccelerometer, CAL_INDEX_ACCELEROMETER, 0);
	WriteCalIndex(&DummySound3DFilter, CAL_INDEX_SOUND3DFILTER, 0);
	WriteCalIndex(&DummyLcdPowerSave, CAL_INDEX_LCDPOWERSAVE, 0);
	WriteCalIndex(&DummyLcdStereoscopic, CAL_INDEX_LCDSTEREOSCOPIC, 0);
	WriteCalIndex(&DummyBacklight, CAL_INDEX_BACKLIGHTPWM, 0);
	WriteCalIndex(&DummyCirclePad2, CAL_INDEX_CIRCLEPAD2, 0);
	WriteCalIndex(&dummies, CAL_INDEX_OUTERCAMS2, 0);
	WriteCalIndex(&DummyLcdPowerSave, CAL_INDEX_LCDPOWERSAVELGY, 0);
	WriteCalIndex(&DummyVolumeSlider, CAL_INDEX_VOLUMESLIDER, 0);
	WriteCalIndex(&DummyLcdModeDelay, CAL_INDEX_LCDMODEDELAY, 0);
	WriteCalIndex(&DummyMicEchoCancel, CAL_INDEX_MICECHOCANCEL, 0);
	WriteCalIndex(&dummies, CAL_INDEX_CSTICK, 0);
	WriteCalIndex(&dummies, CAL_INDEX_LCDPOWERSAVEEXTRA, 0);
	WriteCalIndex(&DummyPit, CAL_INDEX_PIT, 0);
	WriteCalIndex(&DummyQtm, CAL_INDEX_QTM, 0);

	GenerateDummyHeader();
}

bool ManagedHwcal_T::WriteDummy(int index) {
	GenerateDummy();
	if(SystemUsesEEP) return true;

	Handle file = 0;
	u32 filesize = 0;
	Result res = 0;

	bool ret = false;

	res = FSUSER_OpenFile(&file, ::NandAccess::NandROFSArchive, HwcalPaths[index], FS_OPEN_WRITE | FS_OPEN_CREATE, 0);
	if(R_SUCCEEDED(res)) res = FSFILE_SetSize(file, sizeof(HWCAL_T));
	if(R_SUCCEEDED(res)) res = FSFILE_Write(file, &filesize, 0LLU, &Hwcal, sizeof(HWCAL_T));
	if(R_SUCCEEDED(res) && filesize == sizeof(HWCAL_T)) ret = true;

	FSFILE_Close(file);
	svcCloseHandle(file);

	return ret;
}

bool ManagedHwcal_T::CheckHeaderAndHash() const {
	if(getbe32(Hwcal.Header.Magic) != 0x4343414C) // 'CCAL'
		return false;

	u8 hash[SHA256_HASH_LENGTH];

	if(SystemIsDev || SystemUsesEEP) {
		u8 hash[SHA256_HASH_LENGTH];
		sha256_full(&Hwcal.Body, sizeof(Hwcal.Body), hash);
		return memcmp(Hwcal.Header.Sha256, hash, SHA256_HASH_LENGTH) == 0;
	} else {
		u8 hmac[HMAC_SHA256_HASH_LENGTH];
		calculate_hmacsha256(&Hwcal.Body, sizeof(Hwcal.Body), hmac, HwcalHMACKey, sizeof(HwcalHMACKey));
		return memcmp(Hwcal.Header.HmacSha256, hmac, HMAC_SHA256_HASH_LENGTH) == 0;
	}
}

bool ManagedHwcal_T::ReadFromI2C(int index) {
	Handle i2cEEPHandle;

	if(R_FAILED(i2cEEPInit(&i2cEEPHandle)))
		return false;

	u16 offset = index << 11; // index * 0x800

	if(Hwcali2cReadLoop(i2cEEPHandle, &Hwcal.Header, offset, sizeof(HWCALHeader_T)) != sizeof(HWCALHeader_T)) {
		i2cExit(&i2cEEPHandle);
		return false;
	}

	if(Hwcali2cReadLoop(i2cEEPHandle, &Hwcal.Body, offset+sizeof(HWCALHeader_T), sizeof(HWCALBody_T)) != sizeof(HWCALBody_T)) {
		i2cExit(&i2cEEPHandle);
		return false;
	}

	i2cExit(&i2cEEPHandle);
	return true;
}

bool ManagedHwcal_T::ReadFromFile(int index) {
	Handle file = 0;
	u32 filesize = 0;
	Result res = 0;

	bool ret = false;

	res = FSUSER_OpenFile(&file, ::NandAccess::NandROFSArchive, HwcalPaths[index], FS_OPEN_READ, 0);
	if(R_SUCCEEDED(res)) res = FSFILE_Read(file, &filesize, 0LLU, &Hwcal, sizeof(HWCAL_T));
	if(R_SUCCEEDED(res) && filesize == sizeof(HWCAL_T)) ret = true;

	FSFILE_Close(file);
	svcCloseHandle(file);

	return ret;
}

bool ManagedHwcal_T::ReadIndex(int index) {
	bool ret;
	ret = SystemUsesEEP ? ReadFromI2C(index) : ReadFromFile(index);
	return ret ? CheckHeaderAndHash() : false;
}

void ManagedHwcal_T::Load() {
	u8 hw = osWaitRunningHw();

	SystemUsesEEP = hw == HW_KMC_DEBUGGER || hw == HW_KMC_CAPTURE || hw == HW_SNAKE_IS_DEBUGGER || hw == HW_SNAKE_IS_CAPTURE || hw == HW_SNAKE_IS_CAPTURE;
	SystemIsDev = osEnvInfoIsDev();

	// would be faster to avoid a double read but also need more memory for that.

	if(!ReadIndex(0))
		WriteDummy(0);

	auto index0_rev = Hwcal.Header.Revision;

	if(!ReadIndex(1))
		WriteDummy(1);

	if(Hwcal.Header.Revision < index0_rev) {
		ReadIndex(0);
		Index = 0;
	} else Index = 1;
}
