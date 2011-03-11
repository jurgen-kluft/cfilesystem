#ifndef __X_FILESYSTEM_PS3_H__
#define __X_FILESYSTEM_PS3_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase\x_types.h"
#include "xfilesystem\private\x_filedevice.h"

//==============================================================================
// xcore namespace
//==============================================================================
namespace xcore
{
	namespace xfilesystem
	{
		extern xfiledevice*	x_CreateFileDevicePS3(EDeviceType type);
		extern void			x_DestroyFileDevicePS3(xfiledevice*);
	};

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

//==============================================================================
// END __X_FILESYSTEM_PS3_H__
//==============================================================================
#endif