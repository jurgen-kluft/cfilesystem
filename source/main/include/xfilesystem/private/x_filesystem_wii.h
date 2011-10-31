#ifndef __X_FILESYSTEM_WII_H__
#define __X_FILESYSTEM_WII_H__
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

		extern xfiledevice*	x_CreateFileDeviceWII();
		extern void			x_DestroyFileDeviceWII(xfiledevice*);

		extern xfiledevice*	x_CreateNandDeviceWII();
		extern void	x_DestroyNandDeviceWII(xfiledevice*);
	};

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

//==============================================================================
// END __X_FILESYSTEM_WII_H__
//==============================================================================
#endif