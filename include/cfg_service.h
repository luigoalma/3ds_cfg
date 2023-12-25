#pragma once
#include <3ds/types.h>
#include <3ds/result.h>

#define CFG_U_SERVICE_INDEX   0
#define CFG_S_SERVICE_INDEX   1
#define CFG_I_SERVICE_INDEX   2
#define CFG_NOR_SERVICE_INDEX 3

#define OS_REMOTE_SESSION_CLOSED MAKERESULT(RL_STATUS,    RS_CANCELED, RM_OS, 26)
#define OS_INVALID_HEADER        MAKERESULT(RL_PERMANENT, RS_WRONGARG, RM_OS, 47)
#define OS_INVALID_IPC_PARAMATER MAKERESULT(RL_PERMANENT, RS_WRONGARG, RM_OS, 48)

#ifdef __cplusplus
extern "C" {
#endif

void CFG_Common_IPCSession(int service_index);
void CFG_NOR_IPCSession();

void NOR_ServiceInit();
void NOR_ServiceExit();

#ifdef __cplusplus
}
#endif
