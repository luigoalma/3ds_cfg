#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/ipc.h>
#include <3ds/result.h>
#include <3ds/srv.h>
#include <3ds/i2c.h>
#include <memops.h>

Result i2cEEPInit(Handle* i2cEEPHandle)
{
	*i2cEEPHandle = 0;
	return srvGetServiceHandle(i2cEEPHandle, "i2c::EEP");
}

void i2cExit(Handle* i2cHandle)
{
	svcCloseHandle(*i2cHandle);
	*i2cHandle = 0;
}

// length must be divisable by 2
Result I2C_ReadEEPROM(Handle i2cHandle, u8 device, void* out, u16 offset, size_t length) {
	Result res = 0;
	u8* tls = (u8*)getThreadLocalStorage();
	u32* cmdbuf = (u32*)(tls+0x80);
	u32* staticbuf = (u32*)(tls+0x180);

	cmdbuf[0] = IPC_MakeHeader(0x10,3,0); // 0x1000C0
	cmdbuf[1] = device & 0xFF;
	cmdbuf[2] = offset & 0xFFFF;
	cmdbuf[3] = length >> 1;

	u32 saved_staticbuf[2];
	saved_staticbuf[0] = staticbuf[0];
	saved_staticbuf[1] = staticbuf[1];

	staticbuf[0] = IPC_Desc_StaticBuffer(length & ~1U, 0);
	staticbuf[1] = (u32)out;

	res = svcSendSyncRequest(i2cHandle);

	staticbuf[0] = saved_staticbuf[0];
	staticbuf[1] = saved_staticbuf[1];

	if(R_FAILED(res))
		return res;

	return (Result)cmdbuf[1];
}
