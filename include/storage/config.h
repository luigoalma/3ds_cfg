#pragma once
#include <3ds/types.h>
#include <cfg.h>

#ifdef __cplusplus
extern "C" {
#endif

Result Cfg_FormatSysSave();
Result Cfg_OpenSysSave();
Result Cfg_CommitSysSave();
void Cfg_OpenNandAccess();
void Cfg_DeleteFixData();
void Cfg_AtBootConfigLoad();
void Cfg_SaveToFixData();
void Cfg_SaveConfig();
void Cfg_DeleteAndResetConfig();
void Cfg_DeleteAndSetDefaultBlks();
Result Cfg_User_ReadBlk(void* ptr, u32 blkId, size_t size);
Result Cfg_System_ReadBlk(void* ptr, u32 blkId, size_t size);
Result Cfg_System_WriteBlk(const void* ptr, u32 blkId, size_t size);
Result Cfg_System_WriteBlkWithPublish(const void* ptr, u32 blkId, size_t size);
Result Cfg_System_GetBlkPtr(void** ptr, u32 blkId, size_t size);
Result Cfg_CreateBlk(void** ptr, u32 blkId, size_t size, CFG_BlkFlags flags);
Result Cfg_CreateBlkWithData(const void* ptr, u32 blkId, size_t size, CFG_BlkFlags flags);

#ifdef __cplusplus
}
#endif
