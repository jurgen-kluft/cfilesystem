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
		enum EDeviceType
		{	
			FS_DEVICE_UNDEFINED		= 0,
			FS_DEVICE_HOST			= 10,
			FS_DEVICE_BDVD			= 20,
			FS_DEVICE_DVD			= 30,
			FS_DEVICE_UMD			= 40,
			FS_DEVICE_HDD			= 50,
			FS_DEVICE_MS			= 60,
			FS_DEVICE_CACHE			= 70,
			FS_DEVICE_USB			= 80,
			FS_DEVICE_REMOTE		= 90,

			FS_DEVICE_DEFAULT		= FS_DEVICE_HOST,
		};

		// Forward declares
		struct xfileinfo;
		struct xfileasync;

		class xfiledevice
		{
		protected:
			virtual					~xfiledevice() {}

		public:
			virtual bool			Init() = 0;
			virtual bool			Exit() = 0;

			virtual EDeviceType		GetType() const = 0;
			virtual bool			IsReadOnly() const = 0;

			virtual void			HandleAsync(xfileasync* pAsync, xfileinfo* pInfo) = 0;

			virtual bool			OpenOrCreateFile(xfileinfo* pInfo) = 0;
			virtual bool			SetLengthOfFile(xfileinfo* pInfo, u64 inLength) = 0;
			virtual bool			LengthOfFile(xfileinfo* pInfo, u64& outLength) = 0;
			virtual bool			CloseFile(xfileinfo* pInfo) = 0;
			virtual bool			DeleteFile(xfileinfo* pInfo) = 0;
			virtual bool			ReadFile(xfileinfo* pInfo, void* buffer, u64 count, u64& outNumBytesRead) = 0;
			virtual bool			WriteFile(xfileinfo* pInfo, const void* buffer, u64 count, u64& outNumBytesWritten) = 0;

			virtual bool			SeekOrigin(xfileinfo* pInfo, u64 pos, u64& newPos) = 0;
			virtual bool			SeekCurrent(xfileinfo* pInfo, u64 pos, u64& newPos) = 0;
			virtual bool			SeekEnd(xfileinfo* pInfo, u64 pos, u64& newPos) = 0;

			virtual bool			Sync(xfileinfo* pInfo) = 0;
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