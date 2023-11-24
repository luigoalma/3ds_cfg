#include <cstddef>
extern "C" {
	#include <3ds/fs.h>
	#include <3ds/result.h>
	#include <3ds/types.h>
}

#define FS_NOT_MOUNTED           MAKERESULT(RL_STATUS, RS_NOTFOUND, RM_FS, 101)

namespace SystemSave {
	FS_Archive NormalFSArchive = 0LLU;
	FS_Archive FixDataFSArchive = 0LLU;
	const static FS_SystemSaveDataInfo NormalFSInfo = {MEDIATYPE_NAND, 0, 0, 0x10017};
	const static FS_SystemSaveDataInfo FixDataFSInfo = {MEDIATYPE_NAND, 1, 0, 0x10017};
	const static FS_Path NormalFSPath = {PATH_BINARY, sizeof(NormalFSInfo), reinterpret_cast<const void*>(&NormalFSInfo)};
	const static FS_Path FixDataFSPath = {PATH_BINARY, sizeof(FixDataFSInfo), reinterpret_cast<const void*>(&FixDataFSInfo)};

	Result Create(FS_SystemSaveDataInfo& FSInfo) {
		// Same creation parameters for both normal syssave and fixdata syssave
		return FSUSER_CreateSystemSaveData(FSInfo, 0x40000, 0x1000, 10, 10, 11, 11, true);
	}

	Result Delete(FS_SystemSaveDataInfo& FSInfo) {
		return FSUSER_DeleteSystemSaveData(FSInfo);
	}

	Result SimpleOpen(FS_Archive& FSArchive, FS_Path& FSPath) {
		return FSUSER_OpenArchive(&FSArchive, ARCHIVE_SYSTEM_SAVEDATA, FSPath);
	}

	Result Close(FS_Archive& FSArchive) {
		if(!FSArchive) return 0;
		Result res = FSUSER_CloseArchive(FSArchive);
		FSArchive = 0LLU;
		return res;
	}

	Result Format(FS_Archive& FSArchive, FS_SystemSaveDataInfo& FSInfo, FS_Path& FSPath) {
		Close(FSArchive);
		Result res = Delete(FSInfo);
		if(R_FAILED(res) && !R_MODULEDESCRANGE(res, RM_FS, 100, 179)) return res;
		if(R_FAILED(res = Create(FSInfo))) return res;
		if(R_FAILED(res = SimpleOpen(FSArchive, FSPath))) return res;
		return 0;
	}

	Result OpenWithCreation() {
		if(NormalFSArchive) return 0;
		Result res = SimpleOpen(NormalFSArchive, NormalFSPath);
		if(R_SUCCEEDED(res)) return res;
		if(R_MODULEDESCRANGE(res, RM_FS, 100, 179) || R_MODULEDESCRANGE(res, RM_FS, 340, 359)) {
			if(R_FAILED(res = Create(NormalFSInfo))) return res;
			res = SimpleOpen(NormalFSArchive, NormalFSPath);
		} else if(R_MODULEDESCRANGE(res, RM_FS, 360, 399)) {
			res = Format(NormalFSArchive, NormalFSInfo, NormalFSPath);
		}
		return res;
	}

	// fixdata save is treated more loosely
	Result FixDataOpenWithCreation() {
		if(FixDataFSArchive) return 0;
		Result res = SimpleOpen(FixDataFSArchive, FixDataFSPath);
		if(R_SUCCEEDED(res)) return res;
		if(!R_MODULEDESCRANGE(res, RM_FS, 100, 179)) {
			Delete(FixDataFSInfo); // no check
		if(R_FAILED(res = Create(FixDataFSInfo))) return res;
		if(R_FAILED(res = SimpleOpen(FixDataFSArchive, FixDataFSPath))) {
			Delete(FixDataFSInfo); // no check
		}
		return res;
	}

	Result Commit(FS_Archive& FSArchive) {
		if(!FSArchive) return FS_NOT_MOUNTED; //FS not mounted
		char a[2];
		return FSUSER_ControlArchive(FSArchive, ARCHIVE_ACTION_COMMIT_SAVE_DATA, &a[0], 1, &a[1], 1);
	}
};

namespace NandAccess {
	FS_Archive NandRWFSArchive = 0LLU;
	FS_Archive NandROFSArchive = 0LLU;
	const static FS_Path NullPath = {PATH_EMPTY, 1, reinterpret_cast<const void*>(&NullPath.data)};

	void Open() {
		if(!NandRWFSArchive) FSUSER_OpenArchive(&NandRWFSArchive, ARCHIVE_NAND_RW, NullPath);
		if(!NandROFSArchive) FSUSER_OpenArchive(&NandROFSArchive, ARCHIVE_NAND_RO, NullPath);
	}
}
