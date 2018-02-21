#ifndef __X_FILESYSTEM_DEVICE_ALIAS_H__
#define __X_FILESYSTEM_DEVICE_ALIAS_H__
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
	//==============================================================================
	// xdevicealias
	//==============================================================================

	namespace xfilesystem
	{
		class xfiledevice;

		//------------------------------------------------------------------------------
		// Description:
		//     An alias maps an string to a file device.
		//     This can act as an re-direction and is useful for defining folders
		//     as drives, remap drives etc..
		//
		//     source:\folder\filename.ext, where source = c:\temp
		//     data:\folder\filename.ext, where data = g:\
		//     dvd:\folder\filename.ext, where dvd = g:\
		//     
		//------------------------------------------------------------------------------

		extern bool		x_RegisterAlias (const char* alias, const char* aliasTarget);
		extern bool		x_RegisterAlias (const char* alias, xfiledevice* device, const char* remap=NULL);
		extern bool		x_RegisterSysPathAlias (const char* alias, const char* path);
	};

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

//==============================================================================
// END __X_FILESYSTEM_DEVICE_ALIAS_H__
//==============================================================================
#endif