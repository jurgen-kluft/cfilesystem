#ifndef __X_FILESYSTEM_3DS_H__
#define __X_FILESYSTEM_3DS_H__
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

		extern xfiledevice*	x_CreateFileDevice3DS(xbool boCanWrite);
		extern void			x_DestroyFileDevice3DS(xfiledevice*);
	};

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

//==============================================================================
// END __X_FILESYSTEM_3DS_H__
//==============================================================================
#endif
