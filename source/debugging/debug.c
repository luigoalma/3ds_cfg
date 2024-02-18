#include <string.h>
#include <3ds/types.h>
#include <3ds/fs.h>
#include <3ds/ipc.h>
#include <err.h>
#include <debugging/debug.h>
#include <utils/base64.h>

typedef enum {
	IPC_TYPE_UNKNOWN,
	IPC_TYPE_SHARED_HANDLES,
	IPC_TYPE_MOVED_HANDLES,
	IPC_TYPE_CURR_PROCESS_HANDLES,
	IPC_TYPE_STATIC_DESC_BUFFER,
	IPC_TYPE_PXI_RO_BUFFER,
	IPC_TYPE_PXI_RW_BUFFER,
	//IPC_TYPE_BUFFER,
	IPC_TYPE_RO_BUFFER,
	IPC_TYPE_WO_BUFFER,
	IPC_TYPE_RW_BUFFER
} IPC_TRANSLATION_TYPE;

static struct {
	Handle hndl;
	u64 off;
} DebugFile = {0, 0};

static const FS_Path sdmcPath  = {PATH_EMPTY, 1, ""};
static const FS_Path debugPath = {PATH_ASCII, 14, "/CFGDEBUG.LOG"};

static void byte_to_hex(char* out, u8 byte) {
	static const char hex_table[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
	};

	*(out++) = hex_table[byte >> 4];
	*(out++) = hex_table[byte & 0xF];
}

static void binary_to_hex(char* out, const void* data, size_t size, bool invert_endian) {
	const u8* _data = (const u8*)data + (invert_endian ? size : 0);
	const u8* end = _data - (invert_endian ? size : 0);

	if(invert_endian) {
		_data = (const u8*)data + size;
		end = (const u8*)data;
	} else {
		_data = (const u8*)data;
		end = _data + size;
	}

	static const char hex_table[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
	};

	while((uptr)_data != (uptr)end) {
		u8 byte = invert_endian ? *--_data : *(_data++);
		*(out++) = hex_table[byte >> 4];
		*(out++) = hex_table[byte & 0xF];
	}
}

static IPC_TRANSLATION_TYPE get_translation_param_type(u32 param) {
	u8 translation_nibble = param & 0xF;

	if(translation_nibble == 0) {
		// handles
		u32 type = (param << 6) >> 10;

		if(type == 0)
			return IPC_TYPE_SHARED_HANDLES;

		if(type == 0x1)
			return IPC_TYPE_MOVED_HANDLES;

		if(type == 0x2)
			return IPC_TYPE_CURR_PROCESS_HANDLES;

	} else if(translation_nibble == 0x2) {
		// if(((param >> 4) & 0x3F) == 0) // should we check? seems to be ignored
		return IPC_TYPE_STATIC_DESC_BUFFER;

	} else if((translation_nibble & 0xD) == 0x4) {
		return (translation_nibble & 0x2) ? IPC_TYPE_PXI_RO_BUFFER : IPC_TYPE_PXI_RW_BUFFER;

	} else if((translation_nibble & 0x9) == 0x8) {
		u8 permission = translation_nibble & 0x6;

		if(permission == IPC_BUFFER_R)
			return IPC_TYPE_RO_BUFFER;

		if(permission == IPC_BUFFER_W)
			return IPC_TYPE_WO_BUFFER;

		if(permission == IPC_BUFFER_RW)
			return IPC_TYPE_RW_BUFFER;
	}

	return IPC_TYPE_UNKNOWN;
}

void DebugOut_Init() {
	Err_Panic(FSUSER_OpenFileDirectly(&DebugFile.hndl, ARCHIVE_SDMC, sdmcPath, debugPath, FS_OPEN_WRITE | FS_OPEN_CREATE, 0));
	Err_Panic(FSFILE_SetSize(DebugFile.hndl, 0));
	DebugFile.off = 0;
}

void DebugOut_Exit() {
	FSFILE_Close(DebugFile.hndl);
	svcCloseHandle(DebugFile.hndl);
	DebugFile.hndl = 0;
	DebugFile.off = 0;
}

static void DebugOut_WriteImpl(const void* buf, size_t size) {
	u32 written;
	Err_Panic(FSFILE_Write(DebugFile.hndl, &written, DebugFile.off, buf, size, FS_WRITE_FLUSH));
	if(size != written)
		svcBreak(USERBREAK_PANIC);
	DebugFile.off += written;
}

void DebugOut_Write(const void* buf, size_t size) {
	u32* cmdbuf = getThreadCommandBuffer();
	u32 cmdbufbak[64];

	memcpy(cmdbufbak, cmdbuf, sizeof(cmdbufbak));
	DebugOut_WriteImpl(buf, size);
	memcpy(cmdbuf, cmdbufbak, sizeof(cmdbufbak));
}

static void dump_buffer(const void* buf, size_t size) {
	const uint8_t* _buf = (const uint8_t*)buf;

	ALIGN(4) char buffer[65];
	size_t len = 0;

	while(size) {
		size_t _size = (size > 48) ? 48 : size;
		base64_encode(buffer, &len, _buf, _size);
		buffer[len] = '\n';
		DebugOut_WriteImpl(buffer, len+1);
		size -= _size;
		_buf += _size;
	}
}

void DebugOut_WriteCmdBuf() {
	// We read cmdbuf and write to file
	// as effect, we mustn't forget to backup buffer!
	// for cfg, static buffer is not used for received or replied commands
	u32* cmdbuf = getThreadCommandBuffer();
	u32 cmdbufbak[64];

	memcpy(cmdbufbak, cmdbuf, sizeof(cmdbufbak));
	u16 command_id = cmdbuf[0] >> 16;
	s32 normal_params = (cmdbuf[0] >> 6) & 0x3F;
	s32 translate_params = cmdbuf[0] & 0x3F;

	DebugOut_WriteImpl("== Cmd Output ==\n", 17);

	// [0] doesn't count on parameter counts
	if(normal_params + translate_params >= 64) { 
		DebugOut_WriteImpl("Odd cmd buffer detected!!\n", 26);
		normal_params = 64;
		translate_params = 0;
	} else {
		normal_params += 1;
	}

	ALIGN(8) char buffer[48];
	*(u64*)&buffer[0] = 0x203D206469646D63LLU;
	*(u64*)&buffer[8] = 0x000A585858587830LLU;
	binary_to_hex(&buffer[10], &command_id, 2, true);
	DebugOut_WriteImpl(buffer, 15);

	*(u64*)&buffer[0] = 0x305B667562646D63LLU;
	*(u64*)&buffer[8] = 0x7830203D5D585878LLU;
	buffer[24] = '\n';
	for(int i = 0; i < (normal_params + translate_params); ++i) {
		byte_to_hex(&buffer[9], i);
		binary_to_hex(&buffer[16], &cmdbufbak[i], 4, true);
		DebugOut_WriteImpl(buffer, 25);
	}

	u32* translationbuf = &cmdbufbak[normal_params];

	for(int i = 0; translate_params > 0;) {
		u32 param = translationbuf[i];
		IPC_TRANSLATION_TYPE ipctype = get_translation_param_type(param);

		switch(ipctype) {
		case IPC_TYPE_SHARED_HANDLES:
		case IPC_TYPE_MOVED_HANDLES:
		case IPC_TYPE_CURR_PROCESS_HANDLES:
		{
			u8 handle_count = (param >> 26) + 1;
			*(u64*)&buffer[0] = 0x6E61482058587830LLU;
			*(u32*)&buffer[8] = 0x73656C64;
			buffer[12] = '\n';
			byte_to_hex(&buffer[2], handle_count);
			DebugOut_WriteImpl(buffer, 13);
			translate_params -= handle_count + 1; // parameter + handle count
			i += handle_count + 1; // parameter + handle count
			break;
		}
		case IPC_TYPE_STATIC_DESC_BUFFER:
		{
			size_t size = IPC_Get_Desc_StaticBuffer_Size(param);
			*(u64*)&buffer[0]  = 0x7542636974617453LLU;
			*(u64*)&buffer[8]  = 0x203A657A69532066LLU;
			*(u16*)&buffer[16] = 0x7830;
			*(u64*)&buffer[24] = 0x622820617461440ALLU;
			*(u64*)&buffer[32] = 0x0A3A293436657361LLU;
			binary_to_hex(&buffer[18], &size, 3, true);
			DebugOut_WriteImpl(buffer, 40);
			dump_buffer((void*)translationbuf[i+1], size);
			translate_params -= 2;
			i += 2;
			break;
		}
		case IPC_TYPE_RO_BUFFER:
		case IPC_TYPE_WO_BUFFER:
		case IPC_TYPE_RW_BUFFER:
		{
			size_t size = IPC_Get_Desc_Buffer_Size(param);
			*(u64*)&buffer[0]  = 0x2066754220435049LLU;
			*(u64*)&buffer[8]  = 0x7830203A657A6953LLU;
			*(u64*)&buffer[24] = 0x622820617461440ALLU;
			*(u64*)&buffer[32] = 0x0A3A293436657361LLU;
			binary_to_hex(&buffer[16], &size, 4, true);
			DebugOut_WriteImpl(buffer, 40);
			dump_buffer((void*)translationbuf[i+1], size);
			translate_params -= 2;
			i += 2;
			break;
		}
		case IPC_TYPE_PXI_RO_BUFFER:
		case IPC_TYPE_PXI_RW_BUFFER:
			// not gonna deal with PXI rn
		case IPC_TYPE_UNKNOWN:
		default:
			DebugOut_WriteImpl("Unknown IPC type!\n Stopping translation dump.\n", 46);
			translate_params = 0;
			break;
		}
	}
	
	memcpy(cmdbuf, cmdbufbak, sizeof(cmdbufbak));
}
