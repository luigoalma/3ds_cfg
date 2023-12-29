/**
 * @file os.h
 * @brief OS related stuff.
 */
#pragma once
#include "types.h"
#include "svc.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum PACKED RUNNING_HW {
    HW_UNKNOWN = 0,
    HW_PRODUCT = 1,
    HW_TS_BOARD = 2,
    HW_KMC_DEBUGGER = 3,
    HW_KMC_CAPTURE = 4,
    HW_IS_DEBUGGER = 5,
    HW_SNAKE_PRODUCT = 6,
    HW_SNAKE_IS_DEBUGGER = 7,
    HW_SNAKE_IS_CAPTURE = 8,
    HW_SNAKE_KMC_DEBUGGER = 9, 
} RUNNING_HW;

static inline bool osEnvInfoIsDev(void)
{
	return ((*(vu32*)0x1FF80014) & 0x1) == 0;
}

static inline bool osPtmFinishedSettingHwConfigInfo(void)
{
    return ((*(vu8*)0x1FF81086) & 0x1) != 0;
}

static inline RUNNING_HW osRunningHw(void)
{
    return (RUNNING_HW)(*(vu8*)0x1FF81006);
}

static inline RUNNING_HW osWaitRunningHw(void)
{
    while(!osPtmFinishedSettingHwConfigInfo())
        svcSleepThread(1000000LLU);
    return osRunningHw();
}

#ifdef __cplusplus
}
#endif
