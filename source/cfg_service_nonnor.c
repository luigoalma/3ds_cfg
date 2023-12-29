#include <3ds/ipc.h>
#include <3ds/result.h>
#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <cfg.h>
#include <cfg_service.h>
#include <storage/config.h>
#include <storage/hwcal.h>
#include <storage/lfcs.h>
#include <storage/secinfo.h>
#include "country.h"

void CFG_Common_IPCSession(int service_index) {
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
			cmdbuf[1] = Cfg_GetSystemModel(&model);
			cmdbuf[0] = IPC_MakeHeader(0x5, 2, 0);
			cmdbuf[2] = model;
		}
		break;
	case 0x6:
		{
			CFG_SystemModel model;
			Result res = Cfg_GetSystemModel(&model);
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
			cmdbuf[2] = unk;
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
		cmdbuf[1] = Hwcal_ResetAccelerometerCfgBlk();
		cmdbuf[0] = IPC_MakeHeader(0x409, 1, 0);
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
		cmdbuf[1] = Hwcal_ResetGyroscopeCfgBlk();
		cmdbuf[0] = IPC_MakeHeader(0x40C, 1, 0);
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
			CFG_BlkFlags flags = (CFG_BlkFlags)(cmdbuf[3] & 0xFFFF);

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
		Cfg_DeleteAndSetDefaultBlks();
		cmdbuf[0] = IPC_MakeHeader(0x806, 1, 0);
		cmdbuf[1] = 0;
		break;
	case 0x807:
		// originally resulted in wasted cycles to reach a no-op
		// so did nothing at all
		cmdbuf[0] = IPC_MakeHeader(0x807, 1, 0);
		cmdbuf[1] = 0;
		break;
	case 0x808:
		cmdbuf[0] = IPC_MakeHeader(0x808, 1, 0);
		cmdbuf[1] = Cfg_UpgradeSave();
		break;
	case 0x809:
		{
			Result res = Hwcal_ResetCirclePadCfgBlk();

			if(R_SUCCEEDED(res))
				srvPublishToSubscriber(0x10B, 0);

			cmdbuf[0] = IPC_MakeHeader(0x809, 1, 0);
			cmdbuf[1] = res;
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
		cmdbuf[1] = Lfcs_CheckSignature();
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
			cmdbuf[4] = IPC_Desc_Buffer(sigsize, IPC_BUFFER_R);
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
		if (!IPC_CompareHeader(cmdbuf[0], 0x81A, 1, 2) || !IPC_Is_Desc_Buffer(cmdbuf[2], IPC_BUFFER_W)) {
			cmdbuf[0] = IPC_MakeHeader(0x0, 1, 0);
			cmdbuf[1] = OS_INVALID_IPC_PARAMATER;
		} else {
			void* ptr = (void*)cmdbuf[3];
			size_t size = IPC_Get_Desc_Buffer_Size(cmdbuf[2]);

			cmdbuf[1] = Hwcal_GetOuterCams(ptr, size);
			cmdbuf[0] = IPC_MakeHeader(0x81A, 1, 2);
			cmdbuf[2] = IPC_Desc_Buffer(size, IPC_BUFFER_W);
			cmdbuf[3] = (u32)ptr;
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
