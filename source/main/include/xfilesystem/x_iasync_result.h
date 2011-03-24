#ifndef __XFILESYSTEM_IASYNC_RESULT_H__
#define __XFILESYSTEM_IASYNC_RESULT_H__
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
		///< Async result implementation interface
		class xiasync_result
		{
		public:
			virtual					~xiasync_result()				{ }

			virtual bool			isCompleted() = 0;
			virtual void			waitUntilCompleted() = 0;

			virtual void			clear() = 0;
			virtual void			hold() = 0;
			virtual s32				release() = 0;
			virtual void			destroy() = 0;
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
// END __XFILESYSTEM_IASYNC_RESULT_H__
//==============================================================================
#endif
