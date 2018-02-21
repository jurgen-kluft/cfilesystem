#ifndef __XFILESYSTEM_XFILESTREAM_H__
#define __XFILESYSTEM_XFILESTREAM_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase/x_types.h"
#include "xbase/x_debug.h"

#include "xfilesystem/x_stream.h"

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

		class xfilestream : public xstream
		{
		public:
									xfilestream		();
									xfilestream		(const xfilestream&);
									xfilestream		(const xfilepath& filename, EFileMode mode, EFileAccess access, EFileOp op, x_asyncio_callback_struct = x_asyncio_callback_struct());
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
