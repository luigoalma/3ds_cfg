#pragma once
#include <3ds/types.h>
#include <cfg.h>

#ifdef __cplusplus
extern "C" {
#endif

void Lfcs_Init();
void Lfcs_SaveToNextSlot();
Result Lfcs_CheckSignature();
Result Lfcs_GetId(u64* id);
Result Lfcs_GetWholeData(void* data, size_t size);
Result Lfcs_SetSignature(const void* sig, size_t size);
Result Lfcs_SetData(const void* data, size_t size, bool reset_hardcoded);

#ifdef __cplusplus
}
#endif