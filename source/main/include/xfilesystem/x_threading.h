#ifndef __X_FILESYSTEM_IOTHREAD_H__
#define __X_FILESYSTEM_IOTHREAD_H__
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
		//==============================================================================
		// IO Thread and Stream synchronization
		//==============================================================================
		class xio_thread
		{
		public:
			virtual				~xio_thread() {}

			///< IO Thread
			virtual void		sleep(u32 ms) = 0;
			virtual bool		loop() const = 0;
			virtual void		wait() = 0;
			virtual void		signal() = 0;
		};

	};
	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

//==============================================================================
// END __X_FILESYSTEM_IOTHREAD_H__
//==============================================================================
#endif