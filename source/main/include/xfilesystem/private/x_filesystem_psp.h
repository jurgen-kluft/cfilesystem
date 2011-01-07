#ifndef __X_FILESYSTEM_PSP_H__
#define __X_FILESYSTEM_PSP_H__
#include "..\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

//==============================================================================
// INCLUDES
//==============================================================================
#include "..\x_types.h"

//==============================================================================
// xCore namespace
//==============================================================================
namespace xcore
{
	namespace xfilesystem
	{
		extern void			SetDRMLicenseKey	( const u8* p8Key );
		extern void			SetMemoryCardPath	( const char* szMemoryCardPath );
		extern s32			SaveToMemoryStick	( const char* szPhotosDir, const char* szDirectory, char* szFilename, void* pData, s32 nSizeBytes );
	};

	//==============================================================================
	// END xCore namespace
	//==============================================================================
};

//==============================================================================
// END __X_FILESYSTEM_PSP_H__
//==============================================================================
#endif