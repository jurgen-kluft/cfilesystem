#ifndef __X_FILESYSTEM_360_H__
#define __X_FILESYSTEM_360_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase\x_types.h"

//==============================================================================
// xcore namespace
//==============================================================================
namespace xcore
{
	namespace xfilesystem
	{
		class xfiledevice;

		extern xfiledevice*	x_CreateFileDevice360(const char* pDrivePath, bool boCanWrite);
		extern void			x_DestroyFileDevice360(xfiledevice*);
	};

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

//==============================================================================
// END __X_FILESYSTEM_360_H__
//==============================================================================
#endif