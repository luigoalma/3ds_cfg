#pragma once
#include <3ds/types.h>
#include <cfg.h>

#ifdef __cplusplus
extern "C" {
#endif

void Cfg_Lfcs_Init();
void Cfg_Lfcs_SaveToNextSlot();
Result Cfg_Lfcs_CheckSignature();
Result Cfg_Lfcs_GetId(u64* id);
Result Cfg_Lfcs_GetWholeData(void* data, size_t size);
Result Cfg_Lfcs_SetSignature(const void* sig, size_t size);
Result Cfg_Lfcs_SetData(const void* data, size_t size, bool reset_hardcoded);

#ifdef __cplusplus
}
#endif