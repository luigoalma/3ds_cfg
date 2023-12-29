#include <string.h>
#include <3ds/ipc.h>
#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/spi.h>
#include <err.h>
#include <cfg.h>
#include <cfg_service.h>

static struct {
	Handle spiHandle;
	bool nspiSet;
} NORHandler = {0, false};

void NOR_ServiceInit() {
	Err_Panic(spiNORInit(&NORHandler.spiHandle));
	NORHandler.nspiSet = false;
}

void NOR_ServiceExit() {
	spiExit(&NORHandler.spiHandle);
}

static Result NOR_ToggleMode(u8 toggle) {
	NORHandler.nspiSet = toggle ? true : false;
	return SPI_SetDeviceNSPIModeAndRate(
		NORHandler.spiHandle,
		1,
		toggle ? true : false,
		toggle ? 3 : 0
	);
}

static Result NOR_Initialize() {
	Result res = SPI_InitDeviceRate(NORHandler.spiHandle, 1, 0);
	if(R_FAILED(res))
		return res;

	NOR_ToggleMode(true);
	return 0;
}

// NVRAM Write Enable
static Result NOR_SendWREN() {
	ALIGN(4) char cmd[4];

	*(u32*)cmd = 6;
	return SPI_SendCmdOnly(NORHandler.spiHandle, 1, &cmd[0], 1);
}

// NVRAM Write Disable
static Result NOR_SendWRDI() {
	ALIGN(4) char cmd[4];

	*(u32*)cmd = 4;
	return SPI_SendCmdOnly(NORHandler.spiHandle, 1, &cmd[0], 1);
}

// NVRAM Deep Sleep
static Result NOR_DeepSleep() {
	ALIGN(4) char cmd[4];

	*(u32*)cmd = 0xB9;
	return SPI_SendCmdOnly(NORHandler.spiHandle, 1, &cmd[0], 1);
}

// NVRAM Wake Deep Sleep
static Result NOR_WakeDeepSleep() {
	ALIGN(4) char cmd[4];

	*(u32*)cmd = 0xAB;
	return SPI_SendCmdOnly(NORHandler.spiHandle, 1, &cmd[0], 1);
}

// NVRAM Read Device Identification - Chip ID
static Result NOR_GetRDIDChipID(u8* out) {
	ALIGN(4) char cmd[4];
	u8 data[2];

	*(u32*)cmd = 0x9F;
	Result res = SPI_SendCmdAndShortRead(NORHandler.spiHandle, 1, &cmd[0], 1, &data, sizeof(data));

	out[0] = data[0];
	out[1] = data[1];
	return res;
}

// NVRAM Read Device Status Register
static Result NOR_GetRDSR(u8* out) {
	ALIGN(4) char cmd[4];
	u8 data[1];

	*(u32*)cmd = 5;
	Result res = SPI_SendCmdAndShortRead(NORHandler.spiHandle, 1, &cmd[0], 1, &data, sizeof(data));

	out[0] = data[0];
	return res;
}

static bool NOR_GetBusyState() {
	u8 rdsr;
	Err_Panic(NOR_GetRDSR(&rdsr)); // cfg doesnt originally check if it failed
	return (rdsr & 0x1) ? true : false;
}

static bool NOR_GetWritableState() {
	u8 rdsr;
	Err_Panic(NOR_GetRDSR(&rdsr)); // cfg doesnt originally check if it failed
	return (rdsr & 0x2) ? true : false;
}

static void NOR_WaitForNonBusy() {
	while(NOR_GetBusyState()) {}
}

static void NOR_WaitForWritable() {
	while(!NOR_GetWritableState()) {}
}

// page size: 100h
static Result NOR_ErasePage(u32 offset) {
	ALIGN(4) char cmd[4];

	cmd[0] = 0xDB;
	cmd[1] = (offset >> 16) & 0xFF;
	cmd[2] = (offset >> 8) & 0xFF;
	cmd[3] = offset & 0xFF;
	Err_Panic(NOR_SendWREN()); // cfg doesnt originally check if it failed
	NOR_WaitForWritable();
	return SPI_SendCmdOnly(NORHandler.spiHandle, 1, &cmd[0], 4);
}

// sector size: 10000h
static Result NOR_EraseSector(u32 offset) {
	ALIGN(4) char cmd[4];

	cmd[0] = 0xD8;
	cmd[1] = (offset >> 16) & 0xFF;
	cmd[2] = (offset >> 8) & 0xFF;
	cmd[3] = offset & 0xFF;
	Err_Panic(NOR_SendWREN()); // cfg doesnt originally check if it failed
	NOR_WaitForWritable();
	return SPI_SendCmdOnly(NORHandler.spiHandle, 1, &cmd[0], 4);
}

static Result NOR_SingleRead(u32 offset, void* data, size_t dataLen) {
	ALIGN(4) char cmd[4];

	cmd[0] = 3;
	cmd[1] = (offset >> 16) & 0xFF;
	cmd[2] = (offset >> 8) & 0xFF;
	cmd[3] = offset & 0xFF;
	NOR_WaitForNonBusy();
	return SPI_SendCmdAndRead(NORHandler.spiHandle, 1, &cmd[0], 4, data, dataLen);
}

static Result NOR_SingleWrite(u32 offset, const void* data, size_t dataLen) {
	ALIGN(4) char cmd[4];
	size_t size;

	size = 256 - (offset & 0xFF);
	size = (dataLen <= size) ? dataLen : size;

	cmd[0] = 0xA;
	cmd[1] = (offset >> 16) & 0xFF;
	cmd[2] = (offset >> 8) & 0xFF;
	cmd[3] = offset & 0xFF;
	Err_Panic(NOR_SendWREN()); // cfg doesnt originally check if it failed
	NOR_WaitForWritable();
	return SPI_SendCmdAndWrite(NORHandler.spiHandle, 1, &cmd[0], 4, data, dataLen);
}

static void NOR_Write(u32 offset, const void* data, size_t dataLen) {
	const u8* _data = (const u8*)data;

	u8 buf[0x100];

	u32 page_addr = offset & 0xFFFFFF00;
	u8 page_off = offset & 0xFF;

	size_t buf_left = 256 - page_off;

	size_t size = (dataLen > buf_left) ? buf_left : dataLen;
	size_t remainder_size = buf_left - size;

	if(page_off)
		Err_Panic(NOR_SingleRead(page_addr, buf, page_off));

	while(dataLen) {
		memcpy(&buf[page_off], _data, size);

		if(remainder_size) {
			u32 remainder_off = 256 - remainder_size;
			Err_Panic(NOR_SingleRead(page_addr + remainder_off, &buf[remainder_off], remainder_size));
		}

		Err_Panic(NOR_SingleWrite(page_addr, buf, 256));
		NOR_WaitForNonBusy();

		page_addr += 256;
		page_off = 0;
		_data += size;
		dataLen -= size;

		size = (dataLen > 256) ? 256 : dataLen;
		remainder_size = 256 - size;
	}
}

void CFG_NOR_IPCSession() {
	u32* cmdbuf = getThreadCommandBuffer();

	u16 cmdid = cmdbuf[0] >> 16;

	switch (cmdid) {
	case 0x1:
		cmdbuf[1] = NOR_Initialize();
		cmdbuf[0] = IPC_MakeHeader(0x1, 1, 0);
		break;
	case 0x2:
		SPI_IPC0x2(NORHandler.spiHandle, 1);
		cmdbuf[0] = IPC_MakeHeader(0x2, 1, 0);
		cmdbuf[1] = 0;
		break;
	case 0x3:
		cmdbuf[1] = NOR_ToggleMode(cmdbuf[1] & 0xFF);
		cmdbuf[0] = IPC_MakeHeader(0x3, 1, 0);
		break;
	case 0x4:
		cmdbuf[0] = IPC_MakeHeader(0x4, 2, 0);
		cmdbuf[1] = 0;
		cmdbuf[2] = NORHandler.nspiSet;
		break;
	case 0x5:
		if (!IPC_CompareHeader(cmdbuf[0], 0x5, 2, 2) || !IPC_Is_Desc_Buffer(cmdbuf[3], IPC_BUFFER_W)) {
			cmdbuf[0] = IPC_MakeHeader(0x0, 1, 0);
			cmdbuf[1] = OS_INVALID_IPC_PARAMATER;
		} else {
			void* ptr = (void*)cmdbuf[4];
			size_t size = IPC_Get_Desc_Buffer_Size(cmdbuf[3]);

			cmdbuf[1] = NOR_SingleRead(cmdbuf[2], ptr, size);
			cmdbuf[0] = IPC_MakeHeader(0x5, 1, 2);
			cmdbuf[2] = IPC_Desc_Buffer(size, IPC_BUFFER_W);
			cmdbuf[3] = (u32)ptr;
		}
		break;
	case 0x6:
		if (!IPC_CompareHeader(cmdbuf[0], 0x6, 2, 2) || !IPC_Is_Desc_Buffer(cmdbuf[3], IPC_BUFFER_R)) {
			cmdbuf[0] = IPC_MakeHeader(0x0, 1, 0);
			cmdbuf[1] = OS_INVALID_IPC_PARAMATER;
		} else {
			void* ptr = (void*)cmdbuf[4];
			size_t size = IPC_Get_Desc_Buffer_Size(cmdbuf[3]);

			NOR_Write(cmdbuf[2], ptr, size);
			cmdbuf[0] = IPC_MakeHeader(0x6, 1, 2);
			cmdbuf[1] = 0;
			cmdbuf[2] = IPC_Desc_Buffer(size, IPC_BUFFER_R);
			cmdbuf[3] = (u32)ptr;
		}
		break;
	case 0x7:
		cmdbuf[1] = NOR_SendWREN();
		cmdbuf[0] = IPC_MakeHeader(0x7, 1, 0);
		break;
	case 0x8:
		cmdbuf[1] = NOR_SendWRDI();
		cmdbuf[0] = IPC_MakeHeader(0x8, 1, 0);
		break;
	case 0x9:
		cmdbuf[1] = NOR_GetRDIDChipID((u8*)&cmdbuf[2]);
		cmdbuf[0] = IPC_MakeHeader(0x9, 2, 0);
		break;
	case 0xA:
		NOR_WaitForNonBusy();
		cmdbuf[0] = IPC_MakeHeader(0xA, 1, 0);
		cmdbuf[1] = 0;
		break;
	case 0xB:
		NOR_WaitForWritable();
		cmdbuf[0] = IPC_MakeHeader(0xB, 1, 0);
		cmdbuf[1] = 0;
		break;
	case 0xC:
		cmdbuf[2] = NOR_GetBusyState();
		cmdbuf[0] = IPC_MakeHeader(0xC, 2, 0);
		cmdbuf[1] = 0;
		break;
	case 0xD:
		cmdbuf[2] = NOR_GetWritableState();
		cmdbuf[0] = IPC_MakeHeader(0xD, 2, 0);
		cmdbuf[1] = 0;
		break;
	case 0xE:
		cmdbuf[1] = NOR_GetRDSR((u8*)&cmdbuf[2]);
		cmdbuf[0] = IPC_MakeHeader(0xE, 2, 0);
		break;
	case 0xF:
		if (!IPC_CompareHeader(cmdbuf[0], 0xF, 2, 2) || !IPC_Is_Desc_Buffer(cmdbuf[3], IPC_BUFFER_W)) {
			cmdbuf[0] = IPC_MakeHeader(0x0, 1, 0);
			cmdbuf[1] = OS_INVALID_IPC_PARAMATER;
		} else {
			void* ptr = (void*)cmdbuf[4];
			size_t size = IPC_Get_Desc_Buffer_Size(cmdbuf[3]);

			cmdbuf[1] = NOR_SingleRead(cmdbuf[2], ptr, size);
			cmdbuf[0] = IPC_MakeHeader(0xF, 1, 2);
			cmdbuf[2] = IPC_Desc_Buffer(size, IPC_BUFFER_W);
			cmdbuf[3] = (u32)ptr;
		}
		break;
	case 0x10:
		if (!IPC_CompareHeader(cmdbuf[0], 0x10, 2, 2) || !IPC_Is_Desc_Buffer(cmdbuf[3], IPC_BUFFER_R)) {
			cmdbuf[0] = IPC_MakeHeader(0x0, 1, 0);
			cmdbuf[1] = OS_INVALID_IPC_PARAMATER;
		} else {
			const void* ptr = (const void*)cmdbuf[4];
			size_t size = IPC_Get_Desc_Buffer_Size(cmdbuf[3]);

			cmdbuf[1] = NOR_SingleWrite(cmdbuf[2], ptr, size);
			cmdbuf[0] = IPC_MakeHeader(0x10, 1, 2);
			cmdbuf[2] = IPC_Desc_Buffer(size, IPC_BUFFER_R);
			cmdbuf[3] = (u32)ptr;
		}
		break;
	case 0x11:
		cmdbuf[1] = NOR_ErasePage(cmdbuf[1]);
		cmdbuf[0] = IPC_MakeHeader(0x11, 1, 0);
		break;
	case 0x12:
		cmdbuf[1] = NOR_EraseSector(cmdbuf[1]);
		cmdbuf[0] = IPC_MakeHeader(0x12, 1, 0);
		break;
	case 0x13:
		cmdbuf[1] = NOR_DeepSleep();
		cmdbuf[0] = IPC_MakeHeader(0x13, 1, 0);
		break;
	case 0x14:
		cmdbuf[1] = NOR_WakeDeepSleep();
		cmdbuf[0] = IPC_MakeHeader(0x14, 1, 0);
		break;
	default:
		cmdbuf[0] = IPC_MakeHeader(0x0, 1, 0);
		cmdbuf[1] = OS_INVALID_HEADER;
	}
}
