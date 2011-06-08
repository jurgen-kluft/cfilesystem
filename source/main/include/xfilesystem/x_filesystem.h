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
		extern void				init				( u32 max_open_streams, xthreading* threading, x_iallocator* allocator );
		extern void				exit				( void );

		///< doIO; user has to call this from either the main thread or an Io thread.
		///< This call will block the calling thread and it will stay in a do-while
		///< until threading->loop() is false.
		extern void				doIO				( void );
	};

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

//==============================================================================
// END __X_FILESYSTEM_H__
//==============================================================================
#endif