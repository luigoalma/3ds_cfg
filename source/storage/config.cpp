#include <cstddef>
#include <3ds/errf.h>
#include <3ds/fs.h>
#include <3ds/result.h>
#include <3ds/srv.h>
#include <3ds/svc.h>
#include <3ds/types.h>
#include <cfg.h>
#include <err.h>
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
	Err_FailedThrow(::SystemSave::Delete(::SystemSave::FixDataFSInfo))
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

void ConfigData_T::DeleteAndReset() {
	FSUSER_DeleteFile(::SystemSave::NormalFSArchive, ConfigPath);
	TotalEntries = 0;
	DataEntryOffset = CONFIGSIZE;
}

Result ConfigData_T::GetBlkPtr(void*& ptr, u32 blkId, size_t size, CFG_BlkFlags accessFlags, CFG_BlkFlags bitmask) {
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

Result ConfigData_T::GetBlkPtrForReading(const void*& ptr, u32 blkId, size_t size, CFG_BlkFlags accessFlags) {
	return GetBlkPtr(ptr, blkId, size, accessFlags, BLK_READ_PERM_BITMASK);
}

Result ConfigData_T::GetBlkPtrForWriting(void*& ptr, u32 blkId, size_t size, CFG_BlkFlags accessFlags) {
	return GetBlkPtr(ptr, blkId, size, accessFlags, BLK_READ_WRITE_BITMASK);
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

Result ConfigData_T::WriteBlk(const void* ptr, u32 blkId, size_t size, bool system) {
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

static void Cfg_CreateHwcalBlks() {
	void *ptr1,  *ptr2,  *ptr3,  *ptr4,  *ptr5,  *ptr6,  *ptr7,  *ptr8,  *ptr9,  *ptr10;
	void *ptr11, *ptr12, *ptr13, *ptr14, *ptr15, *ptr16, *ptr17, *ptr18, *ptr19, *ptr20;
	void *ptr21;

	ManagedHwcal_T hwcal;
	hwcal.Load();

	u8 rev = hwcal.Hwcal.Header.Revision;

	// original cfg doesnt check success of blk creation...

	Err_FailedThrow(ConfigSave.CreateBlk(ptr1,  0x10000,  sizeof(HWCALRtcCompensationData_T),            BLK_RW_SYSTEM));
	Err_FailedThrow(ConfigSave.CreateBlk(ptr2,  0x50000,  sizeof(HWCALScreenFlickerData_T),              BLK_RW_SYSTEM));
	Err_FailedThrow(ConfigSave.CreateBlk(ptr3,  0x40000,  sizeof(HWCALTouchData_T),                      BLK_RW_SYSTEM));
	Err_FailedThrow(ConfigSave.CreateBlk(ptr4,  0x40002,  sizeof(HWCALGyroscopeData_T),                  BLK_RW_SYSTEM));
	Err_FailedThrow(ConfigSave.CreateBlk(ptr5,  0x30000,  sizeof(HWCALRtcCorrectionData_T),              BLK_RW_SYSTEM));
	Err_FailedThrow(ConfigSave.CreateBlk(ptr6,  0x40003,  sizeof(HWCALAccelerometerData_T),              BLK_RW_SYSTEM));
	Err_FailedThrow(ConfigSave.CreateBlk(ptr7,  0x60000,  sizeof(OuterCamaras_T),                        BLK_RW_SYSTEM));
	Err_FailedThrow(ConfigSave.CreateBlk(ptr8,  0x40001,  sizeof(CirclePadParts_T),                      BLK_RW_SYSTEM));
	Err_FailedThrow(ConfigSave.CreateBlk(ptr9,  0x50002,  sizeof(HWCALBacklightPwmData_T),               BLK_RW_SYSTEM));
	Err_FailedThrow(ConfigSave.CreateBlk(ptr10, 0x50003,  sizeof(HWCALLcdPowerSaveData_T),               BLK_RW_SYSTEM));
	Err_FailedThrow(ConfigSave.CreateBlk(ptr11, 0x50004,  sizeof(HWCALLcdPowerSaveData_T),               BLK_RW_SYSTEM));
	Err_FailedThrow(ConfigSave.CreateBlk(ptr12, 0x50005,  sizeof(HWCALLcdStereoscopicData_T),            BLK_RW_ANY));
	Err_FailedThrow(ConfigSave.CreateBlk(ptr13, 0x120000, sizeof(HWCALSlidersData_T),                    BLK_RW_SYSTEM));
	Err_FailedThrow(ConfigSave.CreateBlk(ptr14, 0x70000,  sizeof(HWCALSound3DFilterData_T),              (rev < 7) ? BLK_RW_SYSTEM : BLK_RW_ANY));
	Err_FailedThrow(ConfigSave.CreateBlk(ptr15, 0x20000,  sizeof(HWCALCodecData_T),                      BLK_RW_SYSTEM));
	Err_FailedThrow(ConfigSave.CreateBlk(ptr16, 0x50006,  sizeof(HWCALLcdModeDelayData_T),               BLK_RW_SYSTEM));
	Err_FailedThrow(ConfigSave.CreateBlk(ptr17, 0x70002,  sizeof(HWCALMicrophoneEchoCancellationData_T), BLK_RW_ANY));
	Err_FailedThrow(ConfigSave.CreateBlk(ptr18, 0x40004,  sizeof(CStick_T),                              BLK_RW_SYSTEM));
	Err_FailedThrow(ConfigSave.CreateBlk(ptr19, 0x50008,  sizeof(HWCALLcdPowerSaveExtraData_T),          BLK_RW_SYSTEM));
	Err_FailedThrow(ConfigSave.CreateBlk(ptr20, 0x50007,  sizeof(HWCALPitData_T),                        BLK_RW_SYSTEM));
	Err_FailedThrow(ConfigSave.CreateBlk(ptr21, 0x180001, sizeof(HWCALQtmData_T),                        BLK_RW_SYSTEM));

	hwcal.ReadCalIndex(ptr1, CAL_INDEX_RTCCOMPENSATION);
	hwcal.ReadCalIndex(ptr2, CAL_INDEX_SCREENFLICKER);
	hwcal.ReadCalIndex(ptr3, CAL_INDEX_TOUCH);
	hwcal.ReadCalIndex(ptr4, CAL_INDEX_GYRO);
	hwcal.ReadCalIndex(ptr5, CAL_INDEX_RTCCORRECTION);
	if(hwcal.CheckAgingFlag(CAL_INDEX_ACCELEROMETER)) {
		hwcal.ReadCalIndex(ptr6, CAL_INDEX_ACCELEROMETER);
	} else {
		memcpy(ptr6, DummyAccelerometer, sizeof(HWCALAccelerometerData_T));
	}
	Hwcal_GetOuterCamsNoCheck(hwcal, ptr7);
	Hwcal_GetCirclePadNoCheck(hwcal, ptr8);
	if(rev < 11) {
		memcpy(ptr9, &DummyBacklight, sizeof(HWCALBacklightPwmData_T));
	} else {
		hwcal.ReadCalIndex(ptr9, CAL_INDEX_BACKLIGHTPWM);
	}
	if(rev < 8) {
		memcpy(ptr10, &DummyLcdPowerSave, sizeof(HWCALLcdPowerSaveData_T));
		memcpy(ptr11, &DummyLcdPowerSave, sizeof(HWCALLcdPowerSaveData_T));
	} else {
		hwcal.ReadCalIndex(ptr10, CAL_INDEX_LCDPOWERSAVE);
		hwcal.ReadCalIndex(ptr11, CAL_INDEX_LCDPOWERSAVELGY);
	}
	if(rev < 10) {
		memcpy(ptr12, &DummyLcdStereoscopic, sizeof(HWCALLcdStereoscopicData_T));
	} else {
		hwcal.ReadCalIndex(ptr12, CAL_INDEX_LCDSTEREOSCOPIC);
	}
	if(rev < 7) {
		memcpy(ptr13, &DummySliders, sizeof(HWCALSlidersData_T));
		memcpy(ptr14, &DefaultSound3DFilter, sizeof(HWCALSound3DFilterData_T));
	} else {
		hwcal.ReadCalIndex(ptr13, CAL_INDEX_SLIDERS);
		hwcal.ReadCalIndex(ptr14, CAL_INDEX_SOUND3DFILTER);
	}
	if(rev < 9) {
		memcpy(ptr15, &DummyCodec, sizeof(HWCALCodecData_T));
		if(hwcal.CheckAgingFlag(CAL_INDEX_CODEC)) {
			HWCALCodecData_T foo;
			hwcal.ReadCalIndex(reinterpret_cast<void*>(&foo), CAL_INDEX_CODEC);
			reinterpret_cast<HWCALCodecData_T*>(ptr15)->PGA_GAIN = foo.PGA_GAIN;
		}
		memcpy(ptr16, &DummyLcdModeDelay, sizeof(HWCALLcdModeDelayData_T));
	} else {
		hwcal.ReadCalIndex(ptr15, CAL_INDEX_CODEC);
		if(rev < 12) {
			reinterpret_cast<HWCALCodecData_T*>(ptr15)->AnalogInterval = DummyCodec.AnalogInterval;
			reinterpret_cast<HWCALCodecData_T*>(ptr15)->Analog_XP_Pullup = DummyCodec.Analog_XP_Pullup;
		}
		hwcal.ReadCalIndex(ptr16, CAL_INDEX_LCDMODEDELAY);
	}
	if(rev < 13) {
		memcpy(ptr17, &DummyMicEchoCancel, sizeof(HWCALMicrophoneEchoCancellationData_T));
	} else {
		hwcal.ReadCalIndex(ptr17, CAL_INDEX_MICECHOCANCEL);
	}
	Hwcal_GetCStickNoCheck(hwcal, ptr18);
	if(rev < 15) {
		memset(ptr19, 0, sizeof(HWCALLcdPowerSaveExtraData_T));
	} else {
		hwcal.ReadCalIndex(ptr19, CAL_INDEX_LCDPOWERSAVEEXTRA);
	}
	if(rev < 16) {
		memcpy(ptr20, &DefaultPit, sizeof(HWCALPitData_T));
	} else {
		hwcal.ReadCalIndex(ptr20, CAL_INDEX_PIT);
	}
	if(rev < 18) {
		memcpy(ptr21, &DefaultQtm, sizeof(HWCALQtmData_T));
	} else {
		hwcal.ReadCalIndex(ptr21, CAL_INDEX_QTM);
	}
}

static void Cfg_CreateNormalBlks() {

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
