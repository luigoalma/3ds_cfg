#pragma once
#include <3ds/types.h>

#define CONFIGSIZE 32768

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
	Result GetBlkPtr(void*& ptr, u32 blkId, size_t size, CFG_BlkFlags flags, CFG_BlkFlags bitmask, bool exactMatchAccess = false);
	Result GetBlkPtrForReading(const void*& ptr, u32 blkId, size_t size, CFG_BlkFlags flags);
	Result GetBlkPtrForWriting(void*& ptr, u32 blkId, size_t size, CFG_BlkFlags flags);
	Result CreateBlk(void*& ptr, u32 blkId, size_t size, CFG_BlkFlags flags);
	Result UpdateBlkFlags(u32 blkId, CFG_BlkFlags flags);
	Result ReadBlk(void* ptr, u32 blkId, size_t size, bool system);
	Result WriteBlk(const void* ptr, u32 blkId, size_t size, bool system);
};

static_assert(sizeof(ConfigData_T) == CONFIGSIZE);
static_assert(offsetof(ConfigData_T, TotalEntries) == 0);
static_assert(offsetof(ConfigData_T, DataEntryOffset) == 2);
static_assert(offsetof(ConfigData_T, BlkEntries) == 4);
static_assert(alignof(ConfigData_T) == 8);

inline void* ConfigBlkEntry_T::GetPtr(void* BasePtr) {
	if(DataSize <= 4) return reinterpret_cast<void*>(&Data[0]);
	return reinterpret_cast<void*>(reinterpret_cast<u8*>(BasePtr) + RelOffset);
}

inline ConfigBlkEntry_T* ConfigData_T::FindBlkId(u32 Id) {
	ConfigBlkEntry_T* ptr = nullptr;

	for(int i = 0; i < TotalEntries; ++i) {
		if(BlkEntries[i].Id == Id) {
			ptr = &BlkEntries[i];
		}
	}

	return ptr;
}
