#include <cstddef>
#include <3ds/fs.h>
#include <3ds/i2c.h>
#include <3ds/os.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/types.h>
#include <utils/sha256.h>
#include <utils/hmacsha256.h>
#include <utils/endian.h>
#include <storage/config.h>
#include <storage/hwcal.h>
#include "cfg.h"
#include "storage.hpp"
#include "hwcal_dummy_defaults.h"

static const FS_Path[2] HwcalPaths = {
	{PATH_ASCII, 16, "/sys/HWCAL0.dat"},
	{PATH_ASCII, 16, "/sys/HWCAL1.dat"}
};

static ALIGN(4) const u8 HwcalHMACKey[32] = {
	0x0D, 0x36, 0xF9, 0xC8, 0xEA, 0xED, 0x15, 0xA7, 0xB9, 0x22, 0xE3, 0xC7, 0x70, 0xE7, 0x53, 0x48,
	0xDA, 0x2D, 0x57, 0x35, 0xC2, 0x59, 0x9F, 0x93, 0x5D, 0xE1, 0x81, 0x4D, 0xE6, 0x65, 0x40, 0x8B
};

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
	CAL_INDEX_SLIDERS           = 16,
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
	case CAL_INDEX_SLIDERS:
		if(!CheckHwcalChecksum<HWCALSliders_T>(Hwcal.Body.Sliders))
			break;
		AgingFlag = Hwcal.Body.Sliders.AgingFlag;
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
	case CAL_INDEX_SLIDERS:
		ret = CAL_COPY_CRCCHECK<HWCALSliders_T>(ptr, Hwcal.Body.Sliders);
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
	case CAL_INDEX_SLIDERS:
		ret = CAL_COPY_CRCMAKE_AGING<HWCALSliders_T>(Hwcal.Body.Sliders, ptr, agingFlag);
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
	WriteCalIndex(&DummySliders, CAL_INDEX_SLIDERS, 0);
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

static void Hwcal_GetCirclePadNoCheck(void* ptr) {
	ManagedHwcal_T hwcal;
	hwcal.Load();

	CirclePadParts_T* _ptr = reinterpret_cast<CirclePadParts_T*>(ptr);

	HWCALCirclePadPart1Data_T part1;
	hwcal.ReadCalIndex(&part1, CAL_INDEX_CIRCLEPAD1);

	if(hwcal.Hwcal.Header.Revision < 7) {
		memcpy(ptr, &DefaultCirclePad, sizeof(CirclePadParts_T));
	} else {
		HWCALCirclePadPart2Data_T part2;
		hwcal.ReadCalIndex(&part2, CAL_INDEX_CIRCLEPAD2);
		_ptr->ScaleX = part2.ScaleX;
		_ptr->ScaleY = part2.ScaleY;
		_ptr->MaxX = part2.MaxX;
		_ptr->MinX = part2.MinX;
		_ptr->MaxY = part2.MaxY;
		_ptr->MinY = part2.MinY;
		_ptr->Type = part2.Type;
		_ptr->Unknown[0] = part2.Unknown[0];
		_ptr->Unknown[1] = part2.Unknown[1];
		_ptr->Unknown[2] = part2.Unknown[2];
	}

	_ptr->CenterX = part1.CenterX;
	_ptr->CenterY = part1.CenterY;
}

extern "C" Result Hwcal_GetCirclePad(void* ptr, size_t size) {
	if(!ptr || size != sizeof(CirclePadParts_T)) // CFG ignored this check, we don't
		return CFG_INVALID_SIZE;

	Hwcal_GetCirclePadNoCheck(ptr);
	return 0;
}

extern "C" Result Hwcal_ResetCirclePadCfgBlk() {
	void* ptr;
	Result res;

	res = Cfg_System_GetBlkPtr(&ptr, 0x40001, sizeof(CirclePadParts_T));
	if(R_SUCCEEDED(res)) {
		Hwcal_GetCirclePadNoCheck(ptr);
		Cfg_SaveConfig();
	}

	return res;
}

static void Hwcal_GetCStickNoCheck(void* ptr) {
	ManagedHwcal_T hwcal;
	hwcal.Load();

	CStick_T* _ptr = reinterpret_cast<CStick_T*>(ptr);

	if(hwcal.Hwcal.Header.Revision < 15) {
		memset(ptr, 0, sizeof(CStick_T));
	} else {
		hwcal.ReadCalIndex(&_ptr->Data, CAL_INDEX_CSTICK);
		memset(&_ptr->Unknown[0], 0, sizeofmember(CStick_T, Unknown));
	}
}

extern "C" Result Hwcal_GetCStick(void* ptr, size_t size) {
	if(!ptr || size != sizeof(CStick_T))
		return CFG_INVALID_SIZE;

	Hwcal_GetCStickNoCheck(ptr);
	return 0;
}

extern "C" Result Hwcal_ResetCStickCfgBlk() {
	void* ptr;
	Result res;

	res = Cfg_System_GetBlkPtr(&ptr, 0x40004, sizeof(CStick_T));
	if(R_SUCCEEDED(res)) {
		Hwcal_GetCStickNoCheck(ptr);
		Cfg_SaveConfig();
	}

	return res;
}

static void Hwcal_GetOuterCamsNoCheck(void* ptr) {
	ManagedHwcal_T hwcal;
	hwcal.Load();

	OuterCamaras_T* _ptr = reinterpret_cast<OuterCamaras_T*>(ptr);

	bool agingPass = hwcal.CheckAgingFlag(CAL_INDEX_OUTERCAMS1);

	if(!agingPass || hwcal.Hwcal.Header.Revision < 2) {
		memset(&_ptr->Part1, &DummyOuterCams1, sizeof(HWCALOuterCamarasPart1Data_T));
		memset(&_ptr->Part2, 0, sizeof(HWCALOuterCamarasPart2Data_T));
	} else if(hwcal.Hwcal.Header.Revision < 4) {
		hwcal.ReadCalIndex(&_ptr->Part1, CAL_INDEX_OUTERCAMS1);
		memset(&_ptr->Part2, 0, sizeof(HWCALOuterCamarasPart2Data_T));
	} else {
		hwcal.ReadCalIndex(&_ptr->Part1, CAL_INDEX_OUTERCAMS1);
		hwcal.ReadCalIndex(&_ptr->Part2, CAL_INDEX_OUTERCAMS2);
	}
}

extern "C" Result Hwcal_GetOuterCams(void* ptr, size_t size) {
	if(!ptr || size != sizeof(OuterCamaras_T)) // CFG ignored this check, we don't
		return CFG_INVALID_SIZE;

	Hwcal_GetOuterCamsNoCheck(ptr);
	return 0;
}

extern "C" Result Hwcal_ResetOuterCamsCfgBlk() {
	void* ptr;
	Result res;

	res = Cfg_System_GetBlkPtr(&ptr, 0x60000, sizeof(OuterCamaras_T));
	if(R_SUCCEEDED(res)) {
		Hwcal_GetOuterCamsNoCheck(ptr);
		Cfg_SaveConfig();
	}

	return res;
}

extern "C" Result Hwcal_ResetGyroscopeCfgBlk() {
	void* ptr;
	Result res;

	res = Cfg_System_GetBlkPtr(&ptr, 0x40002, sizeof(HWCALGyroscopeData_T));
	if(R_SUCCEEDED(res)) {
		ManagedHwcal_T hwcal;
		hwcal.Load();
		hwcal.ReadCalIndex(ptr, CAL_INDEX_GYRO);

		Cfg_SaveConfig();
	}

	return res;
}

extern "C" Result Hwcal_ResetAccelerometerCfgBlk() {
	void* ptr;
	Result res;

	res = Cfg_System_GetBlkPtr(&ptr, 0x40003, sizeof(HWCALAccelerometerData_T));
	if(R_SUCCEEDED(res)) {
		ManagedHwcal_T hwcal;
		hwcal.Load();
		hwcal.ReadCalIndex(ptr, CAL_INDEX_ACCELEROMETER);

		Cfg_SaveConfig();
	}

	return res;
}

static void Hwcal_GetQtmNoCheck(void* ptr) {
	ManagedHwcal_T hwcal;
	hwcal.Load();

	if(hwcal.Hwcal.Header.Revision < 18) {
		memcpy(ptr, &DefaultQtm, sizeof(HWCALQtmData_T));
	} else {
		hwcal.ReadCalIndex(ptr, CAL_INDEX_QTM);
	}
}

extern "C" Result Hwcal_GetQtm(void* ptr, size_t size) {
	if(!ptr || size != sizeof(HWCALQtmData_T))
		return CFG_INVALID_SIZE;

	Hwcal_GetQtmNoCheck(ptr);
	return 0;
}

extern "C" Result Hwcal_ResetQtmCfgBlk() {
	void* ptr;
	Result res;

	res = Cfg_System_GetBlkPtr(&ptr, 0x180001, sizeof(HWCALQtmData_T));
	if(R_SUCCEEDED(res)) {
		Hwcal_GetQtmNoCheck(ptr);
		Cfg_SaveConfig();
	}

	return res;
}
