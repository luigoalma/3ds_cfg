/**
 * @file fs.h
 * @brief Filesystem Services
 */
#pragma once

#include <3ds/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Open flags.
enum
{
    FS_OPEN_READ   = BIT(0), ///< Open for reading.
    FS_OPEN_WRITE  = BIT(1), ///< Open for writing.
    FS_OPEN_CREATE = BIT(2), ///< Create file.
};

/// Write flags.
enum
{
    FS_WRITE_FLUSH       = BIT(0), ///< Flush.
    FS_WRITE_UPDATE_TIME = BIT(8), ///< Update file timestamp.
};

/// Media types.
typedef enum
{
	MEDIATYPE_NAND      = 0, ///< NAND.
	MEDIATYPE_SD        = 1, ///< SD card.
	MEDIATYPE_GAME_CARD = 2, ///< Game card.
} FS_MediaType;

/// Archive IDs.
typedef enum
{
	ARCHIVE_ROMFS                    = 0x00000003, ///< RomFS archive.
	ARCHIVE_SAVEDATA                 = 0x00000004, ///< Save data archive.
	ARCHIVE_EXTDATA                  = 0x00000006, ///< Ext data archive.
	ARCHIVE_SHARED_EXTDATA           = 0x00000007, ///< Shared ext data archive.
	ARCHIVE_SYSTEM_SAVEDATA          = 0x00000008, ///< System save data archive.
	ARCHIVE_SDMC                     = 0x00000009, ///< SDMC archive.
	ARCHIVE_SDMC_WRITE_ONLY          = 0x0000000A, ///< Write-only SDMC archive.
	ARCHIVE_BOSS_EXTDATA             = 0x12345678, ///< BOSS ext data archive.
	ARCHIVE_CARD_SPIFS               = 0x12345679, ///< Card SPI FS archive.
	ARCHIVE_EXTDATA_AND_BOSS_EXTDATA = 0x1234567B, ///< Ext data and BOSS ext data archive.
	ARCHIVE_SYSTEM_SAVEDATA2         = 0x1234567C, ///< System save data archive.
	ARCHIVE_NAND_RW                  = 0x1234567D, ///< Read-write NAND archive.
	ARCHIVE_NAND_RO                  = 0x1234567E, ///< Read-only NAND archive.
	ARCHIVE_NAND_RO_WRITE_ACCESS     = 0x1234567F, ///< Read-only write access NAND archive.
	ARCHIVE_SAVEDATA_AND_CONTENT     = 0x2345678A, ///< User save data and ExeFS/RomFS archive.
	ARCHIVE_SAVEDATA_AND_CONTENT2    = 0x2345678E, ///< User save data and ExeFS/RomFS archive (only ExeFS for fs:LDR).
	ARCHIVE_NAND_CTR_FS              = 0x567890AB, ///< NAND CTR FS archive.
	ARCHIVE_TWL_PHOTO                = 0x567890AC, ///< TWL PHOTO archive.
	ARCHIVE_TWL_SOUND                = 0x567890AD, ///< TWL SOUND archive.
	ARCHIVE_NAND_TWL_FS              = 0x567890AE, ///< NAND TWL FS archive.
	ARCHIVE_NAND_W_FS                = 0x567890AF, ///< NAND W FS archive.
	ARCHIVE_GAMECARD_SAVEDATA        = 0x567890B1, ///< Game card save data archive.
	ARCHIVE_USER_SAVEDATA            = 0x567890B2, ///< User save data archive.
	ARCHIVE_DEMO_SAVEDATA            = 0x567890B4, ///< Demo save data archive.
} FS_ArchiveID;

/// Path types.
typedef enum
{
	PATH_INVALID = 0, ///< Invalid path.
	PATH_EMPTY   = 1, ///< Empty path.
	PATH_BINARY  = 2, ///< Binary path. Meaning is per-archive.
	PATH_ASCII   = 3, ///< ASCII text path.
	PATH_UTF16   = 4, ///< UTF-16 text path.
} FS_PathType;

/// Archive control actions.
typedef enum
{
	ARCHIVE_ACTION_COMMIT_SAVE_DATA = 0, ///< Commits save data changes. No inputs/outputs.
	ARCHIVE_ACTION_GET_TIMESTAMP    = 1, ///< Retrieves a file's last-modified timestamp. In: "u16*, UTF-16 Path", Out: "u64, Time Stamp".
	ARCHIVE_ACTION_UNKNOWN          = 0x789D, //< Unknown action; calls FSPXI command 0x56. In: "FS_Path instance", Out: "u32[4], Unknown"
} FS_ArchiveAction;

/// System save data information.
typedef struct
{
	FS_MediaType mediaType : 8; ///< Media type.
	u8 unknown;                 ///< Unknown.
	u16 reserved;               ///< Reserved.
	u32 saveId;                 ///< Save ID.
} FS_SystemSaveDataInfo;

/// Filesystem path data, detailing the specific target of an operation.
typedef struct
{
	FS_PathType type; ///< FS path type.
	u32 size;         ///< FS path size.
	const void* data; ///< Pointer to FS path data.
} FS_Path;

/// Filesystem archive handle, providing access to a filesystem's contents.
typedef u64 FS_Archive;

/// Initializes FS.
Result fsInit(void);

/// Exits FS.
void fsExit(void);

/**
 * @brief Initializes a FSUSER session.
 * @param session The handle of the FSUSER session to initialize.
 */
Result FSUSER_Initialize(Handle session);

/**
 * @brief Opens a file.
 * @param out Pointer to output the file handle to.
 * @param archive Archive containing the file.
 * @param path Path of the file.
 * @param openFlags Flags to open the file with.
 * @param attributes Attributes of the file.
 */
Result FSUSER_OpenFile(Handle* out, FS_Archive archive, FS_Path path, u32 openFlags, u32 attributes);

/**
 * @brief Opens a file directly, bypassing the requirement of an opened archive handle.
 * @param out Pointer to output the file handle to.
 * @param archiveId ID of the archive containing the file.
 * @param archivePath Path of the archive containing the file.
 * @param filePath Path of the file.
 * @param openFlags Flags to open the file with.
 * @param attributes Attributes of the file.
 */
Result FSUSER_OpenFileDirectly(Handle* out, FS_ArchiveID archiveId, FS_Path archivePath, FS_Path filePath, u32 openFlags, u32 attributes);

/**
 * @brief Deletes a file.
 * @param archive Archive containing the file.
 * @param path Path of the file.
 */
Result FSUSER_DeleteFile(FS_Archive archive, FS_Path path);

/**
 * @brief Renames a file.
 * @param srcArchive Archive containing the source file.
 * @param srcPath Path of the source file.
 * @param dstArchive Archive containing the destination file.
 * @param dstPath Path of the destination file.
 */
Result FSUSER_RenameFile(FS_Archive srcArchive, FS_Path srcPath, FS_Archive dstArchive, FS_Path dstPath);

/**
 * @brief Creates a directory
 * @param archive Archive to create the directory in.
 * @param path Path of the directory.
 * @param attributes Attributes of the directory.
 */
Result FSUSER_CreateDirectory(FS_Archive archive, FS_Path path, u32 attributes);

/**
 * @brief Opens an archive.
 * @param archive Pointer to output the opened archive to.
 * @param id ID of the archive.
 * @param path Path of the archive.
 */
Result FSUSER_OpenArchive(FS_Archive* archive, FS_ArchiveID id, FS_Path path);

/**
 * @brief Performs a control operation on an archive.
 * @param archive Archive to control.
 * @param action Action to perform.
 * @param input Buffer to read input from.
 * @param inputSize Size of the input.
 * @param output Buffer to write output to.
 * @param outputSize Size of the output.
 */
Result FSUSER_ControlArchive(FS_Archive archive, FS_ArchiveAction action, void* input, u32 inputSize, void* output, u32 outputSize);

/**
 * @brief Closes an archive.
 * @param archive Archive to close.
 */
Result FSUSER_CloseArchive(FS_Archive archive);

/**
 * @brief Creates system save data.
 * @param info Info of the save data.
 * @param totalSize Total size of the save data.
 * @param blockSize Block size of the save data. (usually 0x1000)
 * @param directories Number of directories.
 * @param files Number of files.
 * @param directoryBuckets Directory hash tree bucket count.
 * @param fileBuckets File hash tree bucket count.
 * @param duplicateData Whether to store an internal duplicate of the data.
 */
Result FSUSER_CreateSystemSaveData(FS_SystemSaveDataInfo info, u32 totalSize, u32 blockSize, u32 directories, u32 files, u32 directoryBuckets, u32 fileBuckets, bool duplicateData);

/**
 * @brief Deletes system save data.
 * @param info Info of the save data.
 */
Result FSUSER_DeleteSystemSaveData(FS_SystemSaveDataInfo info);

/**
 * @brief Reads from a file.
 * @param handle Handle of the file.
 * @param bytesRead Pointer to output the number of bytes read to.
 * @param offset Offset to read from.
 * @param buffer Buffer to read to.
 * @param size Size of the buffer.
 */
Result FSFILE_Read(Handle handle, u32* bytesRead, u64 offset, void* buffer, u32 size);

/**
 * @brief Writes to a file.
 * @param handle Handle of the file.
 * @param bytesWritten Pointer to output the number of bytes written to.
 * @param offset Offset to write to.
 * @param buffer Buffer to write from.
 * @param size Size of the buffer.
 * @param flags Flags to use when writing.
 */
Result FSFILE_Write(Handle handle, u32* bytesWritten, u64 offset, const void* buffer, u32 size, u32 flags);

/**
 * @brief Sets the size of a file.
 * @param handle Handle of the file.
 * @param size Size to set.
 */
Result FSFILE_SetSize(Handle handle, u64 size);

/**
 * @brief Closes a file.
 * @param handle Handle of the file.
 */
Result FSFILE_Close(Handle handle);

#ifdef __cplusplus
}
#endif
