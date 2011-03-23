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
		class xthreading
		{
		public:
			virtual				~xthreading() {}

			///< IO Thread
			virtual bool		loop() const = 0;
			virtual void		wait() = 0;
			virtual void		signal() = 0;

			///< Stream synchronization
			virtual void		wait(u32 streamIndex) = 0;
			virtual void		signal(u32 streamIndex) = 0;
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