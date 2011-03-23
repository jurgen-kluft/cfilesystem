#ifndef __X_FILESYSTEM_H__
#define __X_FILESYSTEM_H__
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
	///< Forward declares
	class x_iallocator;

	namespace xfilesystem
	{
		///< Forward declares
		class xthreading;

		///< Initialization
		extern void				init				( xthreading* threading, x_iallocator* allocator );
		extern void				exit				( void );

		///< IoThread, user has to call this from the IoThread
		extern void				ioThreadWorker		( void );

		///< Update, user has to call this from the 'main' thread, not from the IoThread
		extern void				update				( void );
	};

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

//==============================================================================
// END __X_FILESYSTEM_H__
//==============================================================================
#endif