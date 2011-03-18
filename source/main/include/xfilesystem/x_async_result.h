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
		///< Forward declares
		class xiasync_result;

		///< Async result (Reference counted)
		///< User can hold on to this object and use it to check for an async file operation
		///< to complete. Holding on to this object means that you are holding on to the
		///< stream object as well.
		class xasync_result
		{
		public:
									xasync_result();
									xasync_result(const xasync_result&);
									~xasync_result();

			bool					isCompleted() const;
			void					waitUntilCompleted();

			void					operator =  (const xasync_result&);
			bool					operator == (const xasync_result&) const;
			bool					operator != (const xasync_result&) const;

		protected:
									xasync_result(xiasync_result* imp);

			xiasync_result*			mImplementation;
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
