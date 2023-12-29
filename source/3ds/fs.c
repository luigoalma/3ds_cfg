#include <string.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/fs.h>
#include <3ds/ipc.h>

static Handle fsuHandle;
static int fsuRefCount;

Result fsInit(void)
{
	Result ret = 0;

	if (fsuRefCount++) return 0;

	ret = srvGetServiceHandle(&fsuHandle, "fs:USER");
	if (R_SUCCEEDED(ret))
	{
		ret = FSUSER_Initialize(fsuHandle);
		if (R_FAILED(ret)) svcCloseHandle(fsuHandle);
	}

	if (R_FAILED(ret)) --fsuRefCount;
	return ret;
}

void fsExit(void)
{
	if (--fsuRefCount) return;
	svcCloseHandle(fsuHandle);
}

Result FSUSER_Initialize(Handle session)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x801,0,2); // 0x8010002
	cmdbuf[1] = IPC_Desc_CurProcessId();

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(session))) return ret;

	return cmdbuf[1];
}

Result FSUSER_OpenFile(Handle* out, FS_Archive archive, FS_Path path, u32 openFlags, u32 attributes)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x802,7,2); // 0x80201C2
	cmdbuf[1] = 0;
	cmdbuf[2] = (u32) archive;
	cmdbuf[3] = (u32) (archive >> 32);
	cmdbuf[4] = path.type;
	cmdbuf[5] = path.size;
	cmdbuf[6] = openFlags;
	cmdbuf[7] = attributes;
	cmdbuf[8] = IPC_Desc_StaticBuffer(path.size, 0);
	cmdbuf[9] = (u32) path.data;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsuHandle))) return ret;

	if(out) *out = cmdbuf[3];

	return cmdbuf[1];
}

Result FSUSER_DeleteFile(FS_Archive archive, FS_Path path)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x804,5,2); // 0x8040142
	cmdbuf[1] = 0;
	cmdbuf[2] = (u32) archive;
	cmdbuf[3] = (u32) (archive >> 32);
	cmdbuf[4] = path.type;
	cmdbuf[5] = path.size;
	cmdbuf[6] = IPC_Desc_StaticBuffer(path.size, 0);
	cmdbuf[7] = (u32) path.data;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsuHandle))) return ret;

	return cmdbuf[1];
}

Result FSUSER_RenameFile(FS_Archive srcArchive, FS_Path srcPath, FS_Archive dstArchive, FS_Path dstPath)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x805,9,4); // 0x8050244
	cmdbuf[1] = 0;
	cmdbuf[2] = (u32) srcArchive;
	cmdbuf[3] = (u32) (srcArchive >> 32);
	cmdbuf[4] = srcPath.type;
	cmdbuf[5] = srcPath.size;
	cmdbuf[6] = (u32) dstArchive;
	cmdbuf[7] = (u32) (dstArchive >> 32);
	cmdbuf[8] = dstPath.type;
	cmdbuf[9] = dstPath.size;
	cmdbuf[10] = IPC_Desc_StaticBuffer(srcPath.size, 1);
	cmdbuf[11] = (u32) srcPath.data;
	cmdbuf[12] = IPC_Desc_StaticBuffer(dstPath.size, 2);
	cmdbuf[13] = (u32) dstPath.data;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsuHandle))) return ret;

	return cmdbuf[1];
}

Result FSUSER_CreateDirectory(FS_Archive archive, FS_Path path, u32 attributes)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x809,6,2); // 0x8090182
	cmdbuf[1] = 0;
	cmdbuf[2] = (u32) archive;
	cmdbuf[3] = (u32) (archive >> 32);
	cmdbuf[4] = path.type;
	cmdbuf[5] = path.size;
	cmdbuf[6] = attributes;
	cmdbuf[7] = IPC_Desc_StaticBuffer(path.size, 0);
	cmdbuf[8] = (u32) path.data;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsuHandle))) return ret;

	return cmdbuf[1];
}

Result FSUSER_OpenArchive(FS_Archive* archive, FS_ArchiveID id, FS_Path path)
{
	if(!archive) return -2;

	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x80C,3,2); // 0x80C00C2
	cmdbuf[1] = id;
	cmdbuf[2] = path.type;
	cmdbuf[3] = path.size;
	cmdbuf[4] = IPC_Desc_StaticBuffer(path.size, 0);
	cmdbuf[5] = (u32) path.data;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsuHandle))) return ret;

	*archive = cmdbuf[2] | ((u64) cmdbuf[3] << 32);

	return cmdbuf[1];
}

Result FSUSER_ControlArchive(FS_Archive archive, FS_ArchiveAction action, void* input, u32 inputSize, void* output, u32 outputSize)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x80D,5,4); // 0x80D0144
	cmdbuf[1] = (u32) archive;
	cmdbuf[2] = (u32) (archive >> 32);
	cmdbuf[3] = action;
	cmdbuf[4] = inputSize;
	cmdbuf[5] = outputSize;
	cmdbuf[6] = IPC_Desc_Buffer(inputSize, IPC_BUFFER_R);
	cmdbuf[7] = (u32) input;
	cmdbuf[8] = IPC_Desc_Buffer(outputSize, IPC_BUFFER_W);
	cmdbuf[9] = (u32) output;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsuHandle))) return ret;

	return cmdbuf[1];
}

Result FSUSER_CloseArchive(FS_Archive archive)
{
	if(!archive) return -2;

	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x80E,2,0); // 0x80E0080
	cmdbuf[1] = (u32) archive;
	cmdbuf[2] = (u32) (archive >> 32);

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsuHandle))) return ret;

	return cmdbuf[1];
}

Result FSUSER_CreateSystemSaveData(FS_SystemSaveDataInfo info, u32 totalSize, u32 blockSize, u32 directories, u32 files, u32 directoryBuckets, u32 fileBuckets, bool duplicateData)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x856,9,0); // 0x8560240
	memcpy(&cmdbuf[1], &info, sizeof(FS_SystemSaveDataInfo));
	cmdbuf[3] = totalSize;
	cmdbuf[4] = blockSize;
	cmdbuf[5] = directories;
	cmdbuf[6] = files;
	cmdbuf[7] = directoryBuckets;
	cmdbuf[8] = fileBuckets;
	cmdbuf[9] = duplicateData;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsuHandle))) return ret;

	return cmdbuf[1];
}

Result FSUSER_DeleteSystemSaveData(FS_SystemSaveDataInfo info)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x857,2,0); // 0x8570080
	memcpy(&cmdbuf[1], &info, sizeof(FS_SystemSaveDataInfo));

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsuHandle))) return ret;

	return cmdbuf[1];
}

Result FSFILE_Read(Handle handle, u32* bytesRead, u64 offset, void* buffer, u32 size)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x802,3,2); // 0x80200C2
	cmdbuf[1] = (u32) offset;
	cmdbuf[2] = (u32) (offset >> 32);
	cmdbuf[3] = size;
	cmdbuf[4] = IPC_Desc_Buffer(size, IPC_BUFFER_W);
	cmdbuf[5] = (u32) buffer;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(handle))) return ret;

	if(bytesRead) *bytesRead = cmdbuf[2];

	return cmdbuf[1];
}

Result FSFILE_Write(Handle handle, u32* bytesWritten, u64 offset, const void* buffer, u32 size, u32 flags)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x803,4,2); // 0x8030102
	cmdbuf[1] = (u32) offset;
	cmdbuf[2] = (u32) (offset >> 32);
	cmdbuf[3] = size;
	cmdbuf[4] = flags;
	cmdbuf[5] = IPC_Desc_Buffer(size, IPC_BUFFER_R);
	cmdbuf[6] = (u32) buffer;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(handle))) return ret;

	if(bytesWritten) *bytesWritten = cmdbuf[2];

	return cmdbuf[1];
}

Result FSFILE_SetSize(Handle handle, u64 size)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x805,2,0); // 0x8050080
	cmdbuf[1] = (u32) size;
	cmdbuf[2] = (u32) (size >> 32);

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(handle))) return ret;

	return cmdbuf[1];
}

Result FSFILE_Close(Handle handle)
{
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x808,0,0); // 0x8080000

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(handle))) return ret;

	ret = cmdbuf[1];
	if(R_SUCCEEDED(ret)) ret = svcCloseHandle(handle);

	return ret;
}
