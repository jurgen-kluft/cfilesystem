#include "xbase\x_target.h"
#ifdef TARGET_PC

//==============================================================================
// INCLUDES
//==============================================================================
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOKANJI
#include <windows.h>
#include <stdio.h>

#include "xbase\x_debug.h"
#include "xbase\x_limits.h"
#include "xbase\x_string_std.h"
#include "xbase\x_va_list.h"

#include "xfilesystem\private\x_filesystem_common.h"
#include "xfilesystem\private\x_filesystem_ps3.h"
#include "xfilesystem\x_filedevice.h"
#include "xfilesystem\private\x_fileinfo.h"
#include "xfilesystem\private\x_fileasync.h"

namespace xcore
{
	//------------------------------------------------------------------------------------------
	//---------------------------------- PC IO Functions ------------------------------------
	//------------------------------------------------------------------------------------------
	namespace xfilesystem
	{
		class FileDevice_PC_System : public xfiledevice
		{
			xbool					mCanWrite;

		public:
									FileDevice_PC_System(xbool boCanWrite)
										: mCanWrite(boCanWrite)							{ } 
			virtual					~FileDevice_PC_System()								{ }

			virtual bool			canSeek() const										{ return true; }
			virtual bool			canWrite() const									{ return mCanWrite; }

			virtual bool			openOrCreateFile(u32 nFileIndex, const char* szFilename, bool boRead, bool boWrite, u32& nFileHandle);
			virtual bool			setLengthOfFile(u32 nFileHandle, u64 inLength);
			virtual bool			lengthOfFile(u32 nFileHandle, u64& outLength);
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

		xfiledevice*		x_CreateFileDevicePC(xbool boCanWrite)
		{
			FileDevice_PC_System* file_device = new FileDevice_PC_System(boCanWrite);
			return file_device;
		}

		void			x_DestroyFileDevicePC(xfiledevice* device)
		{
			FileDevice_PC_System* file_device = (FileDevice_PC_System*)device;
			delete file_device;
		}
		
		bool FileDevice_PC_System::openOrCreateFile(u32 nFileIndex, const char* szFilename, bool boRead, bool boWrite, u32& nFileHandle)
		{
			u32 shareType	= FILE_SHARE_READ;
			u32 fileMode	= GENERIC_WRITE|GENERIC_READ;
			u32 disposition	= 0;
			if(boWrite)
			{
				disposition	= CREATE_ALWAYS;
			}
			else
			{
				fileMode	&= ~GENERIC_WRITE;
				disposition	= OPEN_EXISTING;
			}

			// FILE_FLAG_OVERLAPPED     -    This allows asynchronous I/O.
			// FILE_FLAG_NO_BUFFERING   -    No cached asynchronous I/O.
			u32 attrFlags	= FILE_ATTRIBUTE_NORMAL;

			HANDLE handle = ::CreateFile(szFilename, fileMode, shareType, NULL, disposition, attrFlags, NULL);
			nFileHandle = (u32)handle;
			return nFileHandle != (u32)INVALID_HANDLE_VALUE;
		}

		bool FileDevice_PC_System::setLengthOfFile(u32 nFileHandle, u64 inLength)
		{
			xsize_t distanceLow = (xsize_t)inLength;
			xsize_t distanceHigh = (xsize_t)(inLength >> 32);
			::SetFilePointer((HANDLE)nFileHandle, (LONG)distanceLow, (PLONG)&distanceHigh, FILE_BEGIN);
			::SetEndOfFile((HANDLE)nFileHandle);
			return true;
		}

		bool FileDevice_PC_System::lengthOfFile(u32 nFileHandle, u64& outLength)
		{
			DWORD lowSize, highSize;
			lowSize = ::GetFileSize((HANDLE)nFileHandle, &highSize);
			outLength = highSize;
			outLength = outLength << 16;
			outLength = outLength << 16;
			outLength = outLength | lowSize;
			return true;
		}

		bool FileDevice_PC_System::closeFile(u32 nFileHandle)
		{
			if (!CloseHandle((HANDLE)nFileHandle))
				return false;
			return true;
		}

		bool FileDevice_PC_System::deleteFile(u32 nFileHandle, const char* szFilename)
		{
			if (!closeFile(nFileHandle))
				return false;
			if (!::DeleteFile(szFilename))
				return false;
			return true;
		}

		bool FileDevice_PC_System::readFile(u32 nFileHandle, u64 pos, void* buffer, uintfs count, u64& outNumBytesRead)
		{
			u64 newPos;
			if (seek(nFileHandle, __SEEK_ORIGIN, pos, newPos))
			{
				DWORD numBytesRead;
				xbool boSuccess = ::ReadFile((HANDLE)nFileHandle, buffer, (DWORD)count, &numBytesRead, NULL); 

				if (boSuccess)
				{
					outNumBytesRead = numBytesRead;
				}

				if (!boSuccess) 
				{ 
					outNumBytesRead = -1;

					DWORD dwError = ::GetLastError();
					switch(dwError) 
					{ 
					case ERROR_HANDLE_EOF:	// We have reached the end of the FilePC during the call to ReadFile 
						return false;
					case ERROR_IO_PENDING: 
						return false; 
					default:
						return false;
					}
				}

				return true;
			}
			return false;
		}
		bool FileDevice_PC_System::writeFile(u32 nFileHandle, u64 pos, const void* buffer, uintfs count, u64& outNumBytesWritten)
		{
			u64 newPos;
			if (seek(nFileHandle, __SEEK_ORIGIN, pos, newPos))
			{
				DWORD numBytesWritten;
				xbool boSuccess = ::WriteFile((HANDLE)nFileHandle, buffer, (DWORD)count, &numBytesWritten, NULL); 

				if (boSuccess)
				{
					outNumBytesWritten = numBytesWritten;
				}

				if (!boSuccess) 
				{ 
					outNumBytesWritten = -1;

					DWORD dwError = ::GetLastError();
					switch(dwError) 
					{ 
					case ERROR_HANDLE_EOF:											// We have reached the end of the FilePC during the call to WriteFile 
						return false;
					case ERROR_IO_PENDING: 
						return false; 
					default:
						return false;
					}
				}

				return true;
			}
			return false;
		}

		bool FileDevice_PC_System::seek(u32 nFileHandle, ESeekMode mode, u64 pos, u64& newPos)
		{
			s32 hardwareMode = 0;
			switch(mode)
			{
			case __SEEK_ORIGIN : hardwareMode = FILE_BEGIN; break;
			case __SEEK_CURRENT: hardwareMode = FILE_CURRENT; break;
			case __SEEK_END    : hardwareMode = FILE_END; break; 
			default: 
				ASSERT(0);
				break;
			}

			// seek!
			LARGE_INTEGER position;
			LARGE_INTEGER newFilePointer;

			newPos = pos;
			position.LowPart  = (u32)pos;
			position.HighPart = 0;
			DWORD result = ::SetFilePointerEx((HANDLE)nFileHandle, position, &newFilePointer, hardwareMode);
			if (!result)
			{
				if (result == INVALID_SET_FILE_POINTER) 
				{
					// Failed to seek.
				}
				return false;
			}
			newPos = newFilePointer.LowPart;
			return true;
		}
		bool	FileDevice_PC_System::seekOrigin(u32 nFileHandle, u64 pos, u64& newPos)
		{
			return seek(nFileHandle, __SEEK_ORIGIN, pos, newPos);
		}

		bool	FileDevice_PC_System::seekCurrent(u32 nFileHandle, u64 pos, u64& newPos)
		{
			return seek(nFileHandle, __SEEK_CURRENT, pos, newPos);
		}

		bool	FileDevice_PC_System::seekEnd(u32 nFileHandle, u64 pos, u64& newPos)
		{
			return seek(nFileHandle, __SEEK_END, pos, newPos);
		}

		//==============================================================================
		// END xfilesystem namespace
		//==============================================================================
	};

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

#endif // TARGET_PC
