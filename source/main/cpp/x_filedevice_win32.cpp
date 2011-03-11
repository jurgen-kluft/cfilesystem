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
#include "xfilesystem\private\x_filedevice.h"
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
			EDeviceType				mDeviceType;

		public:
			virtual bool			Init() { return true; }
			virtual bool			Exit() { return true; }

			void					SetType(EDeviceType type)							{ mDeviceType = type; }
			virtual EDeviceType		GetType() const										{ return mDeviceType; }
			virtual bool			IsReadOnly() const
			{
				switch (mDeviceType)
				{
					case FS_DEVICE_HOST			: return false; break;
					case FS_DEVICE_BDVD			: return true; break;
					case FS_DEVICE_DVD			: return true; break;
					case FS_DEVICE_UMD			: return true; break;
					case FS_DEVICE_HDD			: return false; break;
					case FS_DEVICE_MS			: return false; break;
					case FS_DEVICE_CACHE		: return false; break;
					case FS_DEVICE_USB			: return false; break;
				}
				return true;
			}

			virtual void			HandleAsync(xfileasync* pAsync, xfileinfo* pInfo);

			virtual bool			OpenOrCreateFile(xfileinfo* pInfo);
			virtual bool			SetLengthOfFile(xfileinfo* pInfo, u64 inLength);
			virtual bool			LengthOfFile(xfileinfo* pInfo, u64& outLength);
			virtual bool			CloseFile(xfileinfo* pInfo);
			virtual bool			DeleteFile(xfileinfo* pInfo);
			virtual bool			ReadFile(xfileinfo* pInfo, void* buffer, u64 count, u64& outNumBytesRead);
			virtual bool			WriteFile(xfileinfo* pInfo, const void* buffer, u64 count, u64& outNumBytesWritten);

			virtual bool			SeekOrigin(xfileinfo* pInfo, u64 pos, u64& newPos);
			virtual bool			SeekCurrent(xfileinfo* pInfo, u64 pos, u64& newPos);
			virtual bool			SeekEnd(xfileinfo* pInfo, u64 pos, u64& newPos);

			virtual bool			Sync(xfileinfo* pInfo);

			enum EPcSeekMode
			{
				__SEEK_ORIGIN = 1,
				__SEEK_CURRENT = 2,
				__SEEK_END = 3,
			};
			virtual bool			Seek(xfileinfo* pInfo, EPcSeekMode mode, u64 pos, u64& newPos);
			virtual bool			GetBlockSize(xfileinfo* pInfo, u64& outSectorSize);

			void*					operator new (size_t size, void *p)					{ return p; }
			void					operator delete(void* mem, void* )					{ }	
		};

		xfiledevice*		x_CreateFileDevicePC(EDeviceType type)
		{
			void* mem = heapAlloc(sizeof(FileDevice_PC_System), 16);
			FileDevice_PC_System* file_device = new (mem) FileDevice_PC_System();
			file_device->SetType(type);
			file_device->Init();
			return file_device;
		}

		void			x_DestroyFileDevicePC(xfiledevice* device)
		{
			FileDevice_PC_System* file_device = (FileDevice_PC_System*)device;
			file_device->Exit();
			file_device->~FileDevice_PC_System();
			heapFree(file_device);
		}

		void FileDevice_PC_System::HandleAsync(xfileasync* pAsync, xfileinfo* pInfo)
		{
			if(pAsync==NULL || pInfo==NULL)
				return;

			if(pAsync->getStatus() == FILE_OP_STATUS_OPEN_PENDING)
			{
				pAsync->setStatus(FILE_OP_STATUS_OPENING);

				bool boError   = false;
				bool boSuccess = OpenOrCreateFile(pInfo);
				if (!boSuccess)
				{
					x_printf ("OpenOrCreateFile failed on file %s\n", x_va_list(pInfo->m_szFilename));
					boError = true;
				}
				else
				{
					u64 nPos;
					boSuccess = Seek(pInfo, __SEEK_END, 0, nPos);
					if ((!boSuccess) || ((pInfo->m_boWriting == false) && (nPos == 0)) )
					{
						x_printf ("Seek failed on file %s\n", x_va_list(pInfo->m_szFilename));
						boError = true;
					}
					else
					{
						u64 uSize = nPos; 
						boSuccess = Seek(pInfo, __SEEK_ORIGIN, 0, nPos);
						if (!boSuccess)
						{
							x_printf ("Seek failed on file %s\n", x_va_list(pInfo->m_szFilename));
							boError = true;
						}
						else
						{
							u64	uSectorSize = 4096;
							{
								u64 uPad = uSize % uSectorSize;
								if (uPad != 0)
								{
									uPad = uSectorSize - uPad;
								}

								u32 uRoundedSize			= (u32)(uSize + uPad);
								u32 uNumSectors 			= (u32)(uRoundedSize / uSectorSize);

								pInfo->m_uByteOffset		= 0;
								pInfo->m_uByteSize			= uSize;
								pInfo->m_uSectorOffset		= 0;
								pInfo->m_uNumSectors		= uNumSectors;
								pInfo->m_uSectorSize		= uSectorSize;
							}
						}
					}
				}
				if (boError)
				{
					if (pInfo->m_nFileHandle != (u32)INVALID_FILE_HANDLE)
					{
						bool boClose = CloseFile(pInfo);
						if (!boClose)
						{
							x_printf ("CloseFile failed on file %s\n", x_va_list(pInfo->m_szFilename));
						}
						pInfo->m_nFileHandle = (u32)INVALID_FILE_HANDLE;
					}
				}

				pAsync->setStatus(FILE_OP_STATUS_DONE);
			}
			else if(pAsync->getStatus() == FILE_OP_STATUS_CLOSE_PENDING)
			{
				pAsync->setStatus(FILE_OP_STATUS_CLOSING);

				bool boClose = CloseFile(pInfo);
				if (!boClose)
				{
					x_printf ("CloseFile failed on file %s\n", x_va_list(pInfo->m_szFilename));
				}
							
				pInfo->m_nFileHandle	= (u32)INVALID_FILE_HANDLE;
				pAsync->setStatus(FILE_OP_STATUS_DONE);
			}
			else if(pAsync->getStatus() == FILE_OP_STATUS_DELETE_PENDING)
			{
				pAsync->setStatus(FILE_OP_STATUS_DELETING);

				bool boClose = CloseFile(pInfo);
				if (!boClose)
				{
					x_printf ("CloseFile failed on file %s\n", x_va_list(pInfo->m_szFilename));
				}
				else
				{
					bool boDelete = DeleteFile(pInfo);
					if (!boDelete)
					{
						x_printf ("DeleteFile failed on file %s\n", x_va_list(pInfo->m_szFilename));
					}
				}

				pInfo->m_nFileHandle	= (u32)INVALID_FILE_HANDLE;
				pAsync->setStatus(FILE_OP_STATUS_DONE);
			}
			else if(pAsync->getStatus() == FILE_OP_STATUS_STAT_PENDING)
			{
				pAsync->setStatus(FILE_OP_STATUS_STATING);

				//@TODO: use stats

				pAsync->setStatus(FILE_OP_STATUS_DONE);
			}
			else if(pAsync->getStatus() == FILE_OP_STATUS_READ_PENDING)
			{
				u64	nPos;
				bool boSeek = Seek(pInfo, __SEEK_ORIGIN, pAsync->getReadWriteOffset(), nPos);
				if (!boSeek)
				{
					x_printf ("Seek failed on file %s\n", x_va_list(pInfo->m_szFilename));
				}

				pAsync->setStatus(FILE_OP_STATUS_READING);

				u64 nReadSize;
				bool boRead = ReadFile(pInfo, pAsync->getReadAddress(), (u32)pAsync->getReadWriteSize(), nReadSize);
				if (!boRead)
				{
					x_printf ("ReadFile failed on file %s\n", x_va_list(pInfo->m_szFilename));
				}

				pAsync->setStatus(FILE_OP_STATUS_DONE);
			}
			else if(pAsync->getStatus() == FILE_OP_STATUS_WRITE_PENDING)
			{
				u64	nPos;
				bool boSeek = Seek(pInfo, __SEEK_ORIGIN, pAsync->getReadWriteOffset(), nPos);
				if (!boSeek)
				{
					x_printf ("Seek failed on file %s\n", x_va_list(pInfo->m_szFilename));
				}

				pAsync->setStatus(FILE_OP_STATUS_WRITING);

				u64 nWriteSize;
				bool boWrite = WriteFile(pInfo, pAsync->getWriteAddress(), (u32)pAsync->getReadWriteSize(), nWriteSize);
				if (!boWrite)
				{
					x_printf ("WriteFile failed on file %s\n", x_va_list(pInfo->m_szFilename));
				}

				if (pInfo->m_pFileDevice->GetType() != FS_DEVICE_HOST)
				{
					bool boSyncResult = Sync(pInfo);
					if (!boSyncResult)
					{
						x_printf ("__Sync failed on file %s\n", x_va_list(pInfo->m_szFilename));
					}
				}

				pAsync->setStatus(FILE_OP_STATUS_DONE);
			}
		}

		bool FileDevice_PC_System::OpenOrCreateFile(xfileinfo* pInfo)
		{
			u32 shareType	= FILE_SHARE_READ;
			u32 fileMode	= GENERIC_WRITE|GENERIC_READ;
			u32 disposition	= 0;
			if(pInfo->m_boWriting)
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

			HANDLE handle = ::CreateFile(pInfo->m_szFilename, fileMode, shareType, NULL, disposition, attrFlags, NULL);
			pInfo->m_nFileHandle = (u32)handle;
			return pInfo->m_nFileHandle != (u32)INVALID_HANDLE_VALUE;
		}

		bool FileDevice_PC_System::SetLengthOfFile(xfileinfo* pInfo, u64 inLength)
		{
			xsize_t distanceLow = (xsize_t)inLength;
			xsize_t distanceHigh = (xsize_t)(inLength >> 32);
			::SetFilePointer((HANDLE)pInfo->m_nFileHandle, (LONG)distanceLow, (PLONG)&distanceHigh, FILE_BEGIN);
			::SetEndOfFile((HANDLE)pInfo->m_nFileHandle);
			return true;
		}

		bool FileDevice_PC_System::LengthOfFile(xfileinfo* pInfo, u64& outLength)
		{
			DWORD lowSize, highSize;
			lowSize = ::GetFileSize((HANDLE)pInfo->m_nFileHandle, &highSize);
			outLength = highSize;
			outLength = outLength << 16;
			outLength = outLength << 16;
			outLength = outLength | lowSize;
			return true;
		}

		bool FileDevice_PC_System::CloseFile(xfileinfo* pInfo)
		{
			if (!CloseHandle((HANDLE)pInfo->m_nFileHandle))
				return false;
			return true;
		}

		bool FileDevice_PC_System::DeleteFile(xfileinfo* pInfo)
		{
			if (!CloseFile(pInfo))
				return false;
			if (!::DeleteFile(pInfo->m_szFilename))
				return false;
			return true;
		}

		bool FileDevice_PC_System::ReadFile(xfileinfo* pInfo, void* buffer, uintfs count, u64& outNumBytesRead)
		{
			DWORD numBytesRead;
			xbool boSuccess = ::ReadFile((HANDLE)pInfo->m_nFileHandle, buffer, (DWORD)count, &numBytesRead, NULL); 

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
		bool FileDevice_PC_System::WriteFile(xfileinfo* pInfo, const void* buffer, uintfs count, u64& outNumBytesWritten)
		{
			DWORD numBytesWritten;
			xbool boSuccess = ::WriteFile((HANDLE)pInfo->m_nFileHandle, buffer, (DWORD)count, &numBytesWritten, NULL); 

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

		bool FileDevice_PC_System::Seek(xfileinfo* pInfo, EPcSeekMode mode, u64 pos, u64& newPos)
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
			DWORD result = ::SetFilePointerEx((HANDLE)pInfo->m_nFileHandle, position, &newFilePointer, hardwareMode);
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
		bool	FileDevice_PC_System::SeekOrigin(xfileinfo* pInfo, u64 pos, u64& newPos)
		{
			return Seek(pInfo, __SEEK_ORIGIN, pos, newPos);
		}

		bool	FileDevice_PC_System::SeekCurrent(xfileinfo* pInfo, u64 pos, u64& newPos)
		{
			return Seek(pInfo, __SEEK_CURRENT, pos, newPos);
		}

		bool	FileDevice_PC_System::SeekEnd(xfileinfo* pInfo, u64 pos, u64& newPos)
		{
			return Seek(pInfo, __SEEK_END, pos, newPos);
		}

		bool FileDevice_PC_System::Sync(xfileinfo* pInfo)
		{
			return true;
		}

		bool FileDevice_PC_System::GetBlockSize(xfileinfo* pInfo, u64& outSectorSize)
		{
			outSectorSize = 2048;
			return true;
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
