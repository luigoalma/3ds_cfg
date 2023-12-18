#pragma once
#include <3ds/types.h>
#include <storage/hwcal.h>
#include "hwcal_dummy_defaults.h"

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
	CAL_INDEX_QTM               = 23,
	CAL_INDEX_MAX               = CAL_INDEX_QTM,
	CAL_INDEX_COUNT             = CAL_INDEX_MAX+1
};

struct ManagedHwcal_T {
	HWCAL_T Hwcal;
	int Index;
	bool SystemUsesEEP:1;
	bool SystemIsDev:1;

	bool CheckAgingFlag(CALIndexes index) const;
	bool ReadCalIndex(void* ptr, CALIndexes index, bool cleanOnFail = true) const;
	bool ReadCalIndexWithDefault(void* ptr, CALIndexes index, const void* defaultValue = nullptr, int minimalRevision = 0) const;
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

void Hwcal_GetCirclePadNoCheck(ManagedHwcal_T& hwcal, void* ptr);
void Hwcal_GetCStickNoCheck(ManagedHwcal_T& hwcal, void* ptr);
void Hwcal_GetOuterCamsNoCheck(ManagedHwcal_T& hwcal, void* ptr);
