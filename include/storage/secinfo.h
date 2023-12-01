#pragma once
#include <3ds/types.h>
#include <cfg.h>

#ifdef __cplusplus
extern "C" {
#endif

void SecInfo_Init();
void SecInfo_SaveToNextSlot();
Result SecInfo_CheckSignature();
Result SecInfo_GetSignature(void* sig, size_t size);
Result SecInfo_GetRegionByte(u8* region);
Result SecInfo_GetByte0x101(u8* unk);
Result SecInfo_GetSerialNumber(char* serial, size_t size);
Result SecInfo_GetData(void* data, size_t size);
Result SecInfo_SetWholeSecInfo(const void* sig, size_t sigsize, const void* data, size_t datasize);

#ifdef __cplusplus
}
#endif