#pragma once
#include <3ds/types.h>
#include <stddef.h>
#include <assert.h>
#include <assert_helpers.h>

// HWCAL information mostly came from 3dbrew and gbatek
// figured out few little things too but mostly from those places
// so somethings may be wrong, CFG doesn't really tell much to what's what
// only whatever modules need to read it out of CFG

typedef struct {
	s16 RawX0;
	s16 RawY0;
	s16 PointX0;
	s16 PointY0;
	s16 RawX1;
	s16 RawY1;
	s16 PointX1;
	s16 PointY1;
} HWCALTouchData_T;

typedef struct {
	HWCALTouchData_T Data;
	u16 Checksum; // crc16 init 0x55AA
	u8 Padding[2];
} HWCALTouch_T;

static_assert(sizeof(HWCALTouchData_T) == 0x10);
static_assert(offsetof(HWCALTouchData_T, RawX0) == 0);
static_assert(offsetof(HWCALTouchData_T, RawY0) == 2);
static_assert(offsetof(HWCALTouchData_T, PointX0) == 4);
static_assert(offsetof(HWCALTouchData_T, PointY0) == 6);
static_assert(offsetof(HWCALTouchData_T, RawX1) == 8);
static_assert(offsetof(HWCALTouchData_T, RawY1) == 10);
static_assert(offsetof(HWCALTouchData_T, PointX1) == 12);
static_assert(offsetof(HWCALTouchData_T, PointY1) == 14);
static_assert(sizeofmember(HWCALTouchData_T, RawX0) == 2);
static_assert(sizeofmember(HWCALTouchData_T, RawY0) == 2);
static_assert(sizeofmember(HWCALTouchData_T, PointX0) == 2);
static_assert(sizeofmember(HWCALTouchData_T, PointY0) == 2);
static_assert(sizeofmember(HWCALTouchData_T, RawX1) == 2);
static_assert(sizeofmember(HWCALTouchData_T, RawY1) == 2);
static_assert(sizeofmember(HWCALTouchData_T, PointX1) == 2);
static_assert(sizeofmember(HWCALTouchData_T, PointY1) == 2);

static_assert(sizeof(HWCALTouch_T) == 0x14);
static_assert(offsetof(HWCALTouch_T, Data) == 0);
static_assert(offsetof(HWCALTouch_T, Checksum) == 16);
static_assert(offsetof(HWCALTouch_T, Padding) == 18);
static_assert(sizeofmember(HWCALTouch_T, Checksum) == 2);
static_assert(sizeofmember(HWCALTouch_T, Padding) == 2);

typedef struct {
	s16 CenterX;
	s16 CenterY;
	u8 Unk[4];
} HWCALCirclePadPart1Data_T;

typedef struct {
	HWCALCirclePadPart1Data_T Data;
	u16 Checksum; // crc16 init 0x55AA
	u8 Padding[2];
} HWCALCirclePadPart1_T;

static_assert(sizeof(HWCALCirclePadPart1Data_T) == 0x8);
static_assert(offsetof(HWCALCirclePadPart1Data_T, CenterX) == 0);
static_assert(offsetof(HWCALCirclePadPart1Data_T, CenterY) == 2);
static_assert(offsetof(HWCALCirclePadPart1Data_T, Unk) == 4);
static_assert(sizeofmember(HWCALCirclePadPart1Data_T, CenterX) == 2);
static_assert(sizeofmember(HWCALCirclePadPart1Data_T, CenterY) == 2);
static_assert(sizeofmember(HWCALCirclePadPart1Data_T, Unk) == 4);

static_assert(sizeof(HWCALCirclePadPart1_T) == 0xC);
static_assert(offsetof(HWCALCirclePadPart1_T, Data) == 0);
static_assert(offsetof(HWCALCirclePadPart1_T, Checksum) == 8);
static_assert(offsetof(HWCALCirclePadPart1_T, Padding) == 10);
static_assert(sizeofmember(HWCALCirclePadPart1_T, Checksum) == 2);
static_assert(sizeofmember(HWCALCirclePadPart1_T, Padding) == 2);

typedef struct {
	u8 FlickerTop;
	u8 FlickerBottom;
} HWCALScreenFlickerData_T;

typedef struct {
	HWCALScreenFlickerData_T Data;
	u8 FlippedBytes[sizeof(HWCALScreenFlickerData_T)];
} HWCALScreenFlicker_T;

static_assert(sizeof(HWCALScreenFlickerData_T) == 0x2);
static_assert(offsetof(HWCALScreenFlickerData_T, FlickerTop) == 0);
static_assert(offsetof(HWCALScreenFlickerData_T, FlickerBottom) == 1);
static_assert(sizeofmember(HWCALScreenFlickerData_T, FlickerTop) == 1);
static_assert(sizeofmember(HWCALScreenFlickerData_T, FlickerBottom) == 1);

static_assert(sizeof(HWCALScreenFlicker_T) == 0x4);
static_assert(offsetof(HWCALScreenFlicker_T, Data) == 0);
static_assert(offsetof(HWCALScreenFlicker_T, FlippedBytes) == 2);
static_assert(sizeofmember(HWCALScreenFlicker_T, FlippedBytes) == 2);

typedef struct {
	u8 CompensationValue;
} HWCALRtcCompensationData_T;

typedef struct {
	HWCALRtcCompensationData_T Data;
	u8 FlippedBytes[sizeof(HWCALRtcCompensationData_T)];
	u8 Padding[2];
} HWCALRtcCompensation_T;

static_assert(sizeof(HWCALRtcCompensationData_T) == 0x1);
static_assert(offsetof(HWCALRtcCompensationData_T, CompensationValue) == 0);
static_assert(sizeofmember(HWCALRtcCompensationData_T, CompensationValue) == 1);

static_assert(sizeof(HWCALRtcCompensation_T) == 0x4);
static_assert(offsetof(HWCALRtcCompensation_T, Data) == 0);
static_assert(offsetof(HWCALRtcCompensation_T, FlippedBytes) == 1);
static_assert(offsetof(HWCALRtcCompensation_T, Padding) == 2);
static_assert(sizeofmember(HWCALRtcCompensation_T, FlippedBytes) == 1);
static_assert(sizeofmember(HWCALRtcCompensation_T, Padding) == 2);

typedef struct {
	u8 CorrectionValue;
} HWCALRtcCorrectionData_T;

typedef struct {
	HWCALRtcCorrectionData_T Data;
	u8 FlippedBytes[sizeof(HWCALRtcCorrectionData_T)];
	u8 Padding[6];
} HWCALRtcCorrection_T;

static_assert(sizeof(HWCALRtcCorrectionData_T) == 0x1);
static_assert(offsetof(HWCALRtcCorrectionData_T, CorrectionValue) == 0);
static_assert(sizeofmember(HWCALRtcCorrectionData_T, CorrectionValue) == 1);

static_assert(sizeof(HWCALRtcCorrection_T) == 0x8);
static_assert(offsetof(HWCALRtcCorrection_T, Data) == 0);
static_assert(offsetof(HWCALRtcCorrection_T, FlippedBytes) == 1);
static_assert(offsetof(HWCALRtcCorrection_T, Padding) == 2);
static_assert(sizeofmember(HWCALRtcCorrection_T, FlippedBytes) == 1);
static_assert(sizeofmember(HWCALRtcCorrection_T, Padding) == 6);

typedef struct PACKED HWCALOuterCamarasPart1Data_T {
	u32 Flags;
	float Scale;
	float RotationZ;
	float TranslationX;
	float TranslationY;
	float RotationX;
	float RotationY;
	float ViewAngleRight;
	float ViewAngleLeft;
	float ChartDistance;
	float CameraDistance;
	s16 ImageWidth;
	s16 ImageHeight;
	u8 Reserved[16];
	u8 Unknown[64];
	s16 AeBaseTarget;
	s16 kRL;
	s16 kGL;
	s16 kBL;
	s16 CcmPosition;
} HWCALOuterCamarasPart1Data_T;

typedef struct {
	HWCALOuterCamarasPart1Data_T Data;
	u16 Checksum; // crc16 init 0x55AA
} HWCALOuterCamarasPart1_T;

static_assert(sizeof(HWCALOuterCamarasPart1Data_T) == 0x8A);
static_assert(offsetof(HWCALOuterCamarasPart1Data_T, Flags) == 0);
static_assert(offsetof(HWCALOuterCamarasPart1Data_T, Scale) == 4);
static_assert(offsetof(HWCALOuterCamarasPart1Data_T, RotationZ) == 8);
static_assert(offsetof(HWCALOuterCamarasPart1Data_T, TranslationX) == 12);
static_assert(offsetof(HWCALOuterCamarasPart1Data_T, TranslationY) == 16);
static_assert(offsetof(HWCALOuterCamarasPart1Data_T, RotationX) == 20);
static_assert(offsetof(HWCALOuterCamarasPart1Data_T, RotationY) == 24);
static_assert(offsetof(HWCALOuterCamarasPart1Data_T, ViewAngleRight) == 28);
static_assert(offsetof(HWCALOuterCamarasPart1Data_T, ViewAngleLeft) == 32);
static_assert(offsetof(HWCALOuterCamarasPart1Data_T, ChartDistance) == 36);
static_assert(offsetof(HWCALOuterCamarasPart1Data_T, CameraDistance) == 40);
static_assert(offsetof(HWCALOuterCamarasPart1Data_T, ImageWidth) == 44);
static_assert(offsetof(HWCALOuterCamarasPart1Data_T, ImageHeight) == 46);
static_assert(offsetof(HWCALOuterCamarasPart1Data_T, Reserved) == 48);
static_assert(offsetof(HWCALOuterCamarasPart1Data_T, Unknown) == 64);
static_assert(offsetof(HWCALOuterCamarasPart1Data_T, AeBaseTarget) == 128);
static_assert(offsetof(HWCALOuterCamarasPart1Data_T, kRL) == 130);
static_assert(offsetof(HWCALOuterCamarasPart1Data_T, kGL) == 132);
static_assert(offsetof(HWCALOuterCamarasPart1Data_T, kBL) == 134);
static_assert(offsetof(HWCALOuterCamarasPart1Data_T, CcmPosition) == 136);
static_assert(sizeofmember(HWCALOuterCamarasPart1Data_T, Flags) == 4);
static_assert(sizeofmember(HWCALOuterCamarasPart1Data_T, Scale) == 4);
static_assert(sizeofmember(HWCALOuterCamarasPart1Data_T, RotationZ) == 4);
static_assert(sizeofmember(HWCALOuterCamarasPart1Data_T, TranslationX) == 4);
static_assert(sizeofmember(HWCALOuterCamarasPart1Data_T, TranslationY) == 4);
static_assert(sizeofmember(HWCALOuterCamarasPart1Data_T, RotationX) == 4);
static_assert(sizeofmember(HWCALOuterCamarasPart1Data_T, RotationY) == 4);
static_assert(sizeofmember(HWCALOuterCamarasPart1Data_T, ViewAngleRight) == 4);
static_assert(sizeofmember(HWCALOuterCamarasPart1Data_T, ViewAngleLeft) == 4);
static_assert(sizeofmember(HWCALOuterCamarasPart1Data_T, ChartDistance) == 4);
static_assert(sizeofmember(HWCALOuterCamarasPart1Data_T, CameraDistance) == 4);
static_assert(sizeofmember(HWCALOuterCamarasPart1Data_T, ImageWidth) == 2);
static_assert(sizeofmember(HWCALOuterCamarasPart1Data_T, ImageHeight) == 2);
static_assert(sizeofmember(HWCALOuterCamarasPart1Data_T, Reserved) == 16);
static_assert(sizeofmember(HWCALOuterCamarasPart1Data_T, Unknown) == 64);
static_assert(sizeofmember(HWCALOuterCamarasPart1Data_T, AeBaseTarget) == 2);
static_assert(sizeofmember(HWCALOuterCamarasPart1Data_T, kRL) == 2);
static_assert(sizeofmember(HWCALOuterCamarasPart1Data_T, kGL) == 2);
static_assert(sizeofmember(HWCALOuterCamarasPart1Data_T, kBL) == 2);
static_assert(sizeofmember(HWCALOuterCamarasPart1Data_T, CcmPosition) == 2);

static_assert(sizeof(HWCALOuterCamarasPart1_T) == 0x8C);
static_assert(offsetof(HWCALOuterCamarasPart1_T, Data) == 0);
static_assert(offsetof(HWCALOuterCamarasPart1_T, Checksum) == 138);
static_assert(sizeofmember(HWCALOuterCamarasPart1_T, Checksum) == 2);

typedef struct {
	s16 ZeroX;
	s16 PlusX;
	s16 MinusX;
	s16 ZeroY;
	s16 PlusY;
	s16 MinusY;
	s16 ZeroZ;
	s16 PlusZ;
	s16 MinusZ;
} HWCALGyroscopeData_T;

typedef struct {
	HWCALGyroscopeData_T Data;
	u16 Checksum; // crc16 init 0x55AA
} HWCALGyroscope_T;

static_assert(sizeof(HWCALGyroscopeData_T) == 0x12);
static_assert(offsetof(HWCALGyroscopeData_T, ZeroX) == 0);
static_assert(offsetof(HWCALGyroscopeData_T, PlusX) == 2);
static_assert(offsetof(HWCALGyroscopeData_T, MinusX) == 4);
static_assert(offsetof(HWCALGyroscopeData_T, ZeroY) == 6);
static_assert(offsetof(HWCALGyroscopeData_T, PlusY) == 8);
static_assert(offsetof(HWCALGyroscopeData_T, MinusY) == 10);
static_assert(offsetof(HWCALGyroscopeData_T, ZeroZ) == 12);
static_assert(offsetof(HWCALGyroscopeData_T, PlusZ) == 14);
static_assert(offsetof(HWCALGyroscopeData_T, MinusZ) == 16);
static_assert(sizeofmember(HWCALGyroscopeData_T, ZeroX) == 2);
static_assert(sizeofmember(HWCALGyroscopeData_T, PlusX) == 2);
static_assert(sizeofmember(HWCALGyroscopeData_T, MinusX) == 2);
static_assert(sizeofmember(HWCALGyroscopeData_T, ZeroY) == 2);
static_assert(sizeofmember(HWCALGyroscopeData_T, PlusY) == 2);
static_assert(sizeofmember(HWCALGyroscopeData_T, MinusY) == 2);
static_assert(sizeofmember(HWCALGyroscopeData_T, ZeroZ) == 2);
static_assert(sizeofmember(HWCALGyroscopeData_T, PlusZ) == 2);
static_assert(sizeofmember(HWCALGyroscopeData_T, MinusZ) == 2);

static_assert(sizeof(HWCALGyroscope_T) == 0x14);
static_assert(offsetof(HWCALGyroscope_T, Data) == 0);
static_assert(offsetof(HWCALGyroscope_T, Checksum) == 18);
static_assert(sizeofmember(HWCALGyroscope_T, Checksum) == 2);

typedef struct {
	s16 OffsetX;
	s16 ScaleX;
	s16 OffsetY;
	s16 ScaleY;
	s16 OffsetZ;
	s16 ScaleZ;
} HWCALAccelerometerData_T;

typedef struct {
	HWCALAccelerometerData_T Data;
	u16 Checksum; // crc16 init 0x55AA
	u8 Padding[2];
} HWCALAccelerometer_T;

static_assert(sizeof(HWCALAccelerometerData_T) == 0xC);
static_assert(offsetof(HWCALAccelerometerData_T, OffsetX) == 0);
static_assert(offsetof(HWCALAccelerometerData_T, ScaleX) == 2);
static_assert(offsetof(HWCALAccelerometerData_T, OffsetY) == 4);
static_assert(offsetof(HWCALAccelerometerData_T, ScaleY) == 6);
static_assert(offsetof(HWCALAccelerometerData_T, OffsetZ) == 8);
static_assert(offsetof(HWCALAccelerometerData_T, ScaleZ) == 10);
static_assert(sizeofmember(HWCALAccelerometerData_T, OffsetX) == 2);
static_assert(sizeofmember(HWCALAccelerometerData_T, ScaleX) == 2);
static_assert(sizeofmember(HWCALAccelerometerData_T, OffsetY) == 2);
static_assert(sizeofmember(HWCALAccelerometerData_T, ScaleY) == 2);
static_assert(sizeofmember(HWCALAccelerometerData_T, OffsetZ) == 2);
static_assert(sizeofmember(HWCALAccelerometerData_T, ScaleZ) == 2);

static_assert(sizeof(HWCALAccelerometer_T) == 0x10);
static_assert(offsetof(HWCALAccelerometer_T, Data) == 0);
static_assert(offsetof(HWCALAccelerometer_T, Checksum) == 12);
static_assert(offsetof(HWCALAccelerometer_T, Padding) == 14);
static_assert(sizeofmember(HWCALAccelerometer_T, Checksum) == 2);
static_assert(sizeofmember(HWCALAccelerometer_T, Padding) == 2);

typedef struct {
	u8 DriverGainHP;
	u8 DriverGainSP;
	u8 AnalogVolumeHP;
	u8 AnalogVolumeSP;
	s8 ShutterVolume0;
	s8 ShutterVolume1;
	u8 MicrophoneBias;
	u8 QuickCharge;
	u8 PGA_GAIN;
	u8 Reserved1[3];
	s16 FilterHP32[15];
	s16 FilterHP47[15];
	s16 FilterSP32[15];
	s16 FilterSP47[15];
	s16 FilterMic32[28];
	s16 FilterMic47[28];
	s16 FilterFree[28];
	u8 AnalogInterval;
	u8 AnalogStabilize;
	u8 AnalogPrecharge;
	u8 AnalogSense;
	u8 AnalogDebounce;
	u8 Analog_XP_Pullup;
	u8 YM_Driver;
	u8 Reserved2;
} HWCALCodecData_T;

typedef struct {
	HWCALCodecData_T Data;
	u16 Checksum; // crc16 init 0x55AA
	u8 Padding[2];
} HWCALCodec_T;

static_assert(sizeof(HWCALCodecData_T) == 0x134);
static_assert(offsetof(HWCALCodecData_T, DriverGainHP) == 0);
static_assert(offsetof(HWCALCodecData_T, DriverGainSP) == 1);
static_assert(offsetof(HWCALCodecData_T, AnalogVolumeHP) == 2);
static_assert(offsetof(HWCALCodecData_T, AnalogVolumeSP) == 3);
static_assert(offsetof(HWCALCodecData_T, ShutterVolume0) == 4);
static_assert(offsetof(HWCALCodecData_T, ShutterVolume1) == 5);
static_assert(offsetof(HWCALCodecData_T, MicrophoneBias) == 6);
static_assert(offsetof(HWCALCodecData_T, QuickCharge) == 7);
static_assert(offsetof(HWCALCodecData_T, PGA_GAIN) == 8);
static_assert(offsetof(HWCALCodecData_T, Reserved1) == 9);
static_assert(offsetof(HWCALCodecData_T, FilterHP32) == 12);
static_assert(offsetof(HWCALCodecData_T, FilterHP47) == 42);
static_assert(offsetof(HWCALCodecData_T, FilterSP32) == 72);
static_assert(offsetof(HWCALCodecData_T, FilterSP47) == 102);
static_assert(offsetof(HWCALCodecData_T, FilterMic32) == 132);
static_assert(offsetof(HWCALCodecData_T, FilterMic47) == 188);
static_assert(offsetof(HWCALCodecData_T, FilterFree) == 244);
static_assert(offsetof(HWCALCodecData_T, AnalogInterval) == 300);
static_assert(offsetof(HWCALCodecData_T, AnalogStabilize) == 301);
static_assert(offsetof(HWCALCodecData_T, AnalogPrecharge) == 302);
static_assert(offsetof(HWCALCodecData_T, AnalogSense) == 303);
static_assert(offsetof(HWCALCodecData_T, AnalogDebounce) == 304);
static_assert(offsetof(HWCALCodecData_T, Analog_XP_Pullup) == 305);
static_assert(offsetof(HWCALCodecData_T, YM_Driver) == 306);
static_assert(offsetof(HWCALCodecData_T, Reserved2) == 307);
static_assert(sizeofmember(HWCALCodecData_T, DriverGainHP) == 1);
static_assert(sizeofmember(HWCALCodecData_T, DriverGainSP) == 1);
static_assert(sizeofmember(HWCALCodecData_T, AnalogVolumeHP) == 1);
static_assert(sizeofmember(HWCALCodecData_T, AnalogVolumeSP) == 1);
static_assert(sizeofmember(HWCALCodecData_T, ShutterVolume0) == 1);
static_assert(sizeofmember(HWCALCodecData_T, ShutterVolume1) == 1);
static_assert(sizeofmember(HWCALCodecData_T, MicrophoneBias) == 1);
static_assert(sizeofmember(HWCALCodecData_T, QuickCharge) == 1);
static_assert(sizeofmember(HWCALCodecData_T, PGA_GAIN) == 1);
static_assert(sizeofmember(HWCALCodecData_T, Reserved1) == 3);
static_assert(sizeofmember(HWCALCodecData_T, FilterHP32) == 30);
static_assert(sizeofmember(HWCALCodecData_T, FilterHP47) == 30);
static_assert(sizeofmember(HWCALCodecData_T, FilterSP32) == 30);
static_assert(sizeofmember(HWCALCodecData_T, FilterSP47) == 30);
static_assert(sizeofmember(HWCALCodecData_T, FilterMic32) == 56);
static_assert(sizeofmember(HWCALCodecData_T, FilterMic47) == 56);
static_assert(sizeofmember(HWCALCodecData_T, FilterFree) == 56);
static_assert(sizeofmember(HWCALCodecData_T, AnalogInterval) == 1);
static_assert(sizeofmember(HWCALCodecData_T, AnalogStabilize) == 1);
static_assert(sizeofmember(HWCALCodecData_T, AnalogPrecharge) == 1);
static_assert(sizeofmember(HWCALCodecData_T, AnalogSense) == 1);
static_assert(sizeofmember(HWCALCodecData_T, AnalogDebounce) == 1);
static_assert(sizeofmember(HWCALCodecData_T, Analog_XP_Pullup) == 1);
static_assert(sizeofmember(HWCALCodecData_T, YM_Driver) == 1);
static_assert(sizeofmember(HWCALCodecData_T, Reserved2) == 1);

static_assert(sizeof(HWCALCodec_T) == 0x138);
static_assert(offsetof(HWCALCodec_T, Data) == 0);
static_assert(offsetof(HWCALCodec_T, Checksum) == 308);
static_assert(offsetof(HWCALCodec_T, Padding) == 310);
static_assert(sizeofmember(HWCALCodec_T, Checksum) == 2);
static_assert(sizeofmember(HWCALCodec_T, Padding) == 2);

typedef struct {
	u16 VisibleFactor;
	u16 IRFactor;
} HWCALPitData_T;

typedef struct {
	HWCALPitData_T Data;
	u16 AgingFlag;
	u16 Checksum; // crc16 init 0x55AA
} HWCALPit_T;

static_assert(sizeof(HWCALPitData_T) == 0x4);
static_assert(offsetof(HWCALPitData_T, VisibleFactor) == 0);
static_assert(offsetof(HWCALPitData_T, IRFactor) == 2);
static_assert(sizeofmember(HWCALPitData_T, VisibleFactor) == 2);
static_assert(sizeofmember(HWCALPitData_T, IRFactor) == 2);

static_assert(sizeof(HWCALPit_T) == 0x8);
static_assert(offsetof(HWCALPit_T, Data) == 0);
static_assert(offsetof(HWCALPit_T, AgingFlag) == 4);
static_assert(offsetof(HWCALPit_T, Checksum) == 6);
static_assert(sizeofmember(HWCALPit_T, AgingFlag) == 2);
static_assert(sizeofmember(HWCALPit_T, Checksum) == 2);

typedef struct {
	u16 SpecialFilter[256];
	u32 IIRSurroundFilter[5];
} HWCALSound3DFilterData_T;

typedef struct {
	HWCALSound3DFilterData_T Data;
	u16 Checksum; // crc16 init 0x55AA
	u8 Padding[10];
} HWCALSound3DFilter_T;

static_assert(sizeof(HWCALSound3DFilterData_T) == 0x214);
static_assert(offsetof(HWCALSound3DFilterData_T, SpecialFilter) == 0);
static_assert(offsetof(HWCALSound3DFilterData_T, IIRSurroundFilter) == 512);
static_assert(sizeofmember(HWCALSound3DFilterData_T, SpecialFilter) == 512);
static_assert(sizeofmember(HWCALSound3DFilterData_T, IIRSurroundFilter) == 20);

static_assert(sizeof(HWCALSound3DFilter_T) == 0x220);
static_assert(offsetof(HWCALSound3DFilter_T, Data) == 0);
static_assert(offsetof(HWCALSound3DFilter_T, Checksum) == 532);
static_assert(offsetof(HWCALSound3DFilter_T, Padding) == 534);
static_assert(sizeofmember(HWCALSound3DFilter_T, Checksum) == 2);
static_assert(sizeofmember(HWCALSound3DFilter_T, Padding) == 10);

typedef struct {
	u32 DitherPattern;
	s16 StartX;
	s16 StartY;
	u16 SizeX;
	u16 SizeY;
	s16 GTHRatio;
	u8 DitherMode;
	u8 MinRS;
	u8 MaxRS;
	u8 MinGTH;
	u8 MinMax;
	u8 ExMax;
	u8 Inertia;
	u8 LutListRS[9];
	u8 Reserved[2];
} HWCALLcdPowerSaveData_T;

typedef struct {
	HWCALLcdPowerSaveData_T Data;
	u16 Checksum; // crc16 init 0x55AA
	u8 Padding[14];
} HWCALLcdPowerSave_T;

static_assert(sizeof(HWCALLcdPowerSaveData_T) == 0x20);
static_assert(offsetof(HWCALLcdPowerSaveData_T, DitherPattern) == 0);
static_assert(offsetof(HWCALLcdPowerSaveData_T, StartX) == 4);
static_assert(offsetof(HWCALLcdPowerSaveData_T, StartY) == 6);
static_assert(offsetof(HWCALLcdPowerSaveData_T, SizeX) == 8);
static_assert(offsetof(HWCALLcdPowerSaveData_T, SizeY) == 10);
static_assert(offsetof(HWCALLcdPowerSaveData_T, GTHRatio) == 12);
static_assert(offsetof(HWCALLcdPowerSaveData_T, DitherMode) == 14);
static_assert(offsetof(HWCALLcdPowerSaveData_T, MinRS) == 15);
static_assert(offsetof(HWCALLcdPowerSaveData_T, MaxRS) == 16);
static_assert(offsetof(HWCALLcdPowerSaveData_T, MinGTH) == 17);
static_assert(offsetof(HWCALLcdPowerSaveData_T, MinMax) == 18);
static_assert(offsetof(HWCALLcdPowerSaveData_T, ExMax) == 19);
static_assert(offsetof(HWCALLcdPowerSaveData_T, Inertia) == 20);
static_assert(offsetof(HWCALLcdPowerSaveData_T, LutListRS) == 21);
static_assert(offsetof(HWCALLcdPowerSaveData_T, Reserved) == 30);
static_assert(sizeofmember(HWCALLcdPowerSaveData_T, DitherPattern) == 4);
static_assert(sizeofmember(HWCALLcdPowerSaveData_T, StartX) == 2);
static_assert(sizeofmember(HWCALLcdPowerSaveData_T, StartY) == 2);
static_assert(sizeofmember(HWCALLcdPowerSaveData_T, SizeX) == 2);
static_assert(sizeofmember(HWCALLcdPowerSaveData_T, SizeY) == 2);
static_assert(sizeofmember(HWCALLcdPowerSaveData_T, GTHRatio) == 2);
static_assert(sizeofmember(HWCALLcdPowerSaveData_T, DitherMode) == 1);
static_assert(sizeofmember(HWCALLcdPowerSaveData_T, MinRS) == 1);
static_assert(sizeofmember(HWCALLcdPowerSaveData_T, MaxRS) == 1);
static_assert(sizeofmember(HWCALLcdPowerSaveData_T, MinGTH) == 1);
static_assert(sizeofmember(HWCALLcdPowerSaveData_T, MinMax) == 1);
static_assert(sizeofmember(HWCALLcdPowerSaveData_T, ExMax) == 1);
static_assert(sizeofmember(HWCALLcdPowerSaveData_T, Inertia) == 1);
static_assert(sizeofmember(HWCALLcdPowerSaveData_T, LutListRS) == 9);
static_assert(sizeofmember(HWCALLcdPowerSaveData_T, Reserved) == 2);

static_assert(sizeof(HWCALLcdPowerSave_T) == 0x30);
static_assert(offsetof(HWCALLcdPowerSave_T, Data) == 0);
static_assert(offsetof(HWCALLcdPowerSave_T, Checksum) == 32);
static_assert(offsetof(HWCALLcdPowerSave_T, Padding) == 34);
static_assert(sizeofmember(HWCALLcdPowerSave_T, Checksum) == 2);
static_assert(sizeofmember(HWCALLcdPowerSave_T, Padding) == 14);

typedef struct {
	float PupillaryDistanceInMm;
	float DistanceEyesAndUpperScreenInMm;
	float ScreenWidthInMm;
	float ScreenHeightInMm;
	float Unknown[4];
} HWCALLcdStereoscopicData_T;

typedef struct {
	HWCALLcdStereoscopicData_T Data;
	u16 Checksum; // crc16 init 0x55AA
	u8 Padding[14];
} HWCALLcdStereoscopic_T;

static_assert(sizeof(HWCALLcdStereoscopicData_T) == 0x20);
static_assert(offsetof(HWCALLcdStereoscopicData_T, PupillaryDistanceInMm) == 0);
static_assert(offsetof(HWCALLcdStereoscopicData_T, DistanceEyesAndUpperScreenInMm) == 4);
static_assert(offsetof(HWCALLcdStereoscopicData_T, ScreenWidthInMm) == 8);
static_assert(offsetof(HWCALLcdStereoscopicData_T, ScreenHeightInMm) == 12);
static_assert(offsetof(HWCALLcdStereoscopicData_T, Unknown) == 16);
static_assert(sizeofmember(HWCALLcdStereoscopicData_T, PupillaryDistanceInMm) == 4);
static_assert(sizeofmember(HWCALLcdStereoscopicData_T, DistanceEyesAndUpperScreenInMm) == 4);
static_assert(sizeofmember(HWCALLcdStereoscopicData_T, ScreenWidthInMm) == 4);
static_assert(sizeofmember(HWCALLcdStereoscopicData_T, ScreenHeightInMm) == 4);
static_assert(sizeofmember(HWCALLcdStereoscopicData_T, Unknown) == 16);

static_assert(sizeof(HWCALLcdStereoscopic_T) == 0x30);
static_assert(offsetof(HWCALLcdStereoscopic_T, Data) == 0);
static_assert(offsetof(HWCALLcdStereoscopic_T, Checksum) == 32);
static_assert(offsetof(HWCALLcdStereoscopic_T, Padding) == 34);
static_assert(sizeofmember(HWCALLcdStereoscopic_T, Checksum) == 2);
static_assert(sizeofmember(HWCALLcdStereoscopic_T, Padding) == 14);

typedef struct {
	float Coefficients[3][3];
	u8 NumLevels;
	u8 Unknown;
	u16 Brightnesses[7];
	u16 BaseDivisor;
	u16 MinimumBrightnessHw;
} HWCALBacklightPwmData_T;

typedef struct {
	HWCALBacklightPwmData_T Data;
	u16 Checksum; // crc16 init 0x55AA
	u8 Padding[6];
} HWCALBacklightPwm_T;

static_assert(sizeof(HWCALBacklightPwmData_T) == 0x38);
static_assert(offsetof(HWCALBacklightPwmData_T, Coefficients) == 0);
static_assert(offsetof(HWCALBacklightPwmData_T, NumLevels) == 36);
static_assert(offsetof(HWCALBacklightPwmData_T, Unknown) == 37);
static_assert(offsetof(HWCALBacklightPwmData_T, Brightnesses) == 38);
static_assert(offsetof(HWCALBacklightPwmData_T, BaseDivisor) == 52);
static_assert(offsetof(HWCALBacklightPwmData_T, MinimumBrightnessHw) == 54);
static_assert(sizeofmember(HWCALBacklightPwmData_T, Coefficients) == 36);
static_assert(sizeofmember(HWCALBacklightPwmData_T, NumLevels) == 1);
static_assert(sizeofmember(HWCALBacklightPwmData_T, Unknown) == 1);
static_assert(sizeofmember(HWCALBacklightPwmData_T, Brightnesses) == 14);
static_assert(sizeofmember(HWCALBacklightPwmData_T, BaseDivisor) == 2);
static_assert(sizeofmember(HWCALBacklightPwmData_T, MinimumBrightnessHw) == 2);

static_assert(sizeof(HWCALBacklightPwm_T) == 0x40);
static_assert(offsetof(HWCALBacklightPwm_T, Data) == 0);
static_assert(offsetof(HWCALBacklightPwm_T, Checksum) == 56);
static_assert(offsetof(HWCALBacklightPwm_T, Padding) == 58);
static_assert(sizeofmember(HWCALBacklightPwm_T, Checksum) == 2);
static_assert(sizeofmember(HWCALBacklightPwm_T, Padding) == 6);

typedef struct {
	float ScaleX;
	float ScaleY;
	s16 MaxX;
	s16 MinX;
	s16 MaxY;
	s16 MinY;
	s16 Type;
	u16 Unknown[3];
} HWCALCirclePadPart2Data_T;

typedef struct {
	HWCALCirclePadPart2Data_T Data;
	u16 Checksum; // crc16 init 0x55AA
	u8 Padding[6];
} HWCALCirclePadPart2_T;

static_assert(sizeof(HWCALCirclePadPart2Data_T) == 0x18);
static_assert(offsetof(HWCALCirclePadPart2Data_T, ScaleX) == 0);
static_assert(offsetof(HWCALCirclePadPart2Data_T, ScaleY) == 4);
static_assert(offsetof(HWCALCirclePadPart2Data_T, MaxX) == 8);
static_assert(offsetof(HWCALCirclePadPart2Data_T, MinX) == 10);
static_assert(offsetof(HWCALCirclePadPart2Data_T, MaxY) == 12);
static_assert(offsetof(HWCALCirclePadPart2Data_T, MinY) == 14);
static_assert(offsetof(HWCALCirclePadPart2Data_T, Type) == 16);
static_assert(offsetof(HWCALCirclePadPart2Data_T, Unknown) == 18);
static_assert(sizeofmember(HWCALCirclePadPart2Data_T, ScaleX) == 4);
static_assert(sizeofmember(HWCALCirclePadPart2Data_T, ScaleY) == 4);
static_assert(sizeofmember(HWCALCirclePadPart2Data_T, MaxX) == 2);
static_assert(sizeofmember(HWCALCirclePadPart2Data_T, MinX) == 2);
static_assert(sizeofmember(HWCALCirclePadPart2Data_T, MaxY) == 2);
static_assert(sizeofmember(HWCALCirclePadPart2Data_T, MinY) == 2);
static_assert(sizeofmember(HWCALCirclePadPart2Data_T, Type) == 2);
static_assert(sizeofmember(HWCALCirclePadPart2Data_T, Unknown) == 6);

static_assert(sizeof(HWCALCirclePadPart2_T) == 0x20);
static_assert(offsetof(HWCALCirclePadPart2_T, Data) == 0);
static_assert(offsetof(HWCALCirclePadPart2_T, Checksum) == 24);
static_assert(offsetof(HWCALCirclePadPart2_T, Padding) == 26);
static_assert(sizeofmember(HWCALCirclePadPart2_T, Checksum) == 2);
static_assert(sizeofmember(HWCALCirclePadPart2_T, Padding) == 6);

typedef struct {
	u16 Unknown[6];
} HWCALOuterCamarasPart2Data_T;

typedef struct {
	HWCALOuterCamarasPart2Data_T Data;
	u16 Checksum; // crc16 init 0x55AA
	u8 Padding[2];
} HWCALOuterCamarasPart2_T;

static_assert(sizeof(HWCALOuterCamarasPart2Data_T) == 0xC);
static_assert(offsetof(HWCALOuterCamarasPart2Data_T, Unknown) == 0);
static_assert(sizeofmember(HWCALOuterCamarasPart2Data_T, Unknown) == 12);

static_assert(sizeof(HWCALOuterCamarasPart2_T) == 0x10);
static_assert(offsetof(HWCALOuterCamarasPart2_T, Data) == 0);
static_assert(offsetof(HWCALOuterCamarasPart2_T, Checksum) == 12);
static_assert(offsetof(HWCALOuterCamarasPart2_T, Padding) == 14);
static_assert(sizeofmember(HWCALOuterCamarasPart2_T, Checksum) == 2);
static_assert(sizeofmember(HWCALOuterCamarasPart2_T, Padding) == 2);

typedef struct {
	s16 SVR2Min;
	s16 SVR2Max;
	s16 VolumeSliderMin;
	s16 VolumeSliderMax;
} HWCALSlidersData_T;

typedef struct {
	HWCALSlidersData_T Data;
	u16 AgingFlag;
	u16 Checksum; // crc16 init 0x55AA
	u8 Padding[4];
} HWCALSliders_T;

static_assert(sizeof(HWCALSlidersData_T) == 0x8);
static_assert(offsetof(HWCALSlidersData_T, SVR2Min) == 0);
static_assert(offsetof(HWCALSlidersData_T, SVR2Max) == 2);
static_assert(offsetof(HWCALSlidersData_T, VolumeSliderMin) == 4);
static_assert(offsetof(HWCALSlidersData_T, VolumeSliderMax) == 6);
static_assert(sizeofmember(HWCALSlidersData_T, SVR2Min) == 2);
static_assert(sizeofmember(HWCALSlidersData_T, SVR2Max) == 2);
static_assert(sizeofmember(HWCALSlidersData_T, VolumeSliderMin) == 2);
static_assert(sizeofmember(HWCALSlidersData_T, VolumeSliderMax) == 2);

static_assert(sizeof(HWCALSliders_T) == 0x10);
static_assert(offsetof(HWCALSliders_T, Data) == 0);
static_assert(offsetof(HWCALSliders_T, AgingFlag) == 8);
static_assert(offsetof(HWCALSliders_T, Checksum) == 10);
static_assert(offsetof(HWCALSliders_T, Padding) == 12);
static_assert(sizeofmember(HWCALSliders_T, AgingFlag) == 2);
static_assert(sizeofmember(HWCALSliders_T, Checksum) == 2);
static_assert(sizeofmember(HWCALSliders_T, Padding) == 4);

typedef struct {
	u8 To2D;
	u8 To3D;
} HWCALLcdModeDelayData_T;

typedef struct {
	HWCALLcdModeDelayData_T Data;
	u16 AgingFlag;
	u16 Checksum; // crc16 init 0x55AA
	u8 Padding[10];
} HWCALLcdModeDelay_T;

static_assert(sizeof(HWCALLcdModeDelayData_T) == 0x2);
static_assert(offsetof(HWCALLcdModeDelayData_T, To2D) == 0);
static_assert(offsetof(HWCALLcdModeDelayData_T, To3D) == 1);
static_assert(sizeofmember(HWCALLcdModeDelayData_T, To2D) == 1);
static_assert(sizeofmember(HWCALLcdModeDelayData_T, To3D) == 1);

static_assert(sizeof(HWCALLcdModeDelay_T) == 0x10);
static_assert(offsetof(HWCALLcdModeDelay_T, Data) == 0);
static_assert(offsetof(HWCALLcdModeDelay_T, AgingFlag) == 2);
static_assert(offsetof(HWCALLcdModeDelay_T, Checksum) == 4);
static_assert(offsetof(HWCALLcdModeDelay_T, Padding) == 6);
static_assert(sizeofmember(HWCALLcdModeDelay_T, AgingFlag) == 2);
static_assert(sizeofmember(HWCALLcdModeDelay_T, Checksum) == 2);
static_assert(sizeofmember(HWCALLcdModeDelay_T, Padding) == 10);

typedef struct {
	u8 Unknown[8];
} HWCALMicrophoneEchoCancellationData_T;

typedef struct {
	HWCALMicrophoneEchoCancellationData_T Data;
	u16 AgingFlag;
	u16 Checksum; // crc16 init 0x55AA
	u8 Padding[4];
} HWCALMicrophoneEchoCancellation_T;

static_assert(sizeof(HWCALMicrophoneEchoCancellationData_T) == 0x8);
static_assert(offsetof(HWCALMicrophoneEchoCancellationData_T, Unknown) == 0);
static_assert(sizeofmember(HWCALMicrophoneEchoCancellationData_T, Unknown) == 8);

static_assert(sizeof(HWCALMicrophoneEchoCancellation_T) == 0x10);
static_assert(offsetof(HWCALMicrophoneEchoCancellation_T, Data) == 0);
static_assert(offsetof(HWCALMicrophoneEchoCancellation_T, AgingFlag) == 8);
static_assert(offsetof(HWCALMicrophoneEchoCancellation_T, Checksum) == 10);
static_assert(offsetof(HWCALMicrophoneEchoCancellation_T, Padding) == 12);
static_assert(sizeofmember(HWCALMicrophoneEchoCancellation_T, AgingFlag) == 2);
static_assert(sizeofmember(HWCALMicrophoneEchoCancellation_T, Checksum) == 2);
static_assert(sizeofmember(HWCALMicrophoneEchoCancellation_T, Padding) == 4);

typedef struct {
	u8 MaxInertia;
	u8 Pad;
	u16 PWM_CNT_EX;
	u32 Histogram1;
	u32 Histogram2;
	u32 Adjust[64];
} HWCALLcdPowerSaveExtraData_T;

typedef struct {
	HWCALLcdPowerSaveExtraData_T Data;
	u16 AgingFlag;
	u16 Checksum; // crc16 init 0x55AA
} HWCALLcdPowerSaveExtra_T;

static_assert(sizeof(HWCALLcdPowerSaveExtraData_T) == 0x10C);
static_assert(offsetof(HWCALLcdPowerSaveExtraData_T, MaxInertia) == 0);
static_assert(offsetof(HWCALLcdPowerSaveExtraData_T, Pad) == 1);
static_assert(offsetof(HWCALLcdPowerSaveExtraData_T, PWM_CNT_EX) == 2);
static_assert(offsetof(HWCALLcdPowerSaveExtraData_T, Histogram1) == 4);
static_assert(offsetof(HWCALLcdPowerSaveExtraData_T, Histogram2) == 8);
static_assert(offsetof(HWCALLcdPowerSaveExtraData_T, Adjust) == 12);
static_assert(sizeofmember(HWCALLcdPowerSaveExtraData_T, MaxInertia) == 1);
static_assert(sizeofmember(HWCALLcdPowerSaveExtraData_T, Pad) == 1);
static_assert(sizeofmember(HWCALLcdPowerSaveExtraData_T, PWM_CNT_EX) == 2);
static_assert(sizeofmember(HWCALLcdPowerSaveExtraData_T, Histogram1) == 4);
static_assert(sizeofmember(HWCALLcdPowerSaveExtraData_T, Histogram2) == 4);
static_assert(sizeofmember(HWCALLcdPowerSaveExtraData_T, Adjust) == 256);

static_assert(sizeof(HWCALLcdPowerSaveExtra_T) == 0x110);
static_assert(offsetof(HWCALLcdPowerSaveExtra_T, Data) == 0);
static_assert(offsetof(HWCALLcdPowerSaveExtra_T, AgingFlag) == 268);
static_assert(offsetof(HWCALLcdPowerSaveExtra_T, Checksum) == 270);
static_assert(sizeofmember(HWCALLcdPowerSaveExtra_T, AgingFlag) == 2);
static_assert(sizeofmember(HWCALLcdPowerSaveExtra_T, Checksum) == 2);

typedef struct {
	u8 ThinningCountX;
	u8 ThinningCountY;
	u8 Reserved[6];
} HWCALCStickData_T;

typedef struct {
	HWCALCStickData_T Data;
	u16 AgingFlag;
	u16 Checksum; // crc16 init 0x55AA
	u8 Padding[4];
} HWCALCStick_T;

static_assert(sizeof(HWCALCStickData_T) == 0x8);
static_assert(offsetof(HWCALCStickData_T, ThinningCountX) == 0);
static_assert(offsetof(HWCALCStickData_T, ThinningCountY) == 1);
static_assert(offsetof(HWCALCStickData_T, Reserved) == 2);
static_assert(sizeofmember(HWCALCStickData_T, ThinningCountX) == 1);
static_assert(sizeofmember(HWCALCStickData_T, ThinningCountY) == 1);
static_assert(sizeofmember(HWCALCStickData_T, Reserved) == 6);

static_assert(sizeof(HWCALCStick_T) == 0x10);
static_assert(offsetof(HWCALCStick_T, Data) == 0);
static_assert(offsetof(HWCALCStick_T, AgingFlag) == 8);
static_assert(offsetof(HWCALCStick_T, Checksum) == 10);
static_assert(offsetof(HWCALCStick_T, Padding) == 12);
static_assert(sizeofmember(HWCALCStick_T, AgingFlag) == 2);
static_assert(sizeofmember(HWCALCStick_T, Checksum) == 2);
static_assert(sizeofmember(HWCALCStick_T, Padding) == 4);

typedef struct {
	float DivisorAtZero;
	float TranslationX;
	float TranslationY;
	float RotationZ;
	float HorizontalAngle;
	float OptimalDistance;
} HWCALQtmData_T;

typedef struct {
	HWCALQtmData_T Data;
	u16 AgingFlag;
	u16 Checksum; // crc16 init 0x55AA
	u8 Padding[4];
} HWCALQtm_T;

static_assert(sizeof(HWCALQtmData_T) == 0x18);
static_assert(offsetof(HWCALQtmData_T, DivisorAtZero) == 0);
static_assert(offsetof(HWCALQtmData_T, TranslationX) == 4);
static_assert(offsetof(HWCALQtmData_T, TranslationY) == 8);
static_assert(offsetof(HWCALQtmData_T, RotationZ) == 12);
static_assert(offsetof(HWCALQtmData_T, HorizontalAngle) == 16);
static_assert(offsetof(HWCALQtmData_T, OptimalDistance) == 20);
static_assert(sizeofmember(HWCALQtmData_T, DivisorAtZero) == 4);
static_assert(sizeofmember(HWCALQtmData_T, TranslationX) == 4);
static_assert(sizeofmember(HWCALQtmData_T, TranslationY) == 4);
static_assert(sizeofmember(HWCALQtmData_T, RotationZ) == 4);
static_assert(sizeofmember(HWCALQtmData_T, HorizontalAngle) == 4);
static_assert(sizeofmember(HWCALQtmData_T, OptimalDistance) == 4);

static_assert(sizeof(HWCALQtm_T) == 0x20);
static_assert(offsetof(HWCALQtm_T, Data) == 0);
static_assert(offsetof(HWCALQtm_T, AgingFlag) == 24);
static_assert(offsetof(HWCALQtm_T, Checksum) == 26);
static_assert(offsetof(HWCALQtm_T, Padding) == 28);
static_assert(sizeofmember(HWCALQtm_T, AgingFlag) == 2);
static_assert(sizeofmember(HWCALQtm_T, Checksum) == 2);
static_assert(sizeofmember(HWCALQtm_T, Padding) == 4);

typedef struct {
	HWCALTouch_T                      Touch;
	HWCALCirclePadPart1_T             CirclePad1;
	HWCALScreenFlicker_T              ScreenFlicker;
	HWCALRtcCompensation_T            RtcCompensation;
	HWCALRtcCorrection_T              RtcCorrection;
	HWCALOuterCamarasPart1_T          OuterCams1;
	HWCALGyroscope_T                  Gyro;
	HWCALAccelerometer_T              Accelerometer;
	HWCALCodec_T                      Codec;
	HWCALPit_T                        Pit;
	HWCALSound3DFilter_T              Sound3DFilter;
	HWCALLcdPowerSave_T               LcdPowersave;
	HWCALLcdStereoscopic_T            LcdStereoscopic;
	HWCALBacklightPwm_T               BlPwn;
	HWCALCirclePadPart2_T             CirclePad2;
	HWCALOuterCamarasPart2_T          OuterCams2;
	HWCALLcdPowerSave_T               LcdPowersaveLgy;
	HWCALSliders_T                    Sliders;
	HWCALLcdModeDelay_T               LcdModeDelay;
	HWCALMicrophoneEchoCancellation_T MicEchoCancel;
	HWCALLcdPowerSaveExtra_T          LcdPowersaveExtra;
	HWCALCStick_T                     CStick;
	HWCALQtm_T                        Qtm;
	u8 Unused[0x120];
} HWCALBody_T;

static_assert(sizeof(HWCALBody_T) == 0x7D0);
static_assert(offsetof(HWCALBody_T, Touch) == 0);
static_assert(offsetof(HWCALBody_T, CirclePad1) == 20);
static_assert(offsetof(HWCALBody_T, ScreenFlicker) == 32);
static_assert(offsetof(HWCALBody_T, RtcCompensation) == 36);
static_assert(offsetof(HWCALBody_T, RtcCorrection) == 40);
static_assert(offsetof(HWCALBody_T, OuterCams1) == 48);
static_assert(offsetof(HWCALBody_T, Gyro) == 188);
static_assert(offsetof(HWCALBody_T, Accelerometer) == 208);
static_assert(offsetof(HWCALBody_T, Codec) == 224);
static_assert(offsetof(HWCALBody_T, Pit) == 536);
static_assert(offsetof(HWCALBody_T, Sound3DFilter) == 544);
static_assert(offsetof(HWCALBody_T, LcdPowersave) == 1088);
static_assert(offsetof(HWCALBody_T, LcdStereoscopic) == 1136);
static_assert(offsetof(HWCALBody_T, BlPwn) == 1184);
static_assert(offsetof(HWCALBody_T, CirclePad2) == 1248);
static_assert(offsetof(HWCALBody_T, OuterCams2) == 1280);
static_assert(offsetof(HWCALBody_T, LcdPowersaveLgy) == 1296);
static_assert(offsetof(HWCALBody_T, Slider) == 1344);
static_assert(offsetof(HWCALBody_T, LcdModeDelay) == 1360);
static_assert(offsetof(HWCALBody_T, MicEchoCancel) == 1376);
static_assert(offsetof(HWCALBody_T, LcdPowersaveExtra) == 1392);
static_assert(offsetof(HWCALBody_T, CStick) == 1664);
static_assert(offsetof(HWCALBody_T, Qtm) == 1680);
static_assert(offsetof(HWCALBody_T, Unused) == 1712);
static_assert(sizeofmember(HWCALBody_T, Unused) == 288);

typedef struct {
	u32 Magic;
	u32 Version;
	u32 DataSize;
	u8 ModelVersion; // ?
	u8 Revision; // or test level? cfg uses this as reference to know what if some hwcal parts should be used or not
	u16 AgingSuccessBitmask;
	union {
		u8 Sha256[32]; // dev or EEP
		u8 HmacSha256[32]; // retail
	};
} HWCALHeader_T;

static_assert(sizeof(HWCALHeader_T) == 0x30);
static_assert(offsetof(HWCALHeader_T, Magic) == 0);
static_assert(offsetof(HWCALHeader_T, Version) == 4);
static_assert(offsetof(HWCALHeader_T, DataSize) == 8);
static_assert(offsetof(HWCALHeader_T, ModelVersion) == 12);
static_assert(offsetof(HWCALHeader_T, Revision) == 13);
static_assert(offsetof(HWCALHeader_T, AgingSuccessBitmask) == 14);
static_assert(offsetof(HWCALHeader_T, Sha256) == 16);
static_assert(offsetof(HWCALHeader_T, HmacSha256) == 16);
static_assert(sizeofmember(HWCALHeader_T, Magic) == 4);
static_assert(sizeofmember(HWCALHeader_T, Version) == 4);
static_assert(sizeofmember(HWCALHeader_T, DataSize) == 4);
static_assert(sizeofmember(HWCALHeader_T, ModelVersion) == 1);
static_assert(sizeofmember(HWCALHeader_T, Revision) == 1);
static_assert(sizeofmember(HWCALHeader_T, AgingSuccessBitmask) == 2);
static_assert(sizeofmember(HWCALHeader_T, Sha256) == 32);
static_assert(sizeofmember(HWCALHeader_T, HmacSha256) == 32);

typedef struct {
	HWCALHeader_T Header;
	u8 Padding[0x1D0];
	HWCALBody_T Body;
} HWCAL_T;

static_assert(sizeof(HWCAL_T) == 0x9D0);
static_assert(offsetof(HWCAL_T, Header) == 0);
static_assert(offsetof(HWCAL_T, Padding) == 48);
static_assert(offsetof(HWCAL_T, Body) == 512);
static_assert(sizeofmember(HWCAL_T, Padding) == 464);

// Different kinds of types not directly inside HWCAL in format
// structures that cfg appears to do for cfg blks and IPCs with data from HWCAL

typedef struct {
	float ScaleX;
	float ScaleY;
	s16 CenterX;
	s16 CenterY;
	s16 MaxX;
	s16 MinX;
	s16 MaxY;
	s16 MinY;
	s16 Type;
	u16 Unknown[3];
} CirclePadParts_T;

static_assert(sizeof(CirclePadParts_T) == 0x1C);
static_assert(offsetof(CirclePadParts_T, ScaleX) == 0);
static_assert(offsetof(CirclePadParts_T, ScaleY) == 4);
static_assert(offsetof(CirclePadParts_T, CenterX) == 8);
static_assert(offsetof(CirclePadParts_T, CenterY) == 10);
static_assert(offsetof(CirclePadParts_T, MaxX) == 12);
static_assert(offsetof(CirclePadParts_T, MinX) == 14);
static_assert(offsetof(CirclePadParts_T, MaxY) == 16);
static_assert(offsetof(CirclePadParts_T, MinY) == 18);
static_assert(offsetof(CirclePadParts_T, Type) == 20);
static_assert(offsetof(CirclePadParts_T, Unknown) == 22);
static_assert(sizeofmember(CirclePadParts_T, ScaleX) == 4);
static_assert(sizeofmember(CirclePadParts_T, ScaleY) == 4);
static_assert(sizeofmember(CirclePadParts_T, CenterX) == 2);
static_assert(sizeofmember(CirclePadParts_T, CenterY) == 2);
static_assert(sizeofmember(CirclePadParts_T, MaxX) == 2);
static_assert(sizeofmember(CirclePadParts_T, MinX) == 2);
static_assert(sizeofmember(CirclePadParts_T, MaxY) == 2);
static_assert(sizeofmember(CirclePadParts_T, MinY) == 2);
static_assert(sizeofmember(CirclePadParts_T, Type) == 2);
static_assert(sizeofmember(CirclePadParts_T, Unknown) == 6);

typedef struct {
	HWCALCStickData_T Data;
	u8 Unknown[20];
} CStick_T;

static_assert(sizeof(CStick_T) == 0x1C);
static_assert(offsetof(CStick_T, Data) == 0);
static_assert(offsetof(CStick_T, Unknown) == 8);
static_assert(sizeofmember(CStick_T, Unknown) == 20);
