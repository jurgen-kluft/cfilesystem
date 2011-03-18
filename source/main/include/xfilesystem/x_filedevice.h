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
// xcore namespace
//==============================================================================
namespace xcore
{
	namespace xfilesystem
	{
		///< File device
		///< 
		///< This interface exists to present a way to implement different types
		///< of file devices. The most obvious one is a file device that uses the
		///< system to manipulate files. To support HTTP based file manipulation
		///< the user could implement a file device that is using HTTP for data
		///< transfer.
		class xfiledevice
		{
		protected:
			virtual					~xfiledevice() {}

		public:
			virtual bool			canWrite() const = 0;
			virtual bool			canSeek() const = 0;

			virtual bool			openOrCreateFile(u32 nFileIndex, const char* szFilename, bool boRead, bool boWrite, u32& outFileHandle) = 0;
			virtual bool			setLengthOfFile(u32 nFileHandle, u64 inLength) = 0;
			virtual bool			lengthOfFile(u32 nFileHandle, u64& outLength) const = 0;
			virtual bool			closeFile(u32 nFileHandle) = 0;
			virtual bool			deleteFile(u32 nFileHandle, const char* szFilename) = 0;
			virtual bool			readFile(u32 nFileHandle, u64 pos, void* buffer, u64 count, u64& outNumBytesRead) = 0;
			virtual bool			writeFile(u32 nFileHandle, u64 pos, const void* buffer, u64 count, u64& outNumBytesWritten) = 0;
		};
	};

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

//==============================================================================
// END __X_FILESYSTEM_DEVICE_H__
//==============================================================================
#endif