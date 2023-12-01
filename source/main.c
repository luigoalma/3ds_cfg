#include <3ds/ipc.h>
#include <3ds/os.h>
#include <3ds/result.h>
#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/fs.h>
#include <ps.h>
#include <err.h>
#include <memops.h>
#include <utils/sha256.h>
#include <utils/endian.h>
#include <storage/config.h>
#include <storage/hwcal.h>
#include <storage/lfcs.h>
#include <storage/secinfo.h>

#define OS_REMOTE_SESSION_CLOSED MAKERESULT(RL_STATUS,    RS_CANCELED, RM_OS, 26)
#define OS_INVALID_HEADER        MAKERESULT(RL_PERMANENT, RS_WRONGARG, RM_OS, 47)
#define OS_INVALID_IPC_PARAMATER MAKERESULT(RL_PERMANENT, RS_WRONGARG, RM_OS, 48)

static __attribute__((section(".data.TerminationFlag"))) bool TerminationFlag = false;

static const char* const CFG_ServiceNames[] = {"cfg:u", "cfg:s", "cfg:i", "cfg:nor"};

#define CFG_U_SERVICE_INDEX 0
#define CFG_S_SERVICE_INDEX 1
#define CFG_I_SERVICE_INDEX 2
#define CFG_NOR_SERVICE_INDEX 3

inline static void HandleSRVNotification() {
	u32 id;
	Err_FailedThrow(srvReceiveNotification(&id));
	if (id == 0x100)
		TerminationFlag = true;
}

// dunno what this block is yet
static Result Cfg_SetBlk0x160000Byte0(u8 byte) {
	u8 data[4];
	Result res;

	res = Cfg_System_ReadBlk(&data[0], 0x160000, 4);
	if(R_SUCCEEDED(res)) {
		data[0] = byte;
		res = Cfg_System_WriteBlk(&data[0], 0x160000, 4);
		if(R_SUCCEEDED(res)) {
			Cfg_SaveConfig();
			res = 0;
		}
	}

	return res;
}

static void Cfg_TranslateCountryInfo(u32* info, u8 left_to_right) {
	static const u32 TranslationTable[5][2] = {
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

static Result Cfg_TransferableId(u64* id, u32 unique_id) {
	u32 msg[3];

	res = Cfg_System_GetBlkPtr(&msg[0], 0x90001, 8);
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

static Result Cfg_IsCoppacsSupported(bool* support) {
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

static Result Get_GetSystemModel(CFG_SystemModel* model) {
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
			if(serial[0] != 'A' && serial != 'P') {
				*model = CFG_MODEL_3DS;
				return CFG_BAD_MODEL_CHECK;
			}
		}
	}

	*model = _model;
	return 0;
}

static Result Cfg_ResetParentalControls() {
	void* ptr;
	Result res;

	res = Cfg_System_GetBlkPtr(&ptr, 0xC0000, 0xC0);
	if(R_SUCCEEDED(res)) {
		memset(ptr, 0, 0xC0);
		((u8*)ptr)[9] = 0x14;
		res = Cfg_System_GetBlkPtr(&ptr, 0xC0002, 0x200);
		if(R_SUCCEEDED(res)) {
			memset(ptr, 0, 0x200);
			Cfg_SaveConfig();
			res = 0;
		}
	}

	return res;
}

static void CFG_Common_IPCSession(int service_index) {
	u32* cmdbuf = getThreadCommandBuffer();

	u16 cmdid = cmdbuf[0] >> 16;

	if ((service_index == CFG_U_SERVICE_INDEX && cmdid > 0xB) ||
		(service_index == CFG_S_SERVICE_INDEX && cmdid > 0x40F)) {
		cmdbuf[0] = IPC_MakeHeader(0x0, 1, 0);
		cmdbuf[1] = OS_INVALID_HEADER;
		return;
	}

	switch (cmdid) {
	case 0x1:
		if (!IPC_CompareHeader(cmdbuf[0], 0x1, 2, 2) || !IPC_Is_Desc_Buffer(cmdbuf[3], IPC_BUFFER_W)) {
			cmdbuf[0] = IPC_MakeHeader(0x0, 1, 0);
			cmdbuf[1] = OS_INVALID_IPC_PARAMATER;
		} else {
			void* ptr = (void*)cmdbuf[4];
			size_t size = IPC_Get_Desc_Buffer_Size(cmdbuf[3]);

			cmdbuf[1] = Cfg_User_ReadBlk(ptr, cmdbuf[2], size);
			cmdbuf[0] = IPC_MakeHeader(0x1, 1, 2);
			cmdbuf[2] = IPC_Desc_Buffer(size, IPC_BUFFER_W);
			cmdbuf[3] = (u32)ptr;
		}
		break;
	case 0x2: // cfg:u
	case 0x406: // cfg:s
	case 0x816: // cfg:i
		{
			u8 region = 0;
			cmdbuf[1] = SecInfo_GetRegionByte(&region);
			cmdbuf[0] = IPC_MakeHeader(cmdid, 2, 0);
			cmdbuf[2] = region;
		}
		break;
	case 0x3:
		{
			u64 id;
			cmdbuf[1] = Cfg_TransferableId(&id, cmdbuf[1]);
			cmdbuf[0] = IPC_MakeHeader(0x3, 3, 0);
			cmdbuf[2] = id & 0xFFFFFFFF;
			cmdbuf[3] = id >> 32;
		}
		break;
	case 0x4:
		{
			bool support;
			cmdbuf[1] = Cfg_IsCoppacsSupported(&support);
			cmdbuf[0] = IPC_MakeHeader(0x4, 2, 0);
			cmdbuf[2] = support;
		}
		break;
	case 0x5:
		{
			CFG_SystemModel model;
			cmdbuf[1] = Get_GetSystemModel(&model);
			cmdbuf[0] = IPC_MakeHeader(0x5, 2, 0);
			cmdbuf[2] = model;
		}
		break;
	case 0x6:
		{
			CFG_SystemModel model;
			Result res = Get_GetSystemModel(&model);
			cmdbuf[0] = IPC_MakeHeader(0x6, 2, 0);
			cmdbuf[1] = 0;
			cmdbuf[2] = R_FAILED(res) || model != CFG_MODEL_2DS;
		}
		break;
	case 0x7:
		cmdbuf[1] = Cfg_SetBlk0x160000Byte0((u8)cmdbuf[1]);
		cmdbuf[0] = IPC_MakeHeader(0x7, 1, 0);
		break;
	case 0x8:
		{
			u32 info = cmdbuf[1];
			Cfg_TranslateCountryInfo(&info, (u8)cmdbuf[2]);
			cmdbuf[0] = IPC_MakeHeader(0x8, 2, 0);
			cmdbuf[1] = 0;
			cmdbuf[2] = info;
		}
		break;
	case 0x9:
		cmdbuf[0] = IPC_MakeHeader(0x9, 2, 0);
		cmdbuf[1] = CountryEnumToStr((CFG_CountryCode)cmdbuf[1], (char*)&cmdbuf[2]);
		break;
	case 0xA:
		cmdbuf[0] = IPC_MakeHeader(0xA, 2, 0);
		cmdbuf[1] = CountryStrToEnum((const char*)&cmdbuf[1], (CFG_CountryCode*)&cmdbuf[2]);
		break;
	case 0xB:
		// IsFangateSupported
		cmdbuf[0] = IPC_MakeHeader(0xB, 2, 0);
		cmdbuf[1] = 0;
		cmdbuf[2] = 1;
		break;
	case 0x401: // cfg:s
	case 0x801: // cfg:i
		if (!IPC_CompareHeader(cmdbuf[0], cmdid, 2, 2) || !IPC_Is_Desc_Buffer(cmdbuf[3], IPC_BUFFER_W)) {
			cmdbuf[0] = IPC_MakeHeader(0x0, 1, 0);
			cmdbuf[1] = OS_INVALID_IPC_PARAMATER;
		} else {
			void* ptr = (void*)cmdbuf[4];
			size_t size = IPC_Get_Desc_Buffer_Size(cmdbuf[3]);

			cmdbuf[1] = Cfg_System_ReadBlk(ptr, cmdbuf[2], size);
			cmdbuf[0] = IPC_MakeHeader(cmdid, 1, 2);
			cmdbuf[2] = IPC_Desc_Buffer(size, IPC_BUFFER_W);
			cmdbuf[3] = (u32)ptr;
		}
		break;
	case 0x402: // cfg:s
	case 0x802: // cfg:i
		if (!IPC_CompareHeader(cmdbuf[0], cmdid, 2, 2) || !IPC_Is_Desc_Buffer(cmdbuf[3], IPC_BUFFER_R)) {
			cmdbuf[0] = IPC_MakeHeader(0x0, 1, 0);
			cmdbuf[1] = OS_INVALID_IPC_PARAMATER;
		} else {
			void* ptr = (void*)cmdbuf[4];
			size_t size = IPC_Get_Desc_Buffer_Size(cmdbuf[3]);

			cmdbuf[1] = Cfg_System_WriteBlkWithPublish(ptr, cmdbuf[1], size);
			cmdbuf[0] = IPC_MakeHeader(cmdid, 1, 2);
			cmdbuf[2] = IPC_Desc_Buffer(size, IPC_BUFFER_W);
			cmdbuf[3] = (u32)ptr;
		}
		break;
	case 0x403: // cfg:s
	case 0x803: // cfg:i
		Cfg_SaveConfig();
		cmdbuf[0] = IPC_MakeHeader(cmdid, 1, 0);
		cmdbuf[1] = 0;
		break;
	case 0x404: // cfg:s
	case 0x80F: // cfg:i
		if (!IPC_CompareHeader(cmdbuf[0], cmdid, 1, 2) || !IPC_Is_Desc_Buffer(cmdbuf[2], IPC_BUFFER_W)) {
			cmdbuf[0] = IPC_MakeHeader(0x0, 1, 0);
			cmdbuf[1] = OS_INVALID_IPC_PARAMATER;
		} else {
			void* ptr = (void*)cmdbuf[3];
			size_t size = IPC_Get_Desc_Buffer_Size(cmdbuf[2]);

			cmdbuf[1] = Lfcs_GetWholeData(ptr, size);
			cmdbuf[0] = IPC_MakeHeader(cmdid, 1, 2);
			cmdbuf[2] = IPC_Desc_Buffer(size, IPC_BUFFER_W);
			cmdbuf[3] = (u32)ptr;
		}
		break;
	case 0x405: // cfg:s
	case 0x810: // cfg:i
		{
			u64 id;
			cmdbuf[1] = Lfcs_GetId(&id);
			cmdbuf[0] = IPC_MakeHeader(cmdid, 3, 0);
			cmdbuf[2] = id & 0xFFFFFFFF;
			cmdbuf[3] = id >> 32;
		}
		break;
	// case 0x406 at case 0x2
	case 0x407: // cfg:s
	case 0x817: // cfg:i
		{
			u8 unk = 0;
			cmdbuf[1] = SecInfo_GetByte0x101(&unk);
			cmdbuf[0] = IPC_MakeHeader(cmdid, 2, 0);
			cmdbuf[2] = region;
		}
		break;
	case 0x408: // cfg:s
	case 0x818: // cfg:i
		if (!IPC_CompareHeader(cmdbuf[0], cmdid, 1, 2) || !IPC_Is_Desc_Buffer(cmdbuf[2], IPC_BUFFER_W)) {
			cmdbuf[0] = IPC_MakeHeader(0x0, 1, 0);
			cmdbuf[1] = OS_INVALID_IPC_PARAMATER;
		} else {
			void* ptr = (void*)cmdbuf[3];
			size_t size = IPC_Get_Desc_Buffer_Size(cmdbuf[2]);

			cmdbuf[1] = SecInfo_GetSerialNumber(ptr, size);
			cmdbuf[0] = IPC_MakeHeader(cmdid, 1, 2);
			cmdbuf[2] = IPC_Desc_Buffer(size, IPC_BUFFER_W);
			cmdbuf[3] = (u32)ptr;
		}
		break;
	case 0x409:
		{

		}
		break;
	case 0x40A: // cfg:s
	case 0x819: // cfg:i
		Cfg_SaveToFixData();
		cmdbuf[0] = IPC_MakeHeader(cmdid, 1, 0);
		cmdbuf[1] = 0;
		break;
	case 0x40B: // cfg:s
	case 0x81C: // cfg:i
		Cfg_DeleteFixData();
		cmdbuf[0] = IPC_MakeHeader(cmdid, 1, 0);
		cmdbuf[1] = 0;
		break;
	case 0x40C:
		{

		}
		break;
	case 0x40D:
		{
			u32 data = cmdbuf[1] & 0xFFFF;
			cmdbuf[1] = Cfg_System_WriteBlk(&data, 0x150000, 4);
			cmdbuf[0] = IPC_MakeHeader(0x40D, 1, 0);
		}
		break;
	case 0x40E:
		{
			u32 data;
			cmdbuf[1] = Cfg_System_ReadBlk(&data, 0x150000, 4);
			cmdbuf[0] = IPC_MakeHeader(0x40E, 2, 0);
			cmdbuf[2] = data & 0xFFFF;
		}
		break;
	case 0x40F:
		cmdbuf[1] = Cfg_ResetParentalControls();
		cmdbuf[0] = IPC_MakeHeader(0x40F, 1, 0);
		break;
	// case 0x801 at case 0x401
	// case 0x802 at case 0x402
	// case 0x803 at case 0x403
	case 0x804:
		if (!IPC_CompareHeader(cmdbuf[0], 0x804, 3, 2) || !IPC_Is_Desc_Buffer(cmdbuf[4], IPC_BUFFER_R)) {
			cmdbuf[0] = IPC_MakeHeader(0x0, 1, 0);
			cmdbuf[1] = OS_INVALID_IPC_PARAMATER;
		} else {
			const void* ptr = (const void*)cmdbuf[5];
			size_t size = IPC_Get_Desc_Buffer_Size(cmdbuf[4]);
			u32 blk_id = cmdbuf[1];
			BLK_FLAGS flags = (BLK_FLAGS)(cmdbuf[3] & 0xFFFF);

			cmdbuf[1] = Cfg_CreateBlkWithData(ptr, blk_id, size, flags);
			cmdbuf[0] = IPC_MakeHeader(0x804, 1, 2);
			cmdbuf[2] = IPC_Desc_Buffer(size, IPC_BUFFER_R);
			cmdbuf[3] = (u32)ptr;
		}
		break;
	case 0x805:
		Cfg_DeleteAndResetConfig();
		cmdbuf[0] = IPC_MakeHeader(0x805, 1, 0);
		cmdbuf[1] = 0;
		break;
	case 0x806:
		{

		}
		break;
	case 0x807:
		// originally resulted in wasted cycles to reach a no-op
		// so did nothing at all
		cmdbuf[0] = IPC_MakeHeader(0x807, 1, 0);
		cmdbuf[1] = 0;
		break;
	case 0x808:
		{

		}
		break;
	case 0x809:
		{

		}
		break;
	case 0x80A:
		cmdbuf[1] = Hwcal_ResetCirclePadCfgBlk();
		cmdbuf[0] = IPC_MakeHeader(0x80A, 1, 0);
		break;
	case 0x80B:
		if (!IPC_CompareHeader(cmdbuf[0], 0x80B, 2, 2) || !IPC_Is_Desc_Buffer(cmdbuf[3], IPC_BUFFER_R)) {
			cmdbuf[0] = IPC_MakeHeader(0x0, 1, 0);
			cmdbuf[1] = OS_INVALID_IPC_PARAMATER;
		} else {
			void* ptr = (void*)cmdbuf[4];
			size_t size = IPC_Get_Desc_Buffer_Size(cmdbuf[3]);
			bool reset_hardcoded = (cmdbuf[2] & 0xFF) ? true : false;

			cmdbuf[1] = Lfcs_SetData(ptr, size, reset_hardcoded);
			cmdbuf[0] = IPC_MakeHeader(0x80B, 1, 2);
			cmdbuf[2] = IPC_Desc_Buffer(size, IPC_BUFFER_R);
			cmdbuf[3] = (u32)ptr;
		}
		break;
	case 0x80C:
		if (!IPC_CompareHeader(cmdbuf[0], 0x80C, 1, 2) || !IPC_Is_Desc_Buffer(cmdbuf[2], IPC_BUFFER_R)) {
			cmdbuf[0] = IPC_MakeHeader(0x0, 1, 0);
			cmdbuf[1] = OS_INVALID_IPC_PARAMATER;
		} else {
			void* ptr = (void*)cmdbuf[3];
			size_t size = IPC_Get_Desc_Buffer_Size(cmdbuf[2]);

			cmdbuf[1] = Lfcs_SetSignature(ptr, size);
			cmdbuf[0] = IPC_MakeHeader(0x80C, 1, 2);
			cmdbuf[2] = IPC_Desc_Buffer(size, IPC_BUFFER_R);
			cmdbuf[3] = (u32)ptr;
		}
		break;
	case 0x80D:
		Lfcs_SaveToNextSlot();
		cmdbuf[0] = IPC_MakeHeader(0x80D, 1, 0);
		cmdbuf[1] = 0;
		break;
	case 0x80E:
		cmdbuf[1] = Lfcs_CheckSignature()
		cmdbuf[0] = IPC_MakeHeader(0x80E, 1, 0);
		break;
	// case 0x80F at case 0x404
	// case 0x810 at case 0x405
	case 0x811:
		if (!IPC_CompareHeader(cmdbuf[0], 0x811, 2, 4) || !IPC_Is_Desc_Buffer(cmdbuf[3], IPC_BUFFER_R) || !IPC_Is_Desc_Buffer(cmdbuf[5], IPC_BUFFER_R)) {
			cmdbuf[0] = IPC_MakeHeader(0x0, 1, 0);
			cmdbuf[1] = OS_INVALID_IPC_PARAMATER;
		} else {
			const void* sig = (const void*)cmdbuf[6];
			size_t sigsize = IPC_Get_Desc_Buffer_Size(cmdbuf[5]);

			const void* data = (const void*)cmdbuf[4];
			size_t datasize = IPC_Get_Desc_Buffer_Size(cmdbuf[3]);

			cmdbuf[1] = SecInfo_SetWholeSecInfo(sig, sigsize, data, datasize);
			cmdbuf[0] = IPC_MakeHeader(0x811, 1, 4);
			cmdbuf[2] = IPC_Desc_Buffer(datasize, IPC_BUFFER_R);
			cmdbuf[3] = (u32)data;
			cmdbuf[4] = IPC_Desc_Buffer(sigdata, IPC_BUFFER_R);
			cmdbuf[5] = (u32)sig;
		}
		break;
	case 0x812:
		SecInfo_SaveToNextSlot();
		cmdbuf[0] = IPC_MakeHeader(0x812, 1, 0);
		cmdbuf[1] = 0;
		break;
	case 0x813:
		cmdbuf[1] = SecInfo_CheckSignature();
		cmdbuf[0] = IPC_MakeHeader(0x813, 1, 0);
		break;
	case 0x814:
		if (!IPC_CompareHeader(cmdbuf[0], 0x814, 1, 2) || !IPC_Is_Desc_Buffer(cmdbuf[2], IPC_BUFFER_W)) {
			cmdbuf[0] = IPC_MakeHeader(0x0, 1, 0);
			cmdbuf[1] = OS_INVALID_IPC_PARAMATER;
		} else {
			void* ptr = (void*)cmdbuf[3];
			size_t size = IPC_Get_Desc_Buffer_Size(cmdbuf[2]);

			cmdbuf[1] = SecInfo_GetData(ptr, size);
			cmdbuf[0] = IPC_MakeHeader(0x814, 1, 2);
			cmdbuf[2] = IPC_Desc_Buffer(size, IPC_BUFFER_W);
			cmdbuf[3] = (u32)ptr;
		}
		break;
	case 0x815:
		if (!IPC_CompareHeader(cmdbuf[0], 0x815, 1, 2) || !IPC_Is_Desc_Buffer(cmdbuf[2], IPC_BUFFER_W)) {
			cmdbuf[0] = IPC_MakeHeader(0x0, 1, 0);
			cmdbuf[1] = OS_INVALID_IPC_PARAMATER;
		} else {
			void* ptr = (void*)cmdbuf[3];
			size_t size = IPC_Get_Desc_Buffer_Size(cmdbuf[2]);

			cmdbuf[1] = SecInfo_GetSignature(ptr, size);
			cmdbuf[0] = IPC_MakeHeader(0x815, 1, 2);
			cmdbuf[2] = IPC_Desc_Buffer(size, IPC_BUFFER_W);
			cmdbuf[3] = (u32)ptr;
		}
		break;
	// case 0x816 at case 0x2
	// case 0x817 at case 0x407
	// case 0x818 at case 0x408
	// case 0x819 at case 0x40A
	case 0x81A:
		{

		}
		break;
	case 0x81B:
		if (!IPC_CompareHeader(cmdbuf[0], 0x81B, 1, 2) || !IPC_Is_Desc_Buffer(cmdbuf[2], IPC_BUFFER_W)) {
			cmdbuf[0] = IPC_MakeHeader(0x0, 1, 0);
			cmdbuf[1] = OS_INVALID_IPC_PARAMATER;
		} else {
			void* ptr = (void*)cmdbuf[3];
			size_t size = IPC_Get_Desc_Buffer_Size(cmdbuf[2]);

			cmdbuf[1] = Hwcal_GetCirclePad(ptr, size);
			cmdbuf[0] = IPC_MakeHeader(0x81B, 1, 2);
			cmdbuf[2] = IPC_Desc_Buffer(size, IPC_BUFFER_W);
			cmdbuf[3] = (u32)ptr;
		}
		break;
	// case 0x81C at case 0x40B
	case 0x81D:
		cmdbuf[1] = Hwcal_ResetCStickCfgBlk();
		cmdbuf[0] = IPC_MakeHeader(0x81D, 1, 0);
		break;
	case 0x81E:
		if (!IPC_CompareHeader(cmdbuf[0], 0x81E, 1, 2) || !IPC_Is_Desc_Buffer(cmdbuf[2], IPC_BUFFER_W)) {
			cmdbuf[0] = IPC_MakeHeader(0x0, 1, 0);
			cmdbuf[1] = OS_INVALID_IPC_PARAMATER;
		} else {
			void* ptr = (void*)cmdbuf[3];
			size_t size = IPC_Get_Desc_Buffer_Size(cmdbuf[2]);

			cmdbuf[1] = Hwcal_GetCStick(ptr, size);
			cmdbuf[0] = IPC_MakeHeader(0x81E, 1, 2);
			cmdbuf[2] = IPC_Desc_Buffer(size, IPC_BUFFER_W);
			cmdbuf[3] = (u32)ptr;
		}
		break;
	case 0x81F:
		cmdbuf[1] = Hwcal_ResetQtmCfgBlk();
		cmdbuf[0] = IPC_MakeHeader(0x81F, 1, 0);
		break;
	case 0x820:
		if (!IPC_CompareHeader(cmdbuf[0], 0x820, 1, 2) || !IPC_Is_Desc_Buffer(cmdbuf[2], IPC_BUFFER_W)) {
			cmdbuf[0] = IPC_MakeHeader(0x0, 1, 0);
			cmdbuf[1] = OS_INVALID_IPC_PARAMATER;
		} else {
			void* ptr = (void*)cmdbuf[3];
			size_t size = IPC_Get_Desc_Buffer_Size(cmdbuf[2]);

			cmdbuf[1] = Hwcal_GetQtm(ptr, size);
			cmdbuf[0] = IPC_MakeHeader(0x820, 1, 2);
			cmdbuf[2] = IPC_Desc_Buffer(size, IPC_BUFFER_W);
			cmdbuf[3] = (u32)ptr;
		}
		break;
	default:
		cmdbuf[0] = IPC_MakeHeader(0x0, 1, 0);
		cmdbuf[1] = OS_INVALID_HEADER;
	}
}

static void CFG_NOR_IPCSession() {
	u32* cmdbuf = getThreadCommandBuffer();

	u16 cmdid = cmdbuf[0] >> 16;

	switch (cmdid) {
	case 0x1:
		if (!IPC_CompareHeader(cmdbuf[0], 0x1, 2, 2) || !IPC_Is_Desc_Buffer(cmdbuf[3], IPC_BUFFER_W)) {
			cmdbuf[0] = IPC_MakeHeader(0x0, 1, 0);
			cmdbuf[1] = OS_INVALID_IPC_PARAMATER;
		} else {

		}
		break;
	default:
		cmdbuf[0] = IPC_MakeHeader(0x0, 1, 0);
		cmdbuf[1] = OS_INVALID_HEADER;
	}
}

static void CFG_IPCSession(int service_index) {
	if (service_index >= CFG_U_SERVICE_INDEX && service_index <= CFG_I_SERVICE_INDEX) {
		CFG_Common_IPCSession(service_index);
	} else if (service_index == CFG_NOR_SERVICE_INDEX) {
		CFG_NOR_IPCSession();
	} else {
		svcBreak(USERBREAK_PANIC);
	}
}

void TryMountCFGSystemSave() {
	Result res = 0;
	for(int i = 0; i < 4; ++i) {
		if (R_SUCCEEDED(Cfg_OpenSysSave())) {
			break;
		}
	}
}

static inline void initBSS() {
	extern void* __bss_start__;
	extern void* __bss_end__;
	memset(__bss_start__, 0, (size_t)__bss_end__ - (size_t)__bss_start__);
}

void CFGMain() {
	initBSS();

	const s32 SERVICE_COUNT = 4;
	const s32 INDEX_MAX = 30;
	const s32 REMOTE_SESSION_INDEX = SERVICE_COUNT + 1;

	Handle session_handles[30];
	u8 service_indexes[25];

	s32 handle_count = SERVICE_COUNT + 1;

	Err_Panic(srvInit());
	Err_FailedThrow(fsInit());

	TryMountCFGSystemSave();
	Cfg_OpenNandAccess();

	Cfg_AtBootConfigLoad();
	Lfcs_Init();
	SecInfo_Init();

	for (int i = 0; i < SERVICE_COUNT; i++)
		Err_FailedThrow(srvRegisterService(&session_handles[i + 1], CFG_ServiceNames[i], 25));

	Err_FailedThrow(srvEnableNotification(&session_handles[0]));

	Handle target = 0;
	s32 target_index = -1;
	for (;;) {
		s32 index;

		if (!target) {
			if (TerminationFlag && handle_count == REMOTE_SESSION_INDEX)
				break;
			else
				*getThreadCommandBuffer() = 0xFFFF0000;
		}

		Result res = svcReplyAndReceive(&index, session_handles, handle_count, target);
		s32 last_target_index = target_index;
		target = 0;
		target_index = -1;

		if (R_FAILED(res)) {

			if (res != OS_REMOTE_SESSION_CLOSED)
				Err_Throw(res);

			else if (index == -1) {
				if (last_target_index == -1)
					Err_Throw(CFG_CANCELED_RANGE);
				else
					index = last_target_index;
			}

			else if (index >= handle_count)
				Err_Throw(CFG_CANCELED_RANGE);

			svcCloseHandle(session_handles[index]);

			handle_count--;
			for (s32 i = index - REMOTE_SESSION_INDEX; i < handle_count - REMOTE_SESSION_INDEX; i++) {
				session_handles[REMOTE_SESSION_INDEX + i] = session_handles[REMOTE_SESSION_INDEX + i + 1];
				service_indexes[i] = service_indexes[i + 1];
			}

			continue;
		}

		if (index == 0)
			HandleSRVNotification();

		else if (index == 1) {
			Handle newsession = 0;
			Err_FailedThrow(svcAcceptSession(&newsession, session_handles[index]));

			if (handle_count >= INDEX_MAX) {
				svcCloseHandle(newsession);
				continue;
			}

			session_handles[handle_count] = newsession;
			service_indexes[handle_count - REMOTE_SESSION_INDEX] = index - 1;
			handle_count++;

		} else if (index >= REMOTE_SESSION_INDEX && index < INDEX_MAX) {
			u8 sindex = service_indexes[index];

			CFG_IPCSession(sindex);
			target = session_handles[index];
			target_index = index;
		} else {
			Err_Throw(CFG_INTERNAL_RANGE);
		}
	}

	for (int i = 0; i < SERVICE_COUNT; i++) {
		Err_FailedThrow(srvUnregisterService(CFG_ServiceNames[i]));
		svcCloseHandle(session_handles[i + 1]);
	}

	svcCloseHandle(session_handles[0]);

	fsExit();
	srvExit();
}
