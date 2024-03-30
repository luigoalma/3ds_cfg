#include "hwcal_dummy_defaults.h"

const HWCALRtcCompensationData_T DummyRtcComp = (HWCALRtcCompensationData_T){
	.CompensationValue = 0x80
};

const HWCALScreenFlickerData_T DummyScreenFlicker = (HWCALScreenFlickerData_T){
	.FlickerTop = 0x5C,
	.FlickerBottom = 0x5F
};

const HWCALOuterCamarasPart1Data_T DummyOuterCams1 = (HWCALOuterCamarasPart1Data_T){
	.Flags = 0,
	.Scale = 1.0f, // 0x3F800000
	.RotationZ = 0.0f, // 0x0
	.TranslationX = -86.2324295f, // 0xC2AC7701
	.TranslationY = 0.0f, // 0x0
	.RotationX = 0.0f, // 0x0
	.RotationY = 0.0f, // 0x0
	.ViewAngleRight = 66.0f, // 0x42840000
	.ViewAngleLeft = 66.0f, // 0x42840000
	.ChartDistance = 250.0f, // 0x437A0000
	.CameraDistance = 35.0f, // 0x420C0000
	.ImageWidth = 640,
	.ImageHeight = 480,
	.Reserved = {0},
	.Unknown = {0},
	.AeBaseTarget = 0,
	.kRL = 0,
	.kGL = 0,
	.kBL = 0,
	.CcmPosition = 0
};

const HWCALTouchData_T DummyTouch = (HWCALTouchData_T){
	.RawX0 = 1638,
	.RawY0 = 1024,
	.PointX0 = 127,
	.PointY0 = 59,
	.RawX1 = 2457,
	.RawY1 = 3072,
	.PointX1 = 191,
	.PointY1 = 179
};

const HWCALCirclePadPart1Data_T DummyCirclePad1 = (HWCALCirclePadPart1Data_T){
	.CenterX = 0x800,
	.CenterY = 0x800,
	.Unk = {0}
};

const HWCALCodecData_T DummyCodec = (HWCALCodecData_T){
	.DriverGainHP = 0,
	.DriverGainSP = 1,
	.AnalogVolumeHP = 0,
	.AnalogVolumeSP = 7,
	.ShutterVolume0 = 253,
	.ShutterVolume1 = 236,
	.MicrophoneBias = 3,
	.QuickCharge = 2,
	.PGA_GAIN = 0,
	.Reserved1 = {},
	.FilterHP32 = {0x7FFF, 0, 0, 0, 0, 0x7FFF, 0, 0, 0, 0, 0x7FE0, 0xC010, 0, 0x3FE0, 0},
	.FilterHP47 = {0x7FFF, 0, 0, 0, 0, 0x7FFF, 0, 0, 0, 0, 0x7FE9, 0xC00C, 0, 0x3FE9, 0},
	.FilterSP32 = {0x7FFF, 0x9471, 0x578D, 0x7896, 0x8E58, 0xC950, 0x7530, 0xC950, 0, 0, 0x7FE0, 0xC010, 0, 0x3FE0, 0},
	.FilterSP47 = {0x7FFF, 0x8EBD, 0x62BD, 0x7AE0, 0x8A08, 0xC7BE, 0x7530, 0xC7BE, 0, 0, 0x7FE9, 0xC00C, 0, 0x3FE9, 0},
	.FilterMic32 = {0x7FFF, 0, 0, 0x7FFF, 0, 0, 0, 0, 0x7FFF, 0, 0, 0, 0, 0x7FFF, 0, 0, 0, 0, 0x7FFF, 0, 0, 0, 0, 0x7FFF, 0, 0, 0, 0},
	.FilterMic47 = {0x7FFF, 0, 0, 0x7FFF, 0, 0, 0, 0, 0x7FFF, 0, 0, 0, 0, 0x7FFF, 0, 0, 0, 0, 0x7FFF, 0, 0, 0, 0, 0x7FFF, 0, 0, 0, 0},
	.FilterFree = {0x7FFF, 0, 0, 0xCD61, 0xDDAF, 0x7FFF, 0x2251, 0x329F, 0xCD61, 0xDDAF, 0x7FFF, 0x2251, 0x329F, 0xCD61, 0xDDAF, 0x7FFF, 0x2251, 0x329F, 0x7FFF, 0, 0, 0, 0, 0x7FFF, 0, 0, 0, 0},
	.AnalogInterval = 1,
	.AnalogStabilize = 9,
	.AnalogPrecharge = 4,
	.AnalogSense = 3,
	.AnalogDebounce = 0,
	.Analog_XP_Pullup = 6,
	.YM_Driver = 1,
	.Reserved2 = 0
};

const HWCALAccelerometerData_T DummyAccelerometer = (HWCALAccelerometerData_T){
	.OffsetX = 0,
	.ScaleX = 0x400,
	.OffsetY = 0,
	.ScaleY = 0x400,
	.OffsetZ = 0,
	.ScaleZ = 0x400
};

const HWCALSound3DFilterData_T DummySound3DFilter = (HWCALSound3DFilterData_T){
	.SpecialFilter = {
		0xFFB4, 0xFFB6, 0xFF6E, 0xFEFE, 0xFEE2, 0xFF26, 0xFEF0, 0xFE59,
		0xFE37, 0xFE3E, 0xFE07, 0xFDF2, 0xFDBF, 0xFD24, 0xFD10, 0xFD76,
		0xFD19, 0xFCA1, 0xFCE6, 0xFCAF, 0xFC44, 0xFCCA, 0xFD08, 0xFC94,
		0xFC40, 0xFC7F, 0xFDC0, 0xFDDA, 0xFC53, 0xFD50, 0xFF6F, 0xFE8F,
		0xFDFF, 0xFFC5, 0x004B, 0x000F, 0x017D, 0x0251, 0x01FB, 0x0315,
		0x0428, 0x04AA, 0x0685, 0x068C, 0x054A, 0x076C, 0x0A2B, 0x0AF2,
		0x08E1, 0x06B1, 0x0C34, 0x102B, 0x0A0F, 0x07CB, 0x0C11, 0x0ED8,
		0x0CE2, 0x0699, 0x08F0, 0x0CEC, 0x0565, 0x05D7, 0x069A, 0xFD53,
		0x0330, 0x03D8, 0xF415, 0xF35C, 0xF225, 0xE85C, 0xDA68, 0xC544,
		0xDF7C, 0x187D, 0x1935, 0xFCED, 0xF193, 0xF45D, 0x0967, 0x0DA1,
		0xF879, 0xF280, 0xF870, 0x00D7, 0x0BE1, 0x0AAD, 0x0542, 0x0483,
		0xFFCC, 0xFC5E, 0xFE20, 0xFF09, 0xFE55, 0xFD31, 0xFEBC, 0x0261,
		0x02BC, 0x0177, 0x00E0, 0xFFB8, 0xFEC3, 0xFE0A, 0xFED3, 0x02C0,
		0x05E3, 0x0491, 0x0133, 0x0014, 0x01CF, 0x0304, 0x019A, 0xFF59,
		0xFEA7, 0xFF67, 0x0025, 0x005E, 0x002E, 0xFFBF, 0xFF2E, 0xFE4D,
		0xFDA8, 0xFE54, 0xFF9D, 0xFFC5, 0xFE2E, 0xFE40, 0xFE2F, 0xFE22,
		0xFE39, 0xFE89, 0xFEB4, 0xFE7E, 0xFE9D, 0xFF31, 0xFF57, 0xFF40,
		0xFFA2, 0xFFF1, 0xFFF6, 0x0077, 0x00FE, 0x00D1, 0x00F1, 0x01A8,
		0x01DE, 0x01F1, 0x0256, 0x0277, 0x029C, 0x02D5, 0x031B, 0x0399,
		0x0311, 0x0297, 0x03FA, 0x0402, 0x0240, 0x02DD, 0x03F2, 0x0287,
		0x01F0, 0x02DC, 0x01DA, 0x0094, 0x016B, 0x012E, 0xFF80, 0xFF00,
		0xFEA1, 0xFE69, 0xFE35, 0xFC97, 0xFBE7, 0xFB47, 0xFA68, 0xFC34,
		0xFA95, 0xF653, 0xF88A, 0xFB5A, 0xF955, 0xF5FC, 0xF5EE, 0xFBF3,
		0xFB0C, 0xF4DA, 0xFB1C, 0xFE28, 0xFA18, 0xFF33, 0xFEB7, 0xFE73,
		0x0853, 0x075B, 0x0A21, 0x166E, 0x1C4B, 0x27D1, 0x1CAA, 0xF453,
		0xEE0C, 0x00E6, 0x0662, 0x05FE, 0xFBC0, 0xF661, 0x02C5, 0x06F5,
		0x01B9, 0xFD44, 0xF71E, 0xF5D7, 0xF835, 0xF9FA, 0xFED7, 0x032F,
		0x0231, 0xFF25, 0xFF07, 0x004F, 0xFDEE, 0xFACF, 0xFB3D, 0xFC60,
		0xFCBC, 0xFD7D, 0xFED4, 0x0074, 0x0049, 0xFDBC, 0xFB43, 0xFB0F,
		0xFD95, 0x0067, 0x00F5, 0xFF9F, 0xFEAE, 0x009D, 0x0312, 0x0212,
		0x0048, 0x003A, 0x0051, 0x0102, 0x021A, 0x022E, 0x0284, 0x02F2,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
	},
	.IIRSurroundFilter = {0x3A593C54, 0xFC99327F, 0x2706212A, 0x0366CD81, 0xDEA0A283}
};

// Why? Used on formatting save
const HWCALSound3DFilterData_T DefaultSound3DFilter = (HWCALSound3DFilterData_T){
	.SpecialFilter = {
		0xFFB4, 0xFFB6, 0xFF6E, 0xFEFE, 0xFEE2, 0xFF26, 0xFEF0, 0xFE59,
		0xFE37, 0xFE3E, 0xFE07, 0xFDF2, 0xFDBF, 0xFD24, 0xFD10, 0xFD76,
		0xFD19, 0xFCA1, 0xFCE6, 0xFCAF, 0xFC44, 0xFCCA, 0xFD08, 0xFC94,
		0xFC40, 0xFC7F, 0xFDC0, 0xFDDA, 0xFC53, 0xFD50, 0xFF6F, 0xFE8F,
		0xFDFF, 0xFFC5, 0x004B, 0x000F, 0x017D, 0x0251, 0x01FB, 0x0315,
		0x0428, 0x04AA, 0x0685, 0x068C, 0x054A, 0x076C, 0x0A2B, 0x0AF2,
		0x08E1, 0x06B1, 0x0C34, 0x102B, 0x0A0F, 0x07CB, 0x0C11, 0x0ED8,
		0x0CE2, 0x0699, 0x08F0, 0x0CEC, 0x0565, 0x05D7, 0x069A, 0xFD53,
		0x0330, 0x03D8, 0xF415, 0xF35C, 0xF225, 0xE85C, 0xDA68, 0xC544,
		0xDF7C, 0x187D, 0x1935, 0xFCED, 0xF193, 0xF45D, 0x0967, 0x0DA1,
		0xF879, 0xF280, 0xF870, 0x00D7, 0x0BE1, 0x0AAD, 0x0542, 0x0483,
		0xFFCC, 0xFC5E, 0xFE20, 0xFF09, 0xFE55, 0xFD31, 0xFEBC, 0x0261,
		0x02BC, 0x0177, 0x00E0, 0xFFB8, 0xFEC3, 0xFE0A, 0xFED3, 0x02C0,
		0x05E3, 0x0491, 0x0133, 0x0014, 0x01CF, 0x0304, 0x019A, 0xFF59,
		0xFEA7, 0xFF67, 0x0025, 0x005E, 0x002E, 0xFFBF, 0xFF2E, 0xFE4D,
		0xFDA8, 0xFE54, 0xFF9D, 0xFFC5, 0xFE2E, 0xFE40, 0xFE2F, 0xFE22,
		0xFE39, 0xFE89, 0xFEB4, 0xFE7E, 0xFE9D, 0xFF31, 0xFF57, 0xFF40,
		0xFFA2, 0xFFF1, 0xFFF6, 0x0077, 0x00FE, 0x00D1, 0x00F1, 0x01A8,
		0x01DE, 0x01F1, 0x0256, 0x0277, 0x029C, 0x02D5, 0x031B, 0x0399,
		0x0311, 0x0297, 0x03FA, 0x0402, 0x0240, 0x02DD, 0x03F2, 0x0287,
		0x01F0, 0x02DC, 0x01DA, 0x0094, 0x016B, 0x012E, 0xFF80, 0xFF00,
		0xFEA1, 0xFE69, 0xFE35, 0xFC97, 0xFBE7, 0xFB47, 0xFA68, 0xFC34,
		0xFA95, 0xF653, 0xF88A, 0xFB5A, 0xF955, 0xF5FC, 0xF5EE, 0xFBF3,
		0xFB0C, 0xF4DA, 0xFB1C, 0xFE28, 0xFA18, 0xFF33, 0xFEB7, 0xFE73,
		0x0853, 0x075B, 0x0A21, 0x166E, 0x1C4B, 0x27D1, 0x1CAA, 0xF453,
		0xEE0C, 0x00E6, 0x0662, 0x05FE, 0xFBC0, 0xF661, 0x02C5, 0x06F5,
		0x01B9, 0xFD44, 0xF71E, 0xF5D7, 0xF835, 0xF9FA, 0xFED7, 0x032F,
		0x0231, 0xFF25, 0xFF07, 0x004F, 0xFDEE, 0xFACF, 0xFB3D, 0xFC60,
		0xFCBC, 0xFD7D, 0xFED4, 0x0074, 0x0049, 0xFDBC, 0xFB43, 0xFB0F,
		0xFD95, 0x0067, 0x00F5, 0xFF9F, 0xFEAE, 0x009D, 0x0312, 0x0212,
		0x0048, 0x003A, 0x0051, 0x0102, 0x021A, 0x022E, 0x0284, 0x02F2,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
	},
	.IIRSurroundFilter = {0x41FFA566, 0xA1372FF7, 0x252D82E1, 0x5FA5BD54, 0xD9AFC505}
};

const HWCALLcdPowerSaveData_T DummyLcdPowerSave = (HWCALLcdPowerSaveData_T){
	.DitherPattern = 0xC66C9339,
	.StartX = 0,
	.StartY = 0,
	.SizeX = 0x400,
	.SizeY = 0x400,
	.GTHRatio = 0x80,
	.DitherMode = 0,
	.MinRS = 0x72,
	.MaxRS = 0xFF,
	.MinGTH = 0x69,
	.MinMax = 0xFF,
	.ExMax = 0x20,
	.Inertia = 2,
	.LutListRS = {0x14, 0x2C, 0x46, 0x5C, 0x72, 0x82, 0x8E, 0x98, 0xA0},
	.Reserved = {0, 0}
};

const HWCALLcdStereoscopicData_T DummyLcdStereoscopic = (HWCALLcdStereoscopicData_T){
	.PupillaryDistanceInMm = 62.0f, // 0x42780000
	.DistanceEyesAndUpperScreenInMm = 289.0f, // 0x43908000
	.ScreenWidthInMm = 76.8000031f, // 0x4299999A
	.ScreenHeightInMm = 46.0800018f, // 0x423851EC
	.Unknown = {10.0f, 5.0f, 55.5800018f, 21.5699997f} // 0x41200000, 0x40A00000, 0x425E51EC, 0x41AC8F5C
};

const HWCALBacklightPwmData_T DummyBacklight = (HWCALBacklightPwmData_T){
	.Coefficients = {
		{0.001116389990f, 1.414119960f, 0.0717880875f}, // 0x3A9253D5, 0x3FB501E2, 0x3D9305A2
		{0.000418168987f, 0.665669978f, 0.0609865375f}, // 0x39DB3DB1, 0x3F2A6959, 0x3D79CD05
		{0.002085430080f, 1.556390050f, 0.0385938995f}  // 0x3B08ABB6, 0x3FC737CA, 0x3D1E14A3
	},
	.NumLevels = 5,
	.Unknown = 0,
	.Brightnesses = {0x14, 0x2B, 0x49, 0x5F, 0x75, 0xAC, 0xAC},
	.BaseDivisor = 0x200,
	.MinimumBrightnessHw = 0xD
};

const HWCALCirclePadPart2Data_T DummyCirclePad2 = (HWCALCirclePadPart2Data_T){
	.ScaleX = 1.0f, // 0x3F800000
	.ScaleY = 1.0f, // 0x3F800000
	.MaxX = 0xD99,
	.MinX = 0x267,
	.MaxY = 0xD99,
	.MinY = 0x267,
	.Type = 0,
	.Unknown = {0}
};

const HWCALSlidersData_T DummySliders = (HWCALSlidersData_T){
	.SVR2Min = 0x24,
	.SVR2Max = 0xDB,
	.VolumeSliderMin = 0x24,
	.VolumeSliderMax = 0xDB
};

const HWCALLcdModeDelayData_T DummyLcdModeDelay = (HWCALLcdModeDelayData_T){
	.To2D = 0xD,
	.To3D = 0
};

const HWCALMicrophoneEchoCancellationData_T DummyMicEchoCancel = (HWCALMicrophoneEchoCancellationData_T){
	.Unknown = {0x0C, 0x06, 0x0C, 0x01, 0x0C, 0xA3, 0x00, 0x08}
};

const HWCALPitData_T DummyPit = (HWCALPitData_T){
	.VisibleFactor = 0x4C2B,
	.IRFactor = 0x4C2B
};

// Why? Used on formatting or upgrading save
const HWCALPitData_T DefaultPit = (HWCALPitData_T){
	.VisibleFactor = 0x4000,
	.IRFactor = 0x4000
};

const HWCALQtmData_T DummyQtm = (HWCALQtmData_T){
	.DivisorAtZero = 6.5f, // 0x40D00000
	.TranslationX = 0.0f, // 0x0
	.TranslationY = 0.0f, // 0x0
	.RotationZ = 0.0f, // 0x0
	.HorizontalAngle = 66.4000015f, // 0x4284CCCD
	.OptimalDistance = 301.0f, // 0x43968000
};

// Why? Used on formatting save or load defaults
const HWCALQtmData_T DefaultQtm = (HWCALQtmData_T){
	.DivisorAtZero = 4.5f, // 0x40900000
	.TranslationX = 0.0f, // 0x0
	.TranslationY = 0.0f, // 0x0
	.RotationZ = 0.0f, // 0x0
	.HorizontalAngle = 70.5999985f, // 0x428D3333
	.OptimalDistance = 301.0f, // 0x43968000
};

// defaults for combined or alternate structs

const CirclePadParts_T DefaultCirclePad = (CirclePadParts_T){
	.ScaleX = 1.0f, // 0x3F800000
	.ScaleY = 1.0f, // 0x3F800000
	.CenterX = 0x800,
	.CenterY = 0x800,
	.MaxX = 0xD99,
	.MinX = 0x267,
	.MaxY = 0xD99,
	.MinY = 0x267,
	.Type = 0,
	.Unknown = {0}
};
