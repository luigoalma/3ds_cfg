#include <string.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/ps.h>
#include <3ds/ipc.h>

static Handle psHandle;
static int psRefCount;

Result psInit(void)
{
	if (psRefCount++) return 0;
	Result res = srvGetServiceHandle(&psHandle, "ps:ps");
	if (R_FAILED(res)) --psRefCount;
	return res;
}

void psExit(void)
{
	psRefCount = (psRefCount <= 0) ? 0 : psRefCount - 1;
	if (psRefCount) return;
	svcCloseHandle(psHandle);
	psHandle = 0;
}

Result PS_VerifyRsaSha256(u8 *hash, const PS_RSA_Context *ctx, u8 *signature)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 size;

	size = ctx->rsa_bit_size>>3;

	cmdbuf[0] = IPC_MakeHeader(0x2,9,4); // 0x20244
	memcpy(&cmdbuf[1], hash, 32);
	cmdbuf[9] = size;
	cmdbuf[10] = IPC_Desc_StaticBuffer(sizeof(PS_RSA_Context), 0);
	cmdbuf[11] = (u32)ctx;
	cmdbuf[12] = IPC_Desc_Buffer(size, IPC_BUFFER_R);
	cmdbuf[13] = (u32)signature;

	if(R_FAILED(ret = svcSendSyncRequest(psHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result PS_GetLocalFriendCodeSeed(u64* seed)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xA,0,0); // 0xA0000

	if(R_FAILED(ret = svcSendSyncRequest(psHandle)))return ret;

	*seed = (u64)cmdbuf[2] | (u64)cmdbuf[3] << 32;

	return (Result)cmdbuf[1];
}

Result PS_GenerateRandomBytes(void* out, size_t len)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xD,1,2); // 0xD0042
	cmdbuf[1] = len;
	cmdbuf[2] = IPC_Desc_Buffer(len, IPC_BUFFER_W);
	cmdbuf[3] = (u32)out;

	if(R_FAILED(ret = svcSendSyncRequest(psHandle)))return ret;

	return (Result)cmdbuf[1];
}
