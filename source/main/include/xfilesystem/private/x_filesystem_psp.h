#ifndef __X_FILESYSTEM_PSP_H__
#define __X_FILESYSTEM_PSP_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase\x_types.h"
#include "xfilesystem\x_filedevice.h"

//==============================================================================
// xcore namespace
//==============================================================================
namespace xcore
{
	namespace xfilesystem
	{
		class xfiledevice;

		extern xfiledevice*	x_CreateFileDevicePSP(xbool boCanWrite);
		extern void			x_DestroyFileDevicePSP(xfiledevice*);

		extern void			setDRMLicenseKey	( const u8* p8Key );
		extern void			setMemoryCardPath	( const char* szMemoryCardPath );
		extern s32			saveToMemoryStick	( const char* szPhotosDir, const char* szDirectory, char* szFilename, void* pData, s32 nSizeBytes );
	};

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

//==============================================================================
// END __X_FILESYSTEM_PSP_H__
//==============================================================================
#endif