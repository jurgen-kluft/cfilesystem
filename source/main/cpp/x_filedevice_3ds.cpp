#include "xbase\x_target.h"
#ifdef TARGET_3DS

//==============================================================================
// INCLUDES
//==============================================================================


#include "xbase\x_debug.h"
#include "xbase\x_limits.h"
#include "xbase\x_string_std.h"
#include "xbase\x_va_list.h"

#include "xfilesystem\private\x_filesystem_common.h"
#include "xfilesystem\private\x_filesystem_3ds.h"
#include "xfilesystem\private\x_filedevice.h"
#include "xfilesystem\private\x_fileinfo.h"
#include "xfilesystem\private\x_fileasync.h"

namespace xcore
{

	//------------------------------------------------------------------------------------------
	//---------------------------------- Nintendo 3DS IO Functions -----------------------------
	//------------------------------------------------------------------------------------------
	namespace xfilesystem
	{
		class FileDevice_3DS_System : public xfiledevice
		{
			EDeviceType				mDeviceType;

		public:
			virtual bool			Init() {}
			virtual bool			Exit() {}

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

			enum E3dsSeekMode
			{
				__SEEK_ORIGIN = CELL_FS_SEEK_SET,
				__SEEK_CURRENT = CELL_FS_SEEK_CUR,
				__SEEK_END = CELL_FS_SEEK_END,
			};
			virtual bool			Seek(xfileinfo* pInfo, E3dsSeekMode mode, u64 pos, u64& newPos);
			virtual bool			GetBlockSize(xfileinfo* pInfo, u64& outSectorSize);

			void*					operator new (size_t size, void *p)					{ return p; }
			void					operator delete(void* mem, void* )					{ }	
		};

		xfiledevice*		x_CreateFileDevice3DS(EDeviceType type)
		{
			void* mem = heapAlloc(sizeof(FileDevice_3DS_System), 16);
			xfiledevice* file_device = new (mem) FileDevice_3DS_System();
			file_device->Init();
			return device;
		}

		void			x_DestroyFileDevice3DS(xfiledevice* device)
		{
			FileDevice_3DS_System* file_device = (FileDevice_3DS_System*)device;
			file_device->Exit();
			file_device->~FileDevice_3DS_System();
			heapFree(device);
		}

		void FileDevice_3DS_System::HandleAsync(xfileasync* pAsync, xfileinfo* pInfo)
		{
			if(pAsync==NULL || pInfo==NULL)
				return;

			if(pAsync->getStatus() == FILE_OP_STATUS_OPEN_PENDING)
			{
				pAsync->m_nStatus = FILE_OP_STATUS_OPENING;

				bool boError = false;
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
							boSuccess = GetBlockSize(pInfo, uSectorSize);
							if (boSuccess)
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
						CloseFile(pInfo);
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

				// Stats(pInfo);

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

				if (pInfo->m_eSource != FS_SOURCE_HOST)
				{
					bool boSyncResult = Sync(pInfo);
					if (!boSyncResult)
					{
						x_printf ("Sync failed on file %s\n", x_va_list(pInfo->m_szFilename));
					}
				}

				pAsync->m_nStatus	= FILE_OP_STATUS_DONE;
			}
		}

		bool FileDevice_3DS_System::OpenOrCreateFile(xfileinfo* pInfo)
		{
			s32	nFlags;
			if(pInfo->m_boWriting)
			{
				nFlags	= CELL_FS_O_CREAT | CELL_FS_O_RDWR;
			}
			else
			{
				nFlags	= CELL_FS_O_RDONLY;
			}

			s32	hFile	= INVALID_FILE_HANDLE;
			s32	nResult = cellFsOpen (pInfo->m_szFilename, nFlags, &hFile, NULL, 0);
			pInfo->m_nFileHandle = (u32)hFile;

			bool boSuccess = (nResult == CELL_OK);
			return boSuccess;
		}

		bool FileDevice_3DS_System::SetLengthOfFile(xfileinfo* pInfo, u64 inLength)
		{
			return false;
		}

		bool FileDevice_3DS_System::LengthOfFile(xfileinfo* pInfo, u64& outLength)
		{
			CellFsStat stats;
			CellFsErrno nResult = cellFsStat(pInfo->m_szFilename, &stats);
			bool boSuccess = (nResult == CELL_OK);
			if (boSuccess)
				outLength = stats.st_size;
			else
				outLength = 0;

			return boSuccess;
		}

		bool FileDevice_3DS_System::CloseFile(xfileinfo* pInfo)
		{
			s32 nResult = cellFsClose (pInfo->m_nFileHandle);
			bool boSuccess = false;
			if (nResult == CELL_OK)
			{
				pInfo->m_nFileHandle = (u32)INVALID_FILE_HANDLE;
				boSuccess = true;
			}
			return boSuccess;
		}

		bool FileDevice_3DS_System::DeleteFile(xfileinfo* pInfo)
		{
			s32 nResult = cellFsUnlink(pInfo->m_szFilename);
			bool boSuccess = false;
			if (nResult == CELL_OK)
			{
				pInfo->m_nFileHandle = (u32)INVALID_FILE_HANDLE;
				boSuccess = true;
			}
			return boSuccess;
		}

		bool FileDevice_3DS_System::ReadFile(xfileinfo* pInfo, void* buffer, uintfs count, u64& outNumBytesRead)
		{
			u64 numBytesRead;
			s32 nResult = cellFsRead (pInfo->m_nFileHandle, buffer, count, &numBytesRead);

			bool boSuccess = (nResult == CELL_OK);
			if (boSuccess)
			{
				outNumBytesRead = (u32)numBytesRead;
			}
			else
			{ 
				outNumBytesRead = 0;
			}
			return boSuccess;

		}
		bool FileDevice_3DS_System::WriteFile(xfileinfo* pInfo, const void* buffer, uintfs count, u64& outNumBytesWritten)
		{
			u64 numBytesWritten;
			s32 nResult = cellFsWrite (pInfo->m_nFileHandle, buffer, count, &numBytesWritten);

			bool boSuccess = (nResult == CELL_OK);
			if (boSuccess)
			{
				outNumBytesWritten = (u32)numBytesWritten;
			}
			else
			{ 
				outNumBytesWritten = 0;
			}
			return boSuccess;
		}

		bool FileDevice_3DS_System::Seek(xfileinfo* pInfo, E3dsSeekMode mode, u64 pos, u64& newPos)
		{
			u64	nPos;
			s32 nResult = cellFsLseek (pInfo->m_nFileHandle, pos, mode, &nPos);
			bool boSuccess = (nResult == CELL_OK);
			if (boSuccess)
			{
				newPos = nPos;
			}
			else
			{ 
				newPos = pos;
			}
			return boSuccess;
		}
		bool	FileDevice_3DS_System::SeekOrigin(xfileinfo* pInfo, u64 pos, u64& newPos)
		{
			return Seek(pInfo, __SEEK_ORIGIN, pos, newPos);
		}

		bool	FileDevice_3DS_System::SeekCurrent(xfileinfo* pInfo, u64 pos, u64& newPos)
		{
			return Seek(pInfo, __SEEK_CURRENT, pos, newPos);
		}

		bool	FileDevice_3DS_System::SeekEnd(xfileinfo* pInfo, u64 pos, u64& newPos)
		{
			return Seek(pInfo, __SEEK_END, pos, newPos);
		}

		bool FileDevice_3DS_System::Sync(xfileinfo* pInfo)
		{
			s32 nResult = cellFsFsync(pInfo->m_nFileHandle);
			return nResult == CELL_OK;
		}

		bool FileDevice_3DS_System::GetBlockSize(xfileinfo* pInfo, u64& outSectorSize)
		{
			u64	uSectorSize;
			u64	uBlockSize;
			s32 nResult = cellFsFGetBlockSize(pInfo->m_nFileHandle, &uSectorSize, &uBlockSize);
			bool boSuccess = (nResult == CELL_OK);
			if (boSuccess)
			{
				outSectorSize = uSectorSize;
			}
			else
			{ 
				outSectorSize = 0;
			}
			return boSuccess;
		}

		//==============================================================================
		// END xfilesystem namespace
		//==============================================================================
	};

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

#endif // TARGET_3DS
