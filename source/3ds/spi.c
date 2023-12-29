#include <string.h>
#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/ipc.h>
#include <3ds/result.h>
#include <3ds/srv.h>
#include <3ds/spi.h>

Result spiNORInit(Handle* spiNORHandle)
{
	*spiNORHandle = 0;
	return srvGetServiceHandle(spiNORHandle, "SPI::NOR");
}

void spiExit(Handle* spiHandle)
{
	svcCloseHandle(*spiHandle);
	*spiHandle = 0;
}

Result SPI_InitDeviceRate(Handle spiHandle, u8 deviceId, u8 rate) {
	Result res = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1, 2, 0); // 0x10080
	cmdbuf[1] = deviceId & 0xFF;
	cmdbuf[2] = rate & 0xFF;

	res = svcSendSyncRequest(spiHandle);

	if(R_FAILED(res))
		return res;

	return (Result)cmdbuf[1];
}

Result SPI_IPC0x2(Handle spiHandle, u8 deviceId) {
	Result res = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x2, 1, 0); // 0x20040
	cmdbuf[1] = deviceId & 0xFF;

	res = svcSendSyncRequest(spiHandle);

	if(R_FAILED(res))
		return res;

	return (Result)cmdbuf[1];
}

Result SPI_SendCmdAndShortRead(Handle spiHandle, u8 deviceId, const void* cmd, size_t cmdLen, void* data, size_t dataLen) {
	Result res = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x3, 4, 0); // 0x30100
	cmdbuf[1] = deviceId & 0xFF;
	cmdbuf[2] = *(const u32*)cmd;
	cmdbuf[3] = cmdLen;
	cmdbuf[4] = dataLen;

	res = svcSendSyncRequest(spiHandle);

	if(R_FAILED(res))
		return res;

	if(data) memcpy(data, &cmdbuf[2], dataLen);

	return (Result)cmdbuf[1];
}

Result SPI_SendCmdOnly(Handle spiHandle, u8 deviceId, const void* cmd, size_t cmdLen) {
	Result res = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x5, 3, 0); // 0x50C00
	cmdbuf[1] = deviceId & 0xFF;
	cmdbuf[2] = *(const u32*)cmd;
	cmdbuf[3] = cmdLen;

	res = svcSendSyncRequest(spiHandle);

	if(R_FAILED(res))
		return res;

	return (Result)cmdbuf[1];
}


Result SPI_SendCmdAndRead(Handle spiHandle, u8 deviceId, const void* cmd, size_t cmdLen, void* data, size_t dataLen) {
	Result res = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x6, 4, 2); // 0x60102
	cmdbuf[1] = deviceId & 0xFF;
	cmdbuf[2] = *(const u32*)cmd;
	cmdbuf[3] = cmdLen;
	cmdbuf[4] = dataLen;
	cmdbuf[5] = IPC_Desc_Buffer(dataLen, IPC_BUFFER_W);
	cmdbuf[6] = (u32)data;

	res = svcSendSyncRequest(spiHandle);

	if(R_FAILED(res))
		return res;

	return (Result)cmdbuf[1];
}

Result SPI_SendCmdAndWrite(Handle spiHandle, u8 deviceId, const void* cmd, size_t cmdLen, const void* data, size_t dataLen) {
	Result res = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x7, 4, 2); // 0x70102
	cmdbuf[1] = deviceId & 0xFF;
	cmdbuf[2] = *(const u32*)cmd;
	cmdbuf[3] = cmdLen;
	cmdbuf[4] = dataLen;
	cmdbuf[5] = IPC_Desc_Buffer(dataLen, IPC_BUFFER_R);
	cmdbuf[6] = (u32)data;

	res = svcSendSyncRequest(spiHandle);

	if(R_FAILED(res))
		return res;

	return (Result)cmdbuf[1];
}

Result SPI_SetDeviceNSPIModeAndRate(Handle spiHandle, u8 deviceId, u8 enableNspi, u8 rate) {
	Result res = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x8, 3, 0); // 0x800C0
	cmdbuf[1] = deviceId & 0xFF;
	cmdbuf[2] = enableNspi & 0xFF;
	cmdbuf[3] = rate & 0xFF;

	res = svcSendSyncRequest(spiHandle);

	if(R_FAILED(res))
		return res;

	return (Result)cmdbuf[1];
}
