#include <cstddef>
#include <cstring>
#include <3ds/errf.h>
#include <3ds/fs.h>
#include <3ds/result.h>
#include <3ds/srv.h>
#include <3ds/svc.h>
#include <3ds/types.h>
#include <3ds/os.h>
#include <cfg.h>
#include <err.h>
#include <storage/secinfo.h>
#include "storage.hpp"
#include "config.hpp"
#include "hwcal.hpp"

#define UTIL_INVALID_COMBINATION MAKERESULT(RL_USAGE, RS_INVALIDARG, RM_UTIL, RD_INVALID_COMBINATION)

static const FS_Path ConfigPath = {PATH_ASCII, 8, "/config"};

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
	Err_FailedThrow(::SystemSave::Delete(::SystemSave::FixDataFSInfo));
}

ConfigBlkEntry_T* ConfigData_T::FindBlkId(u32 Id) {
	ConfigBlkEntry_T* ptr = nullptr;

	for(int i = 0; i < TotalEntries; ++i) {
		if(BlkEntries[i].Id == Id) {
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

void ConfigData_T::Reset() {
	TotalEntries = 0;
	DataEntryOffset = CONFIGSIZE;
}

void ConfigData_T::Save() const {
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

void ConfigData_T::Load() {
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

void ConfigData_T::SaveFixData() const {
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

bool ConfigData_T::LoadFixData() {
	Handle file = 0;
	Result res = 0;
	bool pass = false;

	if(R_FAILED(::SystemSave::SimpleOpen(::SystemSave::FixDataFSArchive, ::SystemSave::FixDataFSPath)))
		return false;

	res = FSUSER_OpenFile(&file, ::SystemSave::FixDataFSArchive, ConfigPath, FS_OPEN_READ, 0);

	if(R_SUCCEEDED(res)) {
		u32 filesize = 0;
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

void ConfigData_T::DeleteAndReset() {
	FSUSER_DeleteFile(::SystemSave::NormalFSArchive, ConfigPath);
	TotalEntries = 0;
	DataEntryOffset = CONFIGSIZE;
}

Result ConfigData_T::GetBlkPtr(void*& ptr, u32 blkId, size_t size, CFG_BlkFlags accessFlags, CFG_BlkFlags bitmask, bool exactMatchAccess) {
	ptr = nullptr;

	if(size == 0 || size > CONFIGSIZE)
		return CFG_INVALID_SIZE;

	ConfigBlkEntry_T* blk = FindBlkId(blkId);

	if(!blk)
		return CFG_NOT_FOUND;

	u16 flags = blk->Flags & accessFlags & bitmask;

	if(!flags)
		return CFG_NOT_AUTHORIZED;

	if(exactMatchAccess && flags != accessFlags)
		return CFG_NOT_AUTHORIZED;

	if(blk->DataSize != size)
		return CFG_INVALID_SIZE;

	ptr = blk->GetPtr(&Raw[0]);

	return 0;
}

Result ConfigData_T::GetBlkPtrForReading(const void*& ptr, u32 blkId, size_t size, CFG_BlkFlags accessFlags) {
	void* _ptr = nullptr;
	Result res = GetBlkPtr(_ptr, blkId, size, accessFlags, BLK_READ_PERM_BITMASK);
	ptr = _ptr;
	return res;
}

Result ConfigData_T::GetBlkPtrForWriting(void*& ptr, u32 blkId, size_t size, CFG_BlkFlags accessFlags) {
	return GetBlkPtr(ptr, blkId, size, accessFlags, BLK_WRITE_PERM_BITMASK);
}

Result ConfigData_T::CreateBlk(void*& ptr, u32 blkId, size_t size, CFG_BlkFlags flags) {
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

Result ConfigData_T::UpdateBlkFlags(u32 blkId, CFG_BlkFlags flags) {
	ConfigBlkEntry_T* blk = FindBlkId(blkId);

	if(!blk)
		return CFG_NOT_FOUND;

	blk->Flags = flags;

	return 0;
}

Result ConfigData_T::ReadBlk(void* ptr, u32 blkId, size_t size, bool system) {
	const void* _ptr;

	Result res = GetBlkPtrForReading(_ptr, blkId, size, system ? BLK_SYSTEM_READ_PERM : BLK_USER_READ_PERM);
	if(R_FAILED(res)) return res;

	if(size <= 4) {
		if((reinterpret_cast<uptr>(ptr) & 0x3) == 0) {
			*reinterpret_cast<u32*>(ptr) = *reinterpret_cast<const u32*>(_ptr);
		} else if((reinterpret_cast<uptr>(ptr) & 0x1) == 0) {
			*reinterpret_cast<u16*>(ptr) = *reinterpret_cast<const u16*>(_ptr);
			if(size > 2) *(reinterpret_cast<u16*>(ptr)+1) = *(reinterpret_cast<const u16*>(_ptr)+1);
		} else {
			*reinterpret_cast<u8*>(ptr) = *reinterpret_cast<const u8*>(_ptr);
			if(size > 1) *(reinterpret_cast<u8*>(ptr)+1) = *(reinterpret_cast<const u8*>(_ptr)+1);
			if(size > 2) *(reinterpret_cast<u8*>(ptr)+2) = *(reinterpret_cast<const u8*>(_ptr)+2);
			if(size > 3) *(reinterpret_cast<u8*>(ptr)+3) = *(reinterpret_cast<const u8*>(_ptr)+3);
		}
	} else {
		memcpy(ptr, _ptr, size);
	}

	return 0;
}

Result ConfigData_T::WriteBlk(const void* ptr, u32 blkId, size_t size, bool system) {
	void* _ptr;

	Result res = GetBlkPtrForWriting(_ptr, blkId, size, system ? BLK_SYSTEM_WRITE_PERM : BLK_USER_WRITE_PERM);
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

static void Cfg_CreateHwcalBlks() {
	void *ptr;

	ManagedHwcal_T hwcal;
	hwcal.Load();

	u8 rev = hwcal.Hwcal.Header.Revision;

	// original cfg doesnt check success of blk creation...

	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0x10000,  sizeof(HWCALRtcCompensationData_T),            BLK_RW_SYSTEM));
	hwcal.ReadCalIndex(ptr, CAL_INDEX_RTCCOMPENSATION);
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0x50000,  sizeof(HWCALScreenFlickerData_T),              BLK_RW_SYSTEM));
	hwcal.ReadCalIndex(ptr, CAL_INDEX_SCREENFLICKER);
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0x40000,  sizeof(HWCALTouchData_T),                      BLK_RW_SYSTEM));
	hwcal.ReadCalIndex(ptr, CAL_INDEX_TOUCH);
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0x40002,  sizeof(HWCALGyroscopeData_T),                  BLK_RW_SYSTEM));
	hwcal.ReadCalIndex(ptr, CAL_INDEX_GYRO);
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0x30000,  sizeof(HWCALRtcCorrectionData_T),              BLK_RW_SYSTEM));
	hwcal.ReadCalIndex(ptr, CAL_INDEX_RTCCORRECTION);
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0x40003,  sizeof(HWCALAccelerometerData_T),              BLK_RW_SYSTEM));
	if(hwcal.CheckAgingFlag(CAL_INDEX_ACCELEROMETER)) {
		hwcal.ReadCalIndexWithDefault(ptr, CAL_INDEX_ACCELEROMETER, &DummyAccelerometer);
	} else {
		memcpy(ptr, &DummyAccelerometer, sizeof(HWCALAccelerometerData_T));
	}
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0x60000,  sizeof(OuterCamaras_T),                        BLK_RW_SYSTEM));
	Hwcal_GetOuterCamsNoCheck(hwcal, ptr);
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0x40001,  sizeof(CirclePadParts_T),                      BLK_RW_SYSTEM));
	Hwcal_GetCirclePadNoCheck(hwcal, ptr);
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0x50002,  sizeof(HWCALBacklightPwmData_T),               BLK_RW_SYSTEM));
	hwcal.ReadCalIndexWithDefault(ptr, CAL_INDEX_BACKLIGHTPWM, &DummyBacklight, 11);
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0x50003,  sizeof(HWCALLcdPowerSaveData_T),               BLK_RW_SYSTEM));
	hwcal.ReadCalIndexWithDefault(ptr, CAL_INDEX_LCDPOWERSAVE, &DummyLcdPowerSave, 8);
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0x50004,  sizeof(HWCALLcdPowerSaveData_T),               BLK_RW_SYSTEM));
	hwcal.ReadCalIndexWithDefault(ptr, CAL_INDEX_LCDPOWERSAVELGY, &DummyLcdPowerSave, 8);
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0x50005,  sizeof(HWCALLcdStereoscopicData_T),            BLK_RW_ANY));
	hwcal.ReadCalIndexWithDefault(ptr, CAL_INDEX_LCDSTEREOSCOPIC, &DummyLcdStereoscopic, 10);
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0x120000, sizeof(HWCALSlidersData_T),                    BLK_RW_SYSTEM));
	hwcal.ReadCalIndexWithDefault(ptr, CAL_INDEX_SLIDERS, &DummySliders, 7);
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0x70000,  sizeof(HWCALSound3DFilterData_T),              (rev < 7) ? BLK_RW_SYSTEM : BLK_RW_ANY));
	hwcal.ReadCalIndexWithDefault(ptr, CAL_INDEX_SOUND3DFILTER, &DefaultSound3DFilter, 7);
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0x20000,  sizeof(HWCALCodecData_T),                      BLK_RW_SYSTEM));
	hwcal.ReadCalIndexWithDefault(ptr, CAL_INDEX_CODEC, &DummyCodec, 9);
	if(rev < 9) {
		if(hwcal.CheckAgingFlag(CAL_INDEX_CODEC)) {
			HWCALCodecData_T foo;
			hwcal.ReadCalIndex(reinterpret_cast<void*>(&foo), CAL_INDEX_CODEC);
			reinterpret_cast<HWCALCodecData_T*>(ptr)->PGA_GAIN = foo.PGA_GAIN;
		}
	} else if(rev < 12) {
		reinterpret_cast<HWCALCodecData_T*>(ptr)->AnalogInterval = DummyCodec.AnalogInterval;
		reinterpret_cast<HWCALCodecData_T*>(ptr)->Analog_XP_Pullup = DummyCodec.Analog_XP_Pullup;
	}
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0x50006,  sizeof(HWCALLcdModeDelayData_T),               BLK_RW_SYSTEM));
	hwcal.ReadCalIndexWithDefault(ptr, CAL_INDEX_LCDMODEDELAY, &DummyLcdModeDelay, 9);
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0x70002,  sizeof(HWCALMicrophoneEchoCancellationData_T), BLK_RW_ANY));
	hwcal.ReadCalIndexWithDefault(ptr, CAL_INDEX_MICECHOCANCEL, &DummyMicEchoCancel, 13);
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0x40004,  sizeof(CStick_T),                              BLK_RW_SYSTEM));
	Hwcal_GetCStickNoCheck(hwcal, ptr);
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0x50008,  sizeof(HWCALLcdPowerSaveExtraData_T),          BLK_RW_SYSTEM));
	hwcal.ReadCalIndexWithDefault(ptr, CAL_INDEX_LCDPOWERSAVEEXTRA, nullptr, 15);
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0x50007,  sizeof(HWCALPitData_T),                        BLK_RW_SYSTEM));
	hwcal.ReadCalIndexWithDefault(ptr, CAL_INDEX_PIT, &DefaultPit, 16);
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0x180001, sizeof(HWCALQtmData_T),                        BLK_RW_SYSTEM));
	hwcal.ReadCalIndexWithDefault(ptr, CAL_INDEX_QTM, &DefaultQtm, 18);
}

static void Cfg_CreateNormalBlks() {
	void *ptr;

	auto hw = osWaitRunningHw();
	bool snake = hw == HW_UNKNOWN || hw == HW_SNAKE_PRODUCT || hw == HW_SNAKE_IS_DEBUGGER || hw == HW_SNAKE_IS_CAPTURE || hw == HW_SNAKE_KMC_DEBUGGER;

	static const u64 home_tids[7] = {
		0x0004003000008202LLU, // JPN
		0x0004003000008F02LLU, // USA
		0x0004003000009802LLU, // EUR
		0x0004003000008202LLU, // AUS - cfg default to JPN tid
		0x000400300000A102LLU, // CHN
		0x000400300000A902LLU, // KOR
		0x000400300000B102LLU, // TWN
	};

	u8 region = 0;
	SecInfo_GetRegionByte(&region);

	u64 target_home_tid = home_tids[(region > 6) ? 0 : region];

	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0x30001,  8,     BLK_RW_ANY));
	*reinterpret_cast<u64*>(ptr) = 0;
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0x30002,  8,     BLK_RW_SYSTEM));
	*reinterpret_cast<u64*>(ptr) = 0;
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0x50001,  2,     BLK_RW_SYSTEM));
	*reinterpret_cast<u16*>(ptr) = 0x500;
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0x80000,  0xC00, BLK_RW_SYSTEM));
	memset(ptr, 0, 0xC00);
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0x80001,  0xC00, BLK_RW_SYSTEM));
	memset(ptr, 0, 0xC00);
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0x80002,  0xC00, BLK_RW_SYSTEM));
	memset(ptr, 0, 0xC00);
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0xA0000,  0x1C,  BLK_RW_ANY));
	memset(ptr, 0, 0x1C);
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0xA0001,  2,     BLK_RW_ANY));
	*reinterpret_cast<u16*>(ptr) = 0x0101;
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0xA0002,  1,     BLK_RW_ANY));
	*reinterpret_cast<u8*>(ptr) = 0;
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0xB0001,  0x800, BLK_RW_ANY));
	memset(ptr, 0, 0x800);
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0xB0002,  0x800, BLK_RW_ANY));
	memset(ptr, 0, 0x800);
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0xB0000,  4,     BLK_RW_ANY));
	*reinterpret_cast<u32*>(ptr) = 0xFFFFFFFF;
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0xB0003,  4,     BLK_RW_ANY));
	*reinterpret_cast<u32*>(ptr) = 0;
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0x90000,  8,     BLK_RW_ANY));
	*reinterpret_cast<u64*>(ptr) = 0;
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0x90001,  8,     BLK_RW_ANY));
	*reinterpret_cast<u64*>(ptr) = 0;
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0x90002,  4,     BLK_RW_ANY));
	*reinterpret_cast<u32*>(ptr) = 0;
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0xC0000,  0xC0,  BLK_RW_ANY));
	memset(ptr, 0, 0xC0);
	reinterpret_cast<u8*>(ptr)[9] = 0x14;
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0xC0001,  0x14,  BLK_RW_ANY));
	memset(ptr, 0, 0x14);
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0xC0002,  0x200, BLK_RW_ANY));
	memset(ptr, 0, 0x200);
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0xD0000,  4,     BLK_RW_ANY));
	*reinterpret_cast<u32*>(ptr) = 0x00010000;
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0xE0000,  1,     BLK_RW_ANY));
	*reinterpret_cast<u8*>(ptr) = 0;
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0x70001,  1,     BLK_RW_ANY));
	*reinterpret_cast<u8*>(ptr) = 0;
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0xF0000,  0x10,  BLK_RW_SYSTEM));
	reinterpret_cast<u64*>(ptr)[0] = 0;
	reinterpret_cast<u64*>(ptr)[1] = snake ? 0x300000006LLU: 0x300000000LLU;
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0xF0001,  8,     BLK_RW_SYSTEM));
	*reinterpret_cast<u64*>(ptr) = 0;
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0xF0003,  1,     BLK_RW_SYSTEM));
	*reinterpret_cast<u8*>(ptr) = 0;
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0xF0004,  4,     BLK_RW_SYSTEM));
	*reinterpret_cast<u32*>(ptr) = 0;
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0x100000, 2,     BLK_RW_SYSTEM));
	*reinterpret_cast<u16*>(ptr) = 0;
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0x100001, 0x94,  BLK_RW_SYSTEM));
	memset(ptr, 0, 0x94);
	reinterpret_cast<u8*>(ptr)[13] = 0x30;
	reinterpret_cast<u8*>(ptr)[14] = 0x30;
	reinterpret_cast<u8*>(ptr)[15] = 0x30;
	reinterpret_cast<u8*>(ptr)[16] = 0x30;
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0x100002, 1,     BLK_RW_SYSTEM));
	*reinterpret_cast<u8*>(ptr) = 0;
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0x100003, 0x10,  BLK_RW_SYSTEM));
	reinterpret_cast<u64*>(ptr)[0] = 0;
	reinterpret_cast<u64*>(ptr)[1] = 0;
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0x110000, 4,     BLK_RW_SYSTEM));
	*reinterpret_cast<u32*>(ptr) = 0;
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0x110001, 8,     BLK_RW_SYSTEM));
	*reinterpret_cast<u64*>(ptr) = target_home_tid; // only for dev, unless it's an extended memory game reboot on an o3ds, that *suddenly* matters
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0x130000, 4,     BLK_RW_ANY));
	*reinterpret_cast<u32*>(ptr) = 0;
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0x150000, 4,     BLK_RW_SYSTEM));
	*reinterpret_cast<u32*>(ptr) = 0;
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0x150001, 8,     BLK_RW_SYSTEM));
	*reinterpret_cast<u64*>(ptr) = 0;
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0x160000, 4,     BLK_RW_ANY));
	*reinterpret_cast<u32*>(ptr) = 0;
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0x170000, 4,     BLK_RW_ANY));
	*reinterpret_cast<u32*>(ptr) = 0;
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0x180000, 4,     BLK_RW_SYSTEM));
	*reinterpret_cast<u32*>(ptr) = 0;
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0x50009,  8,     BLK_RW_SYSTEM));
	reinterpret_cast<float*>(ptr)[0] = 1.0; // 0x3F800000
	reinterpret_cast<u32*>(ptr)[1]   = 0x100;
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0x190000, 1,     BLK_RW_SYSTEM));
	*reinterpret_cast<u8*>(ptr) = 0;
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0xF0005,  4,     BLK_RW_SYSTEM));
	*reinterpret_cast<u32*>(ptr) = 1;
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0xF0006,  0x28,  BLK_RW_SYSTEM));
	memset(ptr, 0, 0x28);
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0x150002, 4,     BLK_RW_ANY));
	*reinterpret_cast<u32*>(ptr) = 0;
}

extern "C" void Cfg_DeleteAndSetDefaultBlks() {
	void* ptr = nullptr;

	// this has no reason to fail, nor even does
	ConfigSave.DeleteAndReset();
	// yet cfg treated as this may fail and try to write to existing blk if creation failed, which can never fail
	// if anything fails here, you're either very lucky or very unlucky
	Err_FailedThrow(ConfigSave.CreateBlk(ptr, 0x0, 2, BLK_RW_SYSTEM));
	*reinterpret_cast<u16*>(ptr) = 57; // current version
	Cfg_CreateHwcalBlks();
	Cfg_CreateNormalBlks();
}

extern "C" Result Cfg_UpgradeSave() {
	ManagedHwcal_T hwcal;

	Result res = 0;
	void* ptr;
	const void* cptr;

	u16 version;

	if(R_FAILED(ConfigSave.ReadBlk(&version, 0x0, 2, true))) {
		Cfg_DeleteAndSetDefaultBlks();
		return 0;
	}

	if(version < 15) {
		// cfg continues after this ??
		// Why not just format? Version upgrades after format!
		Cfg_DeleteAndSetDefaultBlks();
	}

	if(version < 50) { // dont waste time if version
		hwcal.Load();
	}

	if(version < 16) {
		res = ConfigSave.GetBlkPtr(ptr, 0x20000, sizeof(HWCALCodecData_T), BLK_SYSTEM_ALL_PERM, BLK_ALL_PERM_BITMASK, true);
		if(R_FAILED(res)) return res;

		// also this next segment is off place?
		// format already does the correct blk for 0x20000 based on hwcal
		// but in this condition, we reset some codec block like if always hwcal 9 <= revision < 12

		reinterpret_cast<HWCALCodecData_T*>(ptr)->AnalogInterval = DummyCodec.AnalogInterval;
		reinterpret_cast<HWCALCodecData_T*>(ptr)->Analog_XP_Pullup = DummyCodec.Analog_XP_Pullup;
	}

	if(version < 17) {
		res = ConfigSave.CreateBlk(ptr, 0xC0001, 0x14, BLK_RW_ANY);
		if(R_FAILED(res)) return res;
		memset(ptr, 0, 0x14);
	}

	if(version < 18) {
		res = ConfigSave.CreateBlk(ptr, 0x70002, sizeof(HWCALMicrophoneEchoCancellationData_T), BLK_RW_ANY);
		if(R_FAILED(res)) return res;
		// system's hwcal is not considered... why exactly?
		memcpy(ptr, &DummyMicEchoCancel, sizeof(HWCALMicrophoneEchoCancellationData_T));
	}

	if(version < 19) {
		ConfigBlkEntry_T* blk = ConfigSave.FindBlkId(0xE0000);
		if(!blk) return CFG_NOT_FOUND;
		blk->Flags = BLK_RW_ANY;
	}

	if(version < 20) {
		res = ConfigSave.GetBlkPtrForReading(cptr, 0x90001, 8, BLK_SYSTEM_READ_PERM);
		if(R_FAILED(res)) return res;
		res = ConfigSave.CreateBlk(ptr, 0x90002, 4, BLK_RW_ANY);
		if(R_FAILED(res)) return res;
		*reinterpret_cast<u32*>(ptr) = reinterpret_cast<const u16*>(cptr)[3];
	}

	if(version < 50) {
		// starting at version 50, we start caring about blocks already existing?

		res = ConfigSave.CreateBlk(ptr, 0xF0004, 4, BLK_RW_SYSTEM);
		if(R_FAILED(res) && res != CFG_ALREADY_EXISTS) return res;
		if(res != CFG_ALREADY_EXISTS) 
			*reinterpret_cast<u32*>(ptr) = 0;

		res = ConfigSave.CreateBlk(ptr, 0x150000, 4, BLK_RW_SYSTEM);
		if(R_FAILED(res) && res != CFG_ALREADY_EXISTS) return res;
		if(res != CFG_ALREADY_EXISTS) 
			*reinterpret_cast<u32*>(ptr) = 0;

		res = ConfigSave.CreateBlk(ptr, 0x160000, 4, BLK_RW_ANY);
		if(R_FAILED(res) && res != CFG_ALREADY_EXISTS) return res;
		if(res != CFG_ALREADY_EXISTS) 
			*reinterpret_cast<u32*>(ptr) = 0;

		res = ConfigSave.CreateBlk(ptr, 0x170000, 4, BLK_RW_ANY);
		if(R_FAILED(res) && res != CFG_ALREADY_EXISTS) return res;
		if(res != CFG_ALREADY_EXISTS) 
			*reinterpret_cast<u32*>(ptr) = 0;

		res = ConfigSave.CreateBlk(ptr, 0x40004, sizeof(CStick_T), BLK_RW_SYSTEM);
		if(R_FAILED(res) && res != CFG_ALREADY_EXISTS) return res;
		if(res != CFG_ALREADY_EXISTS) 
			Hwcal_GetCStickNoCheck(hwcal, ptr);

		res = ConfigSave.CreateBlk(ptr, 0x50008, sizeof(HWCALLcdPowerSaveExtraData_T), BLK_RW_SYSTEM);
		if(R_FAILED(res) && res != CFG_ALREADY_EXISTS) return res;
		if(res != CFG_ALREADY_EXISTS) 
			hwcal.ReadCalIndexWithDefault(ptr, CAL_INDEX_LCDPOWERSAVEEXTRA, nullptr, 15);

		res = ConfigSave.CreateBlk(ptr, 0x50007, sizeof(HWCALPitData_T), BLK_RW_SYSTEM);
		if(R_FAILED(res) && res != CFG_ALREADY_EXISTS) return res;
		if(res != CFG_ALREADY_EXISTS) 
			hwcal.ReadCalIndexWithDefault(ptr, CAL_INDEX_PIT, &DefaultPit, 16);

		res = ConfigSave.CreateBlk(ptr, 0x180000, 4, BLK_RW_SYSTEM);
		if(R_FAILED(res) && res != CFG_ALREADY_EXISTS) return res;
		if(res != CFG_ALREADY_EXISTS) 
			*reinterpret_cast<u32*>(ptr) = 0;

		res = ConfigSave.CreateBlk(ptr, 0x50009, 8, BLK_RW_SYSTEM);
		if(R_FAILED(res) && res != CFG_ALREADY_EXISTS) return res;
		if(res != CFG_ALREADY_EXISTS) 
		{
			reinterpret_cast<float*>(ptr)[0] = 1.0; // 0x3F800000
			reinterpret_cast<u32*>(ptr)[1]   = 0x100;
		}

		res = ConfigSave.CreateBlk(ptr, 0x180001, sizeof(HWCALQtmData_T), BLK_RW_SYSTEM);
		if(R_FAILED(res) && res != CFG_ALREADY_EXISTS) return res;
		if(res != CFG_ALREADY_EXISTS) 
			hwcal.ReadCalIndexWithDefault(ptr, CAL_INDEX_QTM, &DefaultQtm, 18);

		// after this point, cfg would check for version < 50 *again*
		// and **now** load hwcal on the previous blocks that are hwcal related
	}

	if(version < 51) {
		res = ConfigSave.CreateBlk(ptr, 0xC0002, 0x200, BLK_RW_ANY);
		if(R_FAILED(res) && res != CFG_ALREADY_EXISTS) return res;
		if(res != CFG_ALREADY_EXISTS) memset(ptr, 0, 0x200);
	}

	if(version < 52) {
		res = ConfigSave.CreateBlk(ptr, 0x150001, 8, BLK_RW_SYSTEM);
		if(R_FAILED(res) && res != CFG_ALREADY_EXISTS) return res;
		if(res != CFG_ALREADY_EXISTS) *reinterpret_cast<u64*>(ptr) = 0;
	}

	// suddenly, we don't care again about existing cfg blocks!!
	if(version < 53) {
		res = ConfigSave.CreateBlk(ptr, 0x190000, 1, BLK_RW_SYSTEM);
		if(R_FAILED(res)) return res;
		*reinterpret_cast<u8*>(ptr) = 0;
	}

	if(version < 54) {
		res = ConfigSave.CreateBlk(ptr, 0xF0005, 4, BLK_RW_SYSTEM);
		if(R_FAILED(res)) return res;
		*reinterpret_cast<u32*>(ptr) = 1;
	}

	if(version < 55) {
		res = ConfigSave.CreateBlk(ptr, 0xF0006, 0x28, BLK_RW_SYSTEM);
		if(R_FAILED(res)) return res;
		memset(ptr, 0, 0x28);
	}

	if(version < 56) {
		res = ConfigSave.CreateBlk(ptr, 0x150002, 4, BLK_RW_ANY); // originally created as SYSTEM
		if(R_FAILED(res)) return res;
		*reinterpret_cast<u32*>(ptr) = 0;
	}

	if(version >= 56 && version < 57) {
		ConfigBlkEntry_T* blk = ConfigSave.FindBlkId(0x150002);
		if(!blk) return CFG_NOT_FOUND;
		blk->Flags = BLK_RW_ANY;
	}

	res = ConfigSave.GetBlkPtrForWriting(ptr, 0x0, 2, BLK_SYSTEM_WRITE_PERM);
	if(R_FAILED(res)) return res;
	*reinterpret_cast<u16*>(ptr) = 57;

	Cfg_SaveConfig();

	return 0;
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
