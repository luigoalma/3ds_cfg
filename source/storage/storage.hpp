#pragma once
#include <3ds/fs.h>
#include <3ds/result.h>

namespace SystemSave {
	extern FS_Archive NormalFSArchive;
	extern FS_Archive FixDataFSArchive;
	extern const FS_SystemSaveDataInfo NormalFSInfo;
	extern const FS_SystemSaveDataInfo FixDataFSInfo;
	extern const FS_Path NormalFSPath;
	extern const FS_Path FixDataFSPath;

	Result Create(const FS_SystemSaveDataInfo& FSInfo);
	Result Delete(const FS_SystemSaveDataInfo& FSInfo);
	Result SimpleOpen(FS_Archive& FSArchive, const FS_Path& FSPath);
	Result Close(FS_Archive& FSArchive);
	Result Format(FS_Archive& FSArchive, const FS_SystemSaveDataInfo& FSInfo, const FS_Path& FSPath);
	Result OpenWithCreation();
	Result FixDataOpenWithCreation();
	Result Commit(FS_Archive& FSArchive);
}

namespace NandAccess {
	extern FS_Archive NandRWFSArchive;
	extern FS_Archive NandROFSArchive;

	void Open();
}
