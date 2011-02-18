#ifndef __X_FILESYSTEM_DEVICE_H__
#define __X_FILESYSTEM_DEVICE_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase\x_types.h"

//==============================================================================
// xCore namespace
//==============================================================================
namespace xcore
{
	namespace xfilesystem
	{
		// Forward declares
		class FileInfo;
		class AsyncIOInfo;

		class xfiledevice
		{
		public:
			virtual			~xfiledevice() {}

			virtual void	HandleAsync(AsyncIOInfo* pAsync, FileInfo* pInfo) = 0;

			virtual bool	OpenOrCreateFile(FileInfo* pInfo) = 0;
			virtual bool	LengthOfFile(FileInfo* pInfo, u64& outLength) = 0;
			virtual bool	CloseFile(FileInfo* pInfo) = 0;
			virtual bool	DeleteFile(FileInfo* pInfo) = 0;
			virtual bool	ReadFile(FileInfo* pInfo, void* buffer, u64 count, u64& outNumBytesRead) = 0;
			virtual bool	WriteFile(FileInfo* pInfo, const void* buffer, u64 count, u64& outNumBytesWritten) = 0;

			virtual bool	SeekOrigin(FileInfo* pInfo, u64 pos, u64& newPos) = 0;
			virtual bool	SeekCurrent(FileInfo* pInfo, u64 pos, u64& newPos) = 0;
			virtual bool	SeekEnd(FileInfo* pInfo, u64 pos, u64& newPos) = 0;

			virtual bool	Sync(FileInfo* pInfo) = 0;
		};
	};

	//==============================================================================
	// END xCore namespace
	//==============================================================================
};

//==============================================================================
// END __X_FILESYSTEM_360_H__
//==============================================================================
#endif