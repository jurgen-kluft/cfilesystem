#ifndef __X_FILESYSTEM_CONSTANTS_H__
#define __X_FILESYSTEM_CONSTANTS_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase/x_types.h"
#include "xbase/x_debug.h"
#include "xbase/x_limits.h"

//==============================================================================
// xcore namespace
//==============================================================================
namespace xcore
{
	namespace xfilesystem
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

		enum EFileHandle
		{
			PENDING_FILE_HANDLE			= -2,
			INVALID_FILE_HANDLE			= -1,
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


		#define XFILESYSTEM_OBJECT_NEW_DELETE()																					\
			void*	operator new(xcore::xsize_t num_bytes)				{ return heapAlloc(num_bytes, X_ALIGNMENT_DEFAULT); }	\
			void*	operator new(xcore::xsize_t num_bytes, void* mem)	{ return mem; }											\
			void	operator delete(void* mem)							{ heapFree(mem); }										\
			void	operator delete(void* mem, void* )					{ }						

	};

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

//==============================================================================
// END __X_FILESYSTEM_PRIVATE_H__
//==============================================================================
#endif