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
#include "xfilesystem\private\x_filedevice.h"
#include "xfilesystem\private\x_fileinfo.h"
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
			EDeviceType				mDeviceType;

		public:
			virtual bool			Init();
			virtual bool			Exit();

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
			}
			virtual void			HandleAsync(xfileasync* pAsync, xfileinfo* pInfo);

			virtual bool			OpenOrCreateFile(xfileinfo* pInfo);
			virtual bool			LengthOfFile(xfileinfo* pInfo, u64& outLength);
			virtual bool			CloseFile(xfileinfo* pInfo);
			virtual bool			DeleteFile(xfileinfo* pInfo);
			virtual bool			ReadFile(xfileinfo* pInfo, void* buffer, u64 count, u64& outNumBytesRead);
			virtual bool			WriteFile(xfileinfo* pInfo, const void* buffer, u64 count, u64& outNumBytesWritten);

			virtual bool			SeekOrigin(xfileinfo* pInfo, u64 pos, u64& newPos);
			virtual bool			SeekCurrent(xfileinfo* pInfo, u64 pos, u64& newPos);
			virtual bool			SeekEnd(xfileinfo* pInfo, u64 pos, u64& newPos);

			virtual bool			Sync(xfileinfo* pInfo);

			enum EPspSeekMode
			{
				__SEEK_ORIGIN = SCE_SEEK_SET,
				__SEEK_CURRENT = SCE_SEEK_CUR,
				__SEEK_END = SCE_SEEK_END,
			};
			virtual bool			Seek(xfileinfo* pInfo, EPspSeekMode mode, u64 pos, u64& newPos);
			virtual bool			GetBlockSize(xfileinfo* pInfo, u64& outSectorSize);

			void*					operator new (size_t size, void *p)					{ return p; }
			void					operator delete(void* mem, void* )					{ }	
		};

		xfiledevice*		x_CreateFileDevicePSP(EDeviceType type)
		{
			void* mem = heapAlloc(sizeof(FileDevice_PSP_System), 16);
			FileDevice_PSP_System* file_device = new (mem) FileDevice_PSP_System();
			file_device->SetType(type);
			file_device->Init();
			return file_device;
		}

		void			x_DestroyFileDevicePS3(xfiledevice* device)
		{
			FileDevice_PSP_System* file_device = (FileDevice_PSP_System*)device;
			file_device->Exit();
			file_device->~FileDevice_PSP_System();
			heapFree(file_device);
		}

		bool FileDevice_PSP_System::Init()
		{
		}

		bool FileDevice_PSP_System::Exit()
		{
		}

		void FileDevice_PSP_System::HandleAsync(xfileasync* pAsync, xfileinfo* pInfo)
		{
			if(pAsync==NULL || pInfo==NULL)
				return;

			if(pAsync->getStatus() == FILE_OP_STATUS_OPEN_PENDING)
			{
				pAsync->m_nStatus = FILE_OP_STATUS_OPENING;

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

				pAsync->m_nStatus	= FILE_OP_STATUS_DONE;
			}
			else if(pAsync->getStatus() == FILE_OP_STATUS_CLOSE_PENDING)
			{
				pAsync->m_nStatus	= FILE_OP_STATUS_CLOSING;

				bool boClose = CloseFile(pInfo);
				if (!boClose)
				{
					x_printf ("CloseFile failed on file %s\n", x_va_list(pInfo->m_szFilename));
				}
							
				pInfo->m_nFileHandle	= (u32)INVALID_FILE_HANDLE;
				pAsync->m_nStatus	= FILE_OP_STATUS_DONE;
			}
			else if(pAsync->getStatus() == FILE_OP_STATUS_DELETE_PENDING)
			{
				pAsync->m_nStatus	= FILE_OP_STATUS_DELETING;

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
				pAsync->m_nStatus	= FILE_OP_STATUS_DONE;
			}
			else if(pAsync->getStatus() == FILE_OP_STATUS_STAT_PENDING)
			{
				pAsync->m_nStatus	= FILE_OP_STATUS_STATING;

				//@TODO: use stats

				pAsync->m_nStatus	= FILE_OP_STATUS_DONE;
			}
			else if(pAsync->getStatus() == FILE_OP_STATUS_READ_PENDING)
			{
				u64	nPos;
				bool boSeek = Seek(pInfo, __SEEK_ORIGIN, pAsync->m_uReadWriteOffset, nPos);
				if (!boSeek)
				{
					x_printf ("Seek failed on file %s\n", x_va_list(pInfo->m_szFilename));
				}

				pAsync->m_nStatus	= FILE_OP_STATUS_READING;

				u64 nReadSize;
				bool boRead = ReadFile(pInfo, pAsync->m_pReadAddress, (u32)pAsync->m_uReadWriteSize, nReadSize);
				if (!boRead)
				{
					x_printf ("ReadFile failed on file %s\n", x_va_list(pInfo->m_szFilename));
				}

				pAsync->m_nStatus	= FILE_OP_STATUS_DONE;
			}
			else if(pAsync->getStatus() == FILE_OP_STATUS_WRITE_PENDING)
			{
				u64	nPos;
				bool boSeek = Seek(pInfo, __SEEK_ORIGIN, pAsync->m_uReadWriteOffset, nPos);
				if (!boSeek)
				{
					x_printf ("Seek failed on file %s\n", x_va_list(pInfo->m_szFilename));
				}

				pAsync->m_nStatus	= FILE_OP_STATUS_WRITING;

				u64 nWriteSize;
				bool boWrite = WriteFile(pInfo, pAsync->m_pWriteAddress, (u32)pAsync->m_uReadWriteSize, nWriteSize);
				if (!boWrite)
				{
					x_printf ("WriteFile failed on file %s\n", x_va_list(pInfo->m_szFilename));
				}

				pAsync->m_nStatus	= FILE_OP_STATUS_DONE;
			}			
		}

		bool FileDevice_PSP_System::OpenOrCreateFile(xfileinfo* pInfo)
		{
			s32 flag;
			if (pInfo->m_boWriting)
			{
				flag = SCE_O_RDWR | SCE_O_TRUNC | SCE_O_CREAT;
			}
			else
			{
				flag = SCE_O_RDONLY;
			}
			if (x_stricmp(GetFileExtension(pInfo->m_szFilename), "EDAT") == 0)
			{
				flag = SCE_FGAMEDATA | SCE_O_RDONLY;
			}
			
			SceMode mode = 0;
			SceUID nResult = sceIoOpen(pInfo->m_szFilename, flag, mode);
			bool boError = nResult < 0;
			if (!boError)
			{
				pInfo->m_nFileHandle = (u32)nResult;
			}
			return !boError;
		}

		bool FileDevice_PSP_System::LengthOfFile(xfileinfo* pInfo, u64& outLength)
		{
			SceIoStat stats;
			s32 nResult = sceIoGetstat(pInfo->m_szFilename, &stats);
			bool boSuccess = (nResult == SCE_KERNEL_ERROR_OK);
			if (boSuccess)
				outLength = stats.st_size;
			else
				outLength = -1;

			return boSuccess;
		}

		bool FileDevice_PSP_System::CloseFile(xfileinfo* pInfo)
		{
			s32 nResult = sceIoClose(pInfo->m_nFileHandle);
			return nResult==SCE_KERNEL_ERROR_OK;
		}

		bool FileDevice_PSP_System::DeleteFile(xfileinfo* pInfo)
		{
			s32 nResult = sceIoRemove(pInfo->m_szFilename);
			return nResult==SCE_KERNEL_ERROR_OK;
		}

		bool FileDevice_PSP_System::ReadFile(xfileinfo* pInfo, void* buffer, uintfs count, u64& outNumBytesRead)
		{
			SceSSize numBytesWritten = sceIoRead(pInfo->m_nFileHandle, buffer, count);
			bool boSuccess = numBytesWritten>=0;
			if (boSuccess)
			{
				outNumBytesRead = numBytesWritten;
			}
			return boSuccess;

		}
		bool FileDevice_PSP_System::WriteFile(xfileinfo* pInfo, const void* buffer, uintfs count, u64& outNumBytesWritten)
		{
			SceSSize numBytesWritten = sceIoWrite(pInfo->m_nFileHandle, buffer, count);
			bool boSuccess = numBytesWritten>=0;
			if (boSuccess)
			{
				outNumBytesWritten = numBytesWritten;
			}
			return boSuccess;
		}

		bool FileDevice_PSP_System::Seek(xfileinfo* pInfo, EPspSeekMode mode, u64 pos, u64& newPos)
		{
			SceOff offset = pos;
			SceOff newOffset = sceIoLseek(pInfo->m_nFileHandle, offset, mode);
			bool boSuccess = newOffset>=0;
			if (boSuccess)
			{
				newPos = newOffset;
			}
			return boSuccess;
		}
		bool	FileDevice_PSP_System::SeekOrigin(xfileinfo* pInfo, u64 pos, u64& newPos)
		{
			return Seek(pInfo, __SEEK_ORIGIN, pos, newPos);
		}

		bool	FileDevice_PSP_System::SeekCurrent(xfileinfo* pInfo, u64 pos, u64& newPos)
		{
			return Seek(pInfo, __SEEK_CURRENT, pos, newPos);
		}

		bool	FileDevice_PSP_System::SeekEnd(xfileinfo* pInfo, u64 pos, u64& newPos)
		{
			return Seek(pInfo, __SEEK_END, pos, newPos);
		}

		bool FileDevice_PSP_System::Sync(xfileinfo* pInfo)
		{
			return true;
		}

		bool FileDevice_PSP_System::GetBlockSize(xfileinfo* pInfo, u64& outSectorSize)
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

#endif // TARGET_PSP
