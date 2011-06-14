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


		///< The user needs to implement this object and integrate a thread synchronization
		///< object capable of putting the calling thread to sleep and waking it up when
		///< signal is called. 
		class xevent
		{
		public:
			virtual				~xevent()						{ }

			virtual void		set() = 0;						///< Set the event
			virtual void		wait() = 0;						///< Wait for event
			virtual void		signal() = 0;					///< Signal event has occured
		};

		class xevent_factory
		{
		public:
			virtual				~xevent_factory()				{ }

			virtual xevent*		construct() = 0;				///< Called from main thread
			virtual void		destruct(xevent*) = 0;			///< Called from io thread
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