#pragma once
#include <3ds/types.h>
#include "hwcal_types.h"

#ifdef __cplusplus
extern "C" {
#endif

Result Hwcal_GetCirclePad(void* ptr, size_t size);
Result Hwcal_ResetCirclePadCfgBlk();
Result Hwcal_GetCStick(void* ptr, size_t size);
Result Hwcal_ResetCStickCfgBlk();
Result Hwcal_GetQtm(void* ptr, size_t size);
Result Hwcal_ResetQtmCfgBlk();

#ifdef __cplusplus
}
#endif
