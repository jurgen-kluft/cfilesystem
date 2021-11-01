#ifndef __X_FILESYSTEM_ENUMERATIONS_H__
#define __X_FILESYSTEM_ENUMERATIONS_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

#include "xbase/x_debug.h"
#include "xbase/x_limits.h"

namespace xcore
{
	/// Typedefs
	typedef		u32			xasync_id;

	enum ESyncMode
	{
		FS_SYNC_WAIT				= 0x00,
		FS_SYNC_NOWAIT				= 0x01,
	};

	enum ESettings
	{
		FS_MAX_PATH					= 256,
		FS_MEM_ALIGNMENT			= 128,
	};

	extern void* INVALID_FILE_HANDLE;
	extern void* PENDING_FILE_HANDLE;
	extern void* INVALID_DIR_HANDLE;

	enum ESeekOrigin
	{
		Seek_Begin, 						///< Specifies the beginning of a stream.
		Seek_Current,						///< Specifies the current position within a stream.
		Seek_End,	 						///< Specifies the end of a stream.
	};
	
	enum EFileMode
	{
		FileMode_CreateNew, 				///< Specifies that the operating system should create a new file. If the file already exists it will do nothing.
		FileMode_Create,					///< Specifies that the operating system should create a new file. If the file already exists, it will be overwritten. EFileMode.Create is equivalent to requesting that if the file does not exist, use CreateNew; otherwise, use Truncate. 
		FileMode_Open, 						///< Specifies that the operating system should open an existing file. The ability to open the file is dependent on the value specified by EFileAccess. Nothing is done if the file does not exist.
		FileMode_OpenOrCreate, 				///< Specifies that the operating system should open a file if it exists; otherwise, a new file should be created.
		FileMode_Truncate, 					///< Specifies that the operating system should open an existing file. Once opened, the file should be truncated so that its size is zero bytes. 
		FileMode_Append 					///< Opens the file if it exists and seeks to the end of the file, or creates a new file. Attempting to seek to a position before the end of the file will do nothing and any attempt to read fails.
	};

	enum EFileAccess
	{
		FileAccess_Read			= 0x01, 	///< Specifies that only reading is allowed
		FileAccess_Write		= 0x02,		///< Specifies that only writing is allowed
		FileAccess_ReadWrite	= 0x03,		///< Specifies that both reading and writing are allowed
	};

	enum EFileOp
	{
		FileOp_Sync,
		FileOp_Async,
	};

	enum EError
	{
		FILE_ERROR_OK,
		FILE_ERROR_NO_FILE,			///< File does not exist
		FILE_ERROR_BADF,			///< File descriptor is invalid  
		FILE_ERROR_PRIORITY,		///< Specified priority is invalid  
		FILE_ERROR_MAX_FILES,		///< Exceeded the maximum number of files that can be handled simultaneously  
		FILE_ERROR_MAX_ASYNC,		///< Exceeded the maximum number of async operations that can be scheduled 
		FILE_ERROR_DEVICE,			///< Specified device does not exist  
		FILE_ERROR_DEVICE_READONLY,	///< Specified device does not exist  
		FILE_ERROR_UNSUP,			///< Unsupported function for this device  
		FILE_ERROR_CANNOT_MOUNT,	///< Could not be mounted  
		FILE_ERROR_ASYNC_BUSY,		///< Asynchronous operation has not completed  
		FILE_ERROR_NOASYNC,			///< No asynchronous operation has been performed  
		FILE_ERROR_NOCWD,			///< Current directory does not exist  
		FILE_ERROR_NAMETOOLONG,		///< Filename is too long  
	};

};

#endif // __X_FILESYSTEM_ENUMERATIONS_H__