#include <cstddef>
extern "C" {
	#include <3ds/errf.h>
	#include <3ds/fs.h>
	#include <3ds/result.h>
	#include <3ds/srv.h>
	#include <3ds/svc.h>
	#include <3ds/types.h>
}
#include <cfg.h>
#include <err.h>
#include "storage.cpp"

#define UTIL_INVALID_COMBINATION MAKERESULT(RL_USAGE, RS_INVALIDARG, RM_UTIL, RD_INVALID_COMBINATION)

#define CONFIGSIZE 32768

static const FS_Path ConfigPath = {PATH_ASCII, 8, "/config"};

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
