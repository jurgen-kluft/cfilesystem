#ifndef __X_FILESYSTEM_WIN32_H__
#define __X_FILESYSTEM_WIN32_H__
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

		extern xfiledevice*	x_CreateFileDevicePC(const char* pDrivePath, xbool boCanWrite);
		extern void			x_DestroyFileDevicePC(xfiledevice*);
	};

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

//==============================================================================
// END __X_FILESYSTEM_WIN32_H__
//==============================================================================
#endif