#pragma once
#include <cstddef>
extern "C" {
	#include <3ds/fs.h>
	#include <3ds/os.h>
	#include <3ds/ps.h>
	#include <3ds/result.h>
	#include <3ds/svc.h>
	#include <3ds/types.h>
}
#include <utils/sha256.h>

// override is a special feature to keep the SecureInfo_C consistency from Luma
template<typename T, const FS_Path* Paths, const FS_Path* InvalidPaths, int PathCount, const FS_Path* OverridePath, const FS_Path* OverridePathTemp, const PS_RSA_Context RsaPair[2]>
struct ManagedSignedFile_T {
	T Data;
	bool IsInit:1;
	bool IsSigned:1;
	bool WasChecked:1;
	bool Overriden:1;
	int Index;

	bool HasDefinedOverride();
	Result CheckSignature();
	bool OpenDeleteFail(Handle& file, FS_Path& rwNandPath);
	void Init();
	void SaveToNextSlot();
};

template<typename T, const FS_Path* Paths, const FS_Path* InvalidPaths, int PathCount, const FS_Path* OverridePath, const FS_Path* OverridePathTemp, const PS_RSA_Context RsaPair[2]>
inline bool ManagedSignedFile_T<T,Paths,InvalidPaths,PathCount,OverridePath,OverridePathTemp,RsaPair>::HasDefinedOverride() {
	return OverridePath != nullptr && OverridePathTemp != nullptr;
}

template<typename T, const FS_Path* Paths, const FS_Path* InvalidPaths, int PathCount, const FS_Path* OverridePath, const FS_Path* OverridePathTemp, const PS_RSA_Context RsaPair[2]>
inline Result ManagedSignedFile_T<T,Paths,InvalidPaths,PathCount,OverridePath,OverridePathTemp,RsaPair>::CheckSignature() {
	if(!IsInit)
		return CFG_NOT_INITIALIZED;

	ALIGN(4) u8 hash[SHA256_HASH_LENGTH];
	sha256_full(Data.SignedData, sizeof(Data.SignedData), hash);

	PS_RSA_Context* ctx = &RsaPair[osEnvInfoIsDev() ? 1 : 0];
	Result res = psInit();
	if(R_SUCCEEDED(res)) {
		res = PS_VerifyRsaSha256(hash, ctx, Data.Signature);
		psExit();
	}

	IsSigned = R_SUCCEEDED(res);

	return R_FAILED(res) ? CFG_FAILED_SIGNATURE_CHECK : 0;
}

template<typename T, const FS_Path* Paths, const FS_Path* InvalidPaths, int PathCount, const FS_Path* OverridePath, const FS_Path* OverridePathTemp, const PS_RSA_Context RsaPair[2]>
inline bool ManagedSignedFile_T<T,Paths,InvalidPaths,PathCount,OverridePath,OverridePathTemp,RsaPair>::OpenDeleteFail(Handle& file, FS_Path& rwNandPath) {
	file = 0;

	Result res = FSUSER_OpenFile(&file, ::NandAccess::NandRWFSArchive, rwNandPath, FS_OPEN_READ, 0);

	if(R_FAILED(res))
		FSUSER_DeleteFile(::NandAccess::NandRWFSArchive, rwNandPath);

	return R_SUCCEEDED(res);
}

template<typename T, const FS_Path* Paths, const FS_Path* InvalidPaths, int PathCount, const FS_Path* OverridePath, const FS_Path* OverridePathTemp, const PS_RSA_Context RsaPair[2]>
inline void ManagedSignedFile_T<T,Paths,InvalidPaths,PathCount,OverridePath,OverridePathTemp,RsaPair>::Init() {
	Index = -1;
	IsSigned = false;
	WasChecked = false;
	Overriden = false;

	for(int i = 0; i < PathCount; ++i)
		FSUSER_DeleteFile(::NandAccess::NandRWFSArchive, InvalidPaths[i]);

	Handle file = 0;
	u32 filesize = 0;

	if(HasDefinedOverride()) {
		Result res = FSUSER_OpenFile(&file, ::NandAccess::NandRWFSArchive, OverridePath, FS_OPEN_READ, 0);
		if(R_SUCCEEDED(res)) res = FSFILE_Read(file, &filesize, 0LLU, &Data.Raw[0], sizeof(T));

		FSFILE_Close(file);
		svcCloseHandle(file);

		if(R_SUCCEEDED(res) && filesize == sizeof(T)) {
			IsInit = true;
			Overriden = true;
			return;
		}

		file = 0;
		filesize = 0;
	}

	for(int i = 0; i < PathCount; ++i) {
		if(OpenDeleteFail(file, Paths[i])) {
			Index = i;
		}
	}

	if(Index >= 0) {
		Result res = FSFILE_Read(file, &filesize, 0LLU, &Data.Raw[0], sizeof(T));
		if(R_FAILED(res) || filesize != sizeof(T)) {
			// checks that cfg didnt do
			memset(&Data.Raw[0], 0, sizeof(T));
		}
	} else {
		Index = 0;
		memset(&Data.Raw[0], 0, sizeof(T));

		static const FS_Path SysPath = {PATH_ASCII, 5, "/sys"};
		FSUSER_CreateDirectory(::NandAccess::NandRWFSArchive, SysPath, 0);

		file = 0;

		Result res = FSUSER_OpenFile(&file, ::NandAccess::NandRWFSArchive, InvalidPaths[0], FS_OPEN_WRITE | FS_OPEN_CREATE, 0);
		if(R_SUCCEEDED(res)) res = FSFILE_SetSize(file, sizeof(T));
		if(R_SUCCEEDED(res)) res = FSFILE_Write(file, &filesize, 0LLU, &Data.Raw[0], sizeof(T), FS_WRITE_FLUSH);
		if(R_SUCCEEDED(res) && filesize == sizeof(T)) FSUSER_RenameFile(::NandAccess::NandRWFSArchive, InvalidPaths[0], ::NandAccess::NandRWFSArchive, Paths[0]);
		else FSUSER_DeleteFile(::NandAccess::NandRWFSArchive, InvalidPaths[0]);
	}

	FSFILE_Close(file);
	svcCloseHandle(file);

	IsInit = true;
}

template<typename T, const FS_Path* Paths, const FS_Path* InvalidPaths, int PathCount, const FS_Path* OverridePath, const FS_Path* OverridePathTemp, const PS_RSA_Context RsaPair[2]>
inline void ManagedSignedFile_T<T,Paths,InvalidPaths,PathCount,OverridePath,OverridePathTemp,RsaPair>::SaveToNextSlot() {
	const FS_Path* target = nullptr;
	Handle file = 0;

	int last_index = Index;
	Index = (++Index >= PathCount) ? 0 : Index;

	if(Overriden) {
		target = OverridePathTemp;
	} else {
		target = &InvalidPaths[Index];
		if(OpenDeleteFail(file, Paths[Index])) {
			FSFILE_Close(file);
			svcCloseHandle(file);
			file = 0;
			FSUSER_DeleteFile(::NandAccess::NandRWFSArchive, InvalidPaths[Index]); // config didnt ensure path was clear to use, and not sure if FS errors on rename but
			FSUSER_RenameFile(::NandAccess::NandRWFSArchive, Paths[Index], ::NandAccess::NandRWFSArchive, InvalidPaths[Index]);
		}
	}

	Result res = FSUSER_OpenFile(&file, ::NandAccess::NandRWFSArchive, *target, FS_OPEN_WRITE | FS_OPEN_CREATE, 0);
	if(R_SUCCEEDED(res)) res = FSFILE_SetSize(file, sizeof(T));
	if(R_SUCCEEDED(res)) res = FSFILE_Write(file, &filesize, 0LLU, &Data.Raw[0], sizeof(T), FS_WRITE_FLUSH);

	FSFILE_Close(file);
	svcCloseHandle(file);

	if(!Overriden) {
		if(R_SUCCEEDED(res) && filesize == sizeof(T)) {
			FSUSER_RenameFile(::NandAccess::NandRWFSArchive, InvalidPaths[Index], ::NandAccess::NandRWFSArchive, Paths[Index]);
			FSUSER_RenameFile(::NandAccess::NandRWFSArchive, Paths[last_index], ::NandAccess::NandRWFSArchive, InvalidPaths[last_index]);
		} else {
			FSUSER_DeleteFile(::NandAccess::NandRWFSArchive, InvalidPaths[Index]);
			Index = last_index; // cfg doesn't check if error and roleback
		}
	} else {
		if(R_SUCCEEDED(res) && filesize == sizeof(T)) {
			FSUSER_DeleteFile(::NandAccess::NandRWFSArchive, OverridePath);
			FSUSER_RenameFile(::NandAccess::NandRWFSArchive, OverridePathTemp, ::NandAccess::NandRWFSArchive, OverridePath);
		} else {
			FSUSER_DeleteFile(::NandAccess::NandRWFSArchive, OverridePathTemp);
		}
	}
}
