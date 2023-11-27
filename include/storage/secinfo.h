#pragma once
#include <3ds/types.h>
#include <cfg.h>

#ifdef __cplusplus
extern "C" {
#endif

void Cfg_SecInfo_Init();
void Cfg_SecInfo_SaveToNextSlot();
Result Cfg_SecInfo_CheckSignature();
Result Cfg_SecInfo_GetSignature(void* sig, size_t size);
Result Cfg_SecInfo_GetRegionByte(u8* region);
Result Cfg_SecInfo_GetByte0x101(u8* unk);
Result Cfg_SecInfo_GetSerialNumber(char* serial, size_t size);
Result Cfg_SecInfo_GetData(void* data, size_t size);
Result Cfg_SecInfo_SetWholeSecInfo(const void* sig, size_t sigsize, const void* data, size_t datasize);

#ifdef __cplusplus
}
#endif