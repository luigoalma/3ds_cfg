#include <string.h>
#include <3ds/result.h>
#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/fs.h>
#include <3ds/ps.h>
#include <cfg.h>
#include <cfg_service.h>
#include <err.h>
#include <utils/sha256.h>
#include <utils/endian.h>
#include <storage/config.h>
#include <storage/lfcs.h>
#include <storage/secinfo.h>
#include <debugging/debug.h>

static __attribute__((section(".data.TerminationFlag"))) bool TerminationFlag = false;

static const char* const CFG_ServiceNames[] = {"cfg:u", "cfg:s", "cfg:i", "cfg:nor"};

inline static void HandleSRVNotification() {
	u32 id;
	Err_FailedThrow(srvReceiveNotification(&id));
	if (id == 0x100)
		TerminationFlag = true;
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

static void TryMountCFGSystemSave() {
	for(int i = 0; i < 4; ++i) {
		if (R_SUCCEEDED(Cfg_OpenSysSave())) {
			break;
		}
	}
}

// not really sure what to call it
static void DeriveIds() {
	void* ptr;
	u64 transfer_id_base;
	bool need_save = false;

	if(R_FAILED(Cfg_System_GetBlkPtr(&ptr, 0x90001, 8)))
		return;

	transfer_id_base = *(u64*)ptr;

	if(transfer_id_base & 0x3FFFFFFFFLL) { // LFCS id part

		if(R_FAILED(Lfcs_GetId(&transfer_id_base)))
			return;

		u16 random;
		if(R_SUCCEEDED(psInit())) {
			PS_GenerateRandomBytes(&random, 2);
			psExit();
		}
		
		transfer_id_base &= 0x3FFFFFFFFLLU;
		transfer_id_base |= (u64)random << 48;

		*(u64*)ptr = transfer_id_base;

		u32 random32 = random;
		if(R_FAILED(Cfg_System_WriteBlk(&random32, 0x90002, 4)))
			return;

		need_save = true;
	}

	if(R_FAILED(Cfg_System_GetBlkPtr(&ptr, 0x100003, 0x10)))
		return;

	if(*(u64*)ptr) {
		if(need_save)
			Cfg_SaveConfig();
		return;
	}

	union {
		u64 v64[2];
		u16 v16[8];
		u8 v8[16];
	} twl_movable;

	// ___
	twl_movable.v16[0] = 0x5F5F;
	twl_movable.v8[2] = 0x5F;

	if(R_SUCCEEDED(psInit())) {
		PS_GenerateRandomBytes(&twl_movable.v8[3], 5);
		psExit();
	}
	Cfg_TransferableId(&twl_movable.v64[1], 0);

	((u64*)ptr)[0] = twl_movable.v64[0] ^ 0x85350AADF30EB782LLU;
	((u64*)ptr)[1] = twl_movable.v64[1] ^ 0x00D506E714AE84AALLU;

	Cfg_SaveConfig();
	return;
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

	DebugOut_Init();

	TryMountCFGSystemSave();
	Cfg_OpenNandAccess();

	Cfg_AtBootConfigLoad();
	Lfcs_Init();
	SecInfo_Init();
	NOR_ServiceInit();
	Cfg_UpgradeSave();
	DeriveIds();

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

		else if (index >= 1 && index < REMOTE_SESSION_INDEX) {
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
			u8 sindex = service_indexes[index - REMOTE_SESSION_INDEX];

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

	NOR_ServiceExit();

	DebugOut_Exit();

	fsExit();
	srvExit();
}
