#ifndef __XFILESYSTEM_XFILESTREAM_H__
#define __XFILESYSTEM_XFILESTREAM_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase\x_types.h"
#include "xbase\x_debug.h"

#include "xfilesystem\x_stream.h"

//==============================================================================
// xcore namespace
//==============================================================================
namespace xcore
{
	namespace xfilesystem
	{
		// Forward declares
		class xfilepath;

		enum EFileMode
		{
			FileMode_CreateNew, 				///< Specifies that the operating system should create a new file. This requires FileIOPermissionAccess.write. If the file already exists, an IOException is thrown.
			FileMode_Create,					///< Specifies that the operating system should create a new file. If the file already exists, it will be overwritten. This requires FileIOPermissionAccess.write. System.IO.FileMode.Create is equivalent to requesting that if the file does not exist, use CreateNew; otherwise, use Truncate. If the file already exists but is a hidden file, an UnauthorizedAccessException is thrown.
			FileMode_Open, 						///< Specifies that the operating system should open an existing file. The ability to open the file is dependent on the value specified by FileAccess. A System.IO.FileNotFoundException is thrown if the file does not exist.
			FileMode_OpenOrCreate, 				///< Specifies that the operating system should open a file if it exists; otherwise, a new file should be created. If the file is opened with FileAccess.Read, FileIOPermissionAccess.Read is required. If the file access is FileAccess.write then FileIOPermissionAccess.write is required. If the file is opened with FileAccess.ReadWrite, both FileIOPermissionAccess.Read and FileIOPermissionAccess.write are required. If the file access is FileAccess.Append, then FileIOPermissionAccess.Append is required.
			FileMode_Truncate, 					///< Specifies that the operating system should open an existing file. Once opened, the file should be truncated so that its size is zero bytes. This requires FileIOPermissionAccess.write. Attempts to read from a file opened with Truncate cause an exception.
			FileMode_Append 					///< Opens the file if it exists and seeks to the end of the file, or creates a new file. FileMode.Append can only be used in conjunction with FileAccess.write. Attempting to seek to a position before the end of the file will throw an IOException and any attempt to read fails and throws an NotSupportedException.
		};

		enum EFileAccess
		{
			FileAccess_Read			= 0x01, 	///< Specifies that the operating system should create a new file. This requires FileIOPermissionAccess.write. If the file already exists, an IOException is thrown.
			FileAccess_Write		= 0x02,		///< Specifies that the operating system should create a new file. If the file already exists, it will be overwritten. This requires FileIOPermissionAccess.write. System.IO.FileMode.Create is equivalent to requesting that if the file does not exist, use CreateNew; otherwise, use Truncate. If the file already exists but is a hidden file, an UnauthorizedAccessException is thrown.
			FileAccess_ReadWrite	= 0x03,		///< Specifies that the operating system should open an existing file. The ability to open the file is dependent on the value specified by FileAccess. A System.IO.FileNotFoundException is thrown if the file does not exist.
		};

		enum EFileOp
		{
			FileOp_Sync,
			FileOp_Async,
		};

		class xfilestream : public xstream
		{
		public:
									xfilestream		(const xfilestream&);
									xfilestream		(const xfilepath& filename, EFileMode mode, EFileAccess access, EFileOp op);
									~xfilestream	(void);

			xfilestream&			operator =		(const xfilestream&);
			bool					operator ==		(const xfilestream&) const;
			bool					operator !=		(const xfilestream&) const;
		};

		//==============================================================================
		// END xfilesystem namespace
		//==============================================================================
	};

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

//==============================================================================
// END __XFILESYSTEM_XFILESTREAM_H__
//==============================================================================
#endif
