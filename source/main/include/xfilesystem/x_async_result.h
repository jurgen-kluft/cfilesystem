#ifndef __XFILESYSTEM_ASYNC_RESULT_H__
#define __XFILESYSTEM_ASYNC_RESULT_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase\x_types.h"
#include "xbase\x_debug.h"

//==============================================================================
// xcore namespace
//==============================================================================
namespace xcore
{
	namespace xfilesystem
	{
		
		// new simplified async IO result structure

		struct x_asyncio_result
		{
			u64		result;				// number of bytes written or read (or 0 for failure)
			xbyte*	buffer;				// pointer to buffer used for read/writing
			u32		fileHandle;			// the file handle this operation was commited on
			u32		operation;			// status enum: which operation was performed? (see EFileOpStatus from fileasync)
			void*			userData;			// user defined data -- set in x_asyncio_callback_struct*
		};

		//< Callback prototype
		typedef void (*x_asyncio_callback_func)(x_asyncio_result);
		
		// callback structure.  Set callback to the function you wish to use, and userData to any data you may need to use later in the callback.
		// This structure is passed in to xfilestream or beginRead/Write as a pointer, and it must be valid as long as the io thread is running.
		struct x_asyncio_callback_struct
		{
			x_asyncio_callback_func callback;
			void*					userData;
			
			x_asyncio_callback_struct(x_asyncio_callback_func inCallback = NULL, void* inUserData = NULL)
			{
				callback = inCallback;
				userData = inUserData;
			}

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
// END __XFILESYSTEM_ASYNC_RESULT_H__
//==============================================================================
#endif
