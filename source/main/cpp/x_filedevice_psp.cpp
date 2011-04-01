#include "xbase\x_target.h"
#ifdef TARGET_PSP

//==============================================================================
// INCLUDES
//==============================================================================
#include <kernel.h>
#include <kerror.h>
#include <stdio.h>
#include <psptypes.h>
#include <psperror.h>
#include <iofilemgr.h>
#include <mediaman.h>
#include <umddevctl.h>
#include <kernelutils.h>
#include <utility/utility_module.h>
#include <np/np_drm.h>

#include "xbase\x_debug.h"
#include "xbase\x_limits.h"
#include "xbase\x_string_std.h"
#include "xbase\x_va_list.h"

#include "xfilesystem\private\x_filesystem_common.h"
#include "xfilesystem\private\x_filesystem_ps3.h"
#include "xfilesystem\x_filedevice.h"
#include "xfilesystem\private\x_filedata.h"
#include "xfilesystem\private\x_fileasync.h"

namespace xcore
{

	//------------------------------------------------------------------------------------------
	//---------------------------------- PSP IO Functions ------------------------------------
	//------------------------------------------------------------------------------------------
	namespace xfilesystem
	{
		class FileDevice_PSP_System : public xfiledevice
		{
			xbool					mCanWrite;

		public:
									FileDevice_PSP_System(xbool boCanWrite)
										: mCanWrite(boCanWrite)							{ }
			virtual					~FileDevice_PSP_System()							{ }

			virtual bool			canSeek() const										{ return true; }
			virtual bool			canWrite() const									{ return mCanWrite; }

			virtual bool			openOrCreateFile(u32 nFileIndex, const char* szFilename, bool boRead, bool boWrite, u32& nFileHandle);
			virtual bool			setLengthOfFile(u32 nFileHandle, u64 inLength);
			virtual bool			lengthOfFile(u32 nFileHandle, u64& outLength) const;
			virtual bool			closeFile(u32 nFileHandle);
			virtual bool			deleteFile(u32 nFileHandle, const char* szFilename);
			virtual bool			readFile(u32 nFileHandle, u64 pos, void* buffer, u64 count, u64& outNumBytesRead);
			virtual bool			writeFile(u32 nFileHandle, u64 pos, const void* buffer, u64 count, u64& outNumBytesWritten);

			enum ESeekMode { __SEEK_ORIGIN = 1, __SEEK_CURRENT = 2, __SEEK_END = 3, };
			bool					seek(u32 nFileHandle, ESeekMode mode, u64 pos, u64& newPos);
			bool					seekOrigin(u32 nFileHandle, u64 pos, u64& newPos);
			bool					seekCurrent(u32 nFileHandle, u64 pos, u64& newPos);
			bool					seekEnd(u32 nFileHandle, u64 pos, u64& newPos);

			XFILESYSTEM_OBJECT_NEW_DELETE()
		};

		xfiledevice*		x_CreateFileDevicePSP(xbool boCanWrite)
		{
			FileDevice_PSP_System* file_device = new FileDevice_PSP_System();
			return file_device;
		}

		void			x_DestroyFileDevicePS3(xfiledevice* device)
		{
			FileDevice_PSP_System* file_device = (FileDevice_PSP_System*)device;
			delete file_device;
		}
	
		bool FileDevice_PSP_System::openOrCreateFile(u32 nFileIndex, const char* szFilename, bool boRead, bool boWrite, u32& nFileHandle)
		{
			s32 flag;
			if (boWrite)
			{
				flag = SCE_O_RDWR | SCE_O_TRUNC | SCE_O_CREAT;
			}
			else
			{
				flag = SCE_O_RDONLY;
			}
			if (x_stricmp(GetFileExtension(szFilename), "EDAT") == 0)
			{
				flag = SCE_FGAMEDATA | SCE_O_RDONLY;
			}
			
			SceMode mode = 0;
			SceUID nResult = sceIoOpen(szFilename, flag, mode);
			bool boError = nResult < 0;
			if (!boError)
			{
				nFileHandle = (u32)nResult;
			}
			return !boError;
		}

		bool FileDevice_PSP_System::lengthOfFile(u32 nFileHandle, u64& outLength) const
		{
			SceIoStat stats;
			s32 nResult = sceIoGetstat(szFilename, &stats);
			bool boSuccess = (nResult == SCE_KERNEL_ERROR_OK);
			if (boSuccess)
				outLength = stats.st_size;
			else
				outLength = -1;

			return boSuccess;
		}

		bool FileDevice_PSP_System::closeFile(u32 nFileHandle)
		{
			s32 nResult = sceIoClose(nFileHandle);
			return nResult==SCE_KERNEL_ERROR_OK;
		}

		bool FileDevice_PSP_System::deleteFile(u32 nFileHandle, const char* szFilename)
		{
			s32 nResult = sceIoRemove(szFilename);
			return nResult==SCE_KERNEL_ERROR_OK;
		}

		bool FileDevice_PSP_System::readFile(u32 nFileHandle, void* buffer, u64 pos, u64 count, u64& outNumBytesRead)
		{
			u64 newPos;
			if (seek(nFileHandle, __SEEK_ORIGIN, pos, newPos))
			{
				SceSSize numBytesWritten = sceIoRead(nFileHandle, buffer, count);
				bool boSuccess = numBytesWritten>=0;
				if (boSuccess)
				{
					outNumBytesRead = numBytesWritten;
				}
				return boSuccess;
			}
			return false;
		}
		bool FileDevice_PSP_System::writeFile(u32 nFileHandle, const void* buffer, u64 pos, u64 count, u64& outNumBytesWritten)
		{
			u64 newPos;
			if (seek(nFileHandle, __SEEK_ORIGIN, pos, newPos))
			{
				SceSSize numBytesWritten = sceIoWrite(nFileHandle, buffer, count);
				bool boSuccess = numBytesWritten>=0;
				if (boSuccess)
				{
					outNumBytesWritten = numBytesWritten;
				}
				return boSuccess;
			}
			return false;
		}

		bool FileDevice_PSP_System::seek(u32 nFileHandle, ESeekMode mode, u64 pos, u64& newPos)
		{
			SceOff offset = pos;
			SceOff newOffset = sceIoLseek(nFileHandle, offset, mode);
			bool boSuccess = newOffset>=0;
			if (boSuccess)
			{
				newPos = newOffset;
			}
			return boSuccess;
		}
		bool	FileDevice_PSP_System::seekOrigin(u32 nFileHandle, u64 pos, u64& newPos)
		{
			return seek(pInfo, __SEEK_ORIGIN, pos, newPos);
		}

		bool	FileDevice_PSP_System::seekCurrent(u32 nFileHandle, u64 pos, u64& newPos)
		{
			return seek(pInfo, __SEEK_CURRENT, pos, newPos);
		}

		bool	FileDevice_PSP_System::seekEnd(u32 nFileHandle, u64 pos, u64& newPos)
		{
			return seek(pInfo, __SEEK_END, pos, newPos);
		}

		//==============================================================================
		// END xfilesystem namespace
		//==============================================================================
	};

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

#endif // TARGET_PSP
