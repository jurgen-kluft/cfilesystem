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
			xcore::u64		result;				// number of bytes written or read (or 0 for failure)
			xcore::xbyte*	buffer;				// pointer to buffer used for read/writing
			xcore::u32		fileHandle;			// the file handle this operation was commited on
			xcore::u32		operation;			// status enum: which operation was performed? (see EFileOpStatus from fileasync)
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
			
		};



		// ------------------------------------------------
		// TODO: REMOVE  BELOW OLD CLASS 
		
		///< Forward declares
		class xiasync_result;

		///< Async result (Reference counted)
		///< User can hold on to this object and use it to check for an async
		///< file operations to complete. 

		class xasync_result
		{
		public:
									xasync_result();
									xasync_result(const xasync_result&);
									~xasync_result();

			bool					checkForCompletion();
			void					waitForCompletion();

			u64						getResult() const;

			xasync_result&			operator =  (const xasync_result&);
			bool					operator == (const xasync_result&) const;
			bool					operator != (const xasync_result&) const;

		protected:
									xasync_result(xiasync_result* imp);

			xiasync_result*			mImplementation;
			u64						mResult;
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
