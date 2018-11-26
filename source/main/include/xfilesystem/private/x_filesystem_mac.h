#ifndef __X_FILESYSTEM_MAC_H__
#define __X_FILESYSTEM_MAC_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase/x_types.h"

//==============================================================================
// xcore namespace
//==============================================================================
namespace xcore
{
	namespace xfilesystem
	{
		class xfiledevice;

		extern xfiledevice*	x_CreateFileDeviceMac(const char* pDrivePath,xbool boCanWrite);
		extern void			x_DestroyFileDeviceMac(xfiledevice*);
	};

};

#endif