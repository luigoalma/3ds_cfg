#include <string.h>
#include <3ds/types.h>
#include <storage/config.h>
#include <storage/secinfo.h>
#include <utils/sha256.h>
#include <utils/endian.h>
#include <cfg.h>

// dunno what this block is yet
Result Cfg_SetBlk0x160000Byte0(u8 byte) {
	u8 data[4];
	Result res;

	res = Cfg_System_ReadBlk(&data[0], 0x160000, 4);
	if(R_FAILED(res))
		return res;

	data[0] = byte;
	res = Cfg_System_WriteBlk(&data[0], 0x160000, 4);
	if(R_FAILED(res))
		return res;

	Cfg_SaveConfig();
	res = 0;

	return res;
}

void Cfg_TranslateCountryInfo(u32* info, u8 left_to_right) {
	const u32 TranslationTable[5][2] = {
		{0x6E030000, 0x6E040000},
		{0x6E040000, 0x6E050000},
		{0x6E050000, 0x6E060000},
		{0x6E060000, 0x6E070000},
		{0x6E070000, 0x6E030000}
	};

	int check_index = left_to_right ? 0 : 1;
	int get_index = left_to_right ? 1 : 0;
	u32 _info = *info;

	for(int i = 0; i < 5; ++i) {
		if(_info == TranslationTable[i][check_index]) {
			*info = TranslationTable[i][get_index];
			break;
		}
	}
}

Result Cfg_TransferableId(u64* id, u32 unique_id) {
	u32 msg[3];

	Result res = Cfg_System_ReadBlk(&msg[0], 0x90001, 8);
	if(R_FAILED(res)) {
		*id = 0;
		return res;
	}

	msg[2] = unique_id & 0xFFFFF;

	sha256_t ctx;
	sha256_init(&ctx);
	sha256_update(&ctx, &msg[0], 12);
	sha256_finish(&ctx);

	*id = getbe32(ctx.H[6]) | ((u64)getbe32(ctx.H[7]) << 32);
	return 0;
}

Result Cfg_IsCoppacsSupported(bool* support) {
	u8 data[4];
	u8 region;
	Result res;

	res = Cfg_System_ReadBlk(&data[0], 0xB0000, 4);
	if(R_SUCCEEDED(res)) res = SecInfo_GetRegionByte(&region);
	if(R_SUCCEEDED(res)) {
		*support = (region == CFG_REGION_USA) && ((data[3] == CFG_COUNTRY_US) || (data[3] == CFG_COUNTRY_CA));
		res = 0;
	} else *support = false;

	return res;
}

Result Cfg_GetSystemModel(CFG_SystemModel* model) {
	u8 data[4];
	Result res;
	char serial[15];

	res = Cfg_System_ReadBlk(&data[0], 0xF0004, 4);
	if(R_FAILED(res)) {
		*model = CFG_MODEL_3DS;
		return res;
	}
	
	CFG_SystemModel _model = (CFG_SystemModel)data[0];

	if(_model == CFG_MODEL_2DS) {
		if(R_SUCCEEDED(SecInfo_GetSerialNumber(serial, sizeof(serial)))) {
			if(serial[0] != 'A' && serial[0] != 'P') {
				*model = CFG_MODEL_3DS;
				return CFG_BAD_MODEL_CHECK;
			}
		}
	}

	*model = _model;
	return 0;
}

Result Cfg_ResetParentalControls() {
	void* ptr;
	Result res;

	res = Cfg_System_GetBlkPtr(&ptr, 0xC0000, 0xC0);
	if(R_FAILED(res))
		return res;

	memset(ptr, 0, 0xC0);
	((u8*)ptr)[9] = 0x14;

	res = Cfg_System_GetBlkPtr(&ptr, 0xC0002, 0x200);
	if(R_FAILED(res))
		return res;

	memset(ptr, 0, 0x200);
	Cfg_SaveConfig();

	return 0;
}
