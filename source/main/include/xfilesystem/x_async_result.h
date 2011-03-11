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
		
		///< Async result implementation interface
		///< User doesn't deal with this object, it is meant for extendability
		class xiasync_result
		{
		public:
			virtual bool			isCompleted() = 0;
			virtual void			waitUntilCompleted() = 0;

		protected:
			friend class xasync_result;
			virtual void			hold() = 0;
			virtual s32				release() = 0;
			virtual void			destroy() = 0;
		};

		///< Async result (Reference counted)
		///< User can hold on to this object and use it to check for an async file operation
		///< to complete. Holding on to this object means that you are holding on to the
		///< stream object as well.
		class xasync_result
		{
			xiasync_result*			mImplementation;

		public:
									xasync_result();
									xasync_result(xiasync_result* imp);
									xasync_result(const xasync_result&);
									~xasync_result();

			bool					isCompleted() const;
			void					waitUntilCompleted();
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
