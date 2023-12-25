#pragma once
#include <3ds/types.h>

#ifdef __cplusplus
extern "C" {
#endif

Result spiNORInit(Handle* spiNORHandle);
void spiExit(Handle* spiHandle);
Result SPI_InitDeviceRate(Handle spiHandle, u8 deviceId, u8 rate);
Result SPI_IPC0x2(Handle spiHandle, u8 deviceId);
Result SPI_SendCmdAndShortRead(Handle spiHandle, u8 deviceId, const void* cmd, size_t cmdLen, void* data, size_t dataLen);
Result SPI_SendCmdOnly(Handle spiHandle, u8 deviceId, const void* cmd, size_t cmdLen);
Result SPI_SendCmdAndRead(Handle spiHandle, u8 deviceId, const void* cmd, size_t cmdLen, void* data, size_t dataLen);
Result SPI_SendCmdAndWrite(Handle spiHandle, u8 deviceId, const void* cmd, size_t cmdLen, const void* data, size_t dataLen);
Result SPI_SetDeviceNSPIModeAndRate(Handle spiHandle, u8 deviceId, u8 enableNspi, u8 rate);

#ifdef __cplusplus
}
#endif
