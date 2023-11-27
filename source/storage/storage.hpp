#pragma once
#include <3ds/fs.h>
#include <3ds/result.h>

namespace SystemSave {
	extern FS_Archive NormalFSArchive;
	extern FS_Archive FixDataFSArchive;

	Result Create(FS_SystemSaveDataInfo& FSInfo);
	Result Delete(FS_SystemSaveDataInfo& FSInfo);
	Result SimpleOpen(FS_Archive& FSArchive, FS_Path& FSPath);
	Result Close(FS_Archive& FSArchive);
	Result Format(FS_Archive& FSArchive, FS_SystemSaveDataInfo& FSInfo, FS_Path& FSPath);
	Result OpenWithCreation();
	Result FixDataOpenWithCreation();
	Result Commit(FS_Archive& FSArchive);
}

namespace NandAccess {
	extern FS_Archive NandRWFSArchive;
	extern FS_Archive NandROFSArchive;

	void Open();
}
