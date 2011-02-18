#include "xbase\x_target.h"
#ifdef TARGET_PS3

//==============================================================================
// INCLUDES
//==============================================================================

#include <stdio.h>
#include <sys/paths.h>
#include <sys/process.h>
#include <sys/timer.h>
#include <cell/cell_fs.h>
#include <cell/fs/cell_fs_file_api.h>
#include <cell/sysmodule.h>
#include <sys/event.h>
#include <sys/ppu_thread.h>
#include <sys/synchronization.h>

#include "xbase\x_debug.h"
#include "xbase\x_limits.h"
#include "xbase\x_string_std.h"
#include "xbase\x_va_list.h"

#include "xfilesystem\private\x_filesystem_common.h"
#include "xfilesystem\private\x_filesystem_ps3.h"
#include "xfilesystem\private\x_filesystem_device.h"


namespace xcore
{

	//------------------------------------------------------------------------------------------
	//---------------------------------- PS3 IO Functions ------------------------------------
	//------------------------------------------------------------------------------------------
	namespace xfilesystem
	{
		class xfiledevice_ps3system : public xfiledevice
		{
		public:
			virtual void	HandleAsync(AsyncIOInfo* pAsync, FileInfo* pInfo);

			virtual bool	OpenOrCreateFile(FileInfo* pInfo);
			virtual bool	LengthOfFile(FileInfo* pInfo, u64& outLength);
			virtual bool	CloseFile(FileInfo* pInfo);
			virtual bool	DeleteFile(FileInfo* pInfo);
			virtual bool	ReadFile(FileInfo* pInfo, void* buffer, u64 count, u64& outNumBytesRead);
			virtual bool	WriteFile(FileInfo* pInfo, const void* buffer, u64 count, u64& outNumBytesWritten);

			virtual bool	SeekOrigin(FileInfo* pInfo, u64 pos, u64& newPos);
			virtual bool	SeekCurrent(FileInfo* pInfo, u64 pos, u64& newPos);
			virtual bool	SeekEnd(FileInfo* pInfo, u64 pos, u64& newPos);

			virtual bool	Sync(FileInfo* pInfo);

			enum EPs3SeekMode
			{
				__SEEK_ORIGIN = CELL_FS_SEEK_SET,
				__SEEK_CURRENT = CELL_FS_SEEK_CUR,
				__SEEK_END = CELL_FS_SEEK_END,
			};
			virtual bool	Seek(FileInfo* pInfo, EPs3SeekMode mode, u64 pos, u64& newPos);

			virtual bool	GetBlockSize(FileInfo* pInfo, u64& outSectorSize);
		};

		void xfiledevice_ps3system::HandleAsync(AsyncIOInfo* pAsync, FileInfo* pxInfo)
		{
			if(pAsync==NULL || pxInfo==NULL)
				return;

			if(pAsync->m_nStatus == FILE_OP_STATUS_OPEN_PENDING)
			{
				pAsync->m_nStatus = FILE_OP_STATUS_OPENING;

				bool boError = false;
				bool boSuccess = OpenOrCreateFile(pxInfo);
				if (!boSuccess)
				{
					x_printf ("__OpenOrCreateFile failed on file %s\n", x_va_list(pxInfo->m_szFilename));
					boError = true;
				}
				else
				{
					u64 nPos;
					boSuccess = Seek(pxInfo, __SEEK_END, 0, nPos);
					if ((!boSuccess) || ((pxInfo->m_boWriting == false) && (nPos == 0)) )
					{
						x_printf ("__Seek failed on file %s\n", x_va_list(pxInfo->m_szFilename));
						boError = true;
					}
					else
					{
						u64 uSize = nPos; 
						boSuccess = Seek(pxInfo, __SEEK_ORIGIN, 0, nPos);
						if (!boSuccess)
						{
							x_printf ("__Seek failed on file %s\n", x_va_list(pxInfo->m_szFilename));
							boError = true;
						}
						else
						{
							u64	uSectorSize = 4096;
							boSuccess = GetBlockSize(pxInfo, uSectorSize);
							if (boSuccess)
							{
								u64 uPad = uSize % uSectorSize;
								if (uPad != 0)
								{
									uPad = uSectorSize - uPad;
								}

								u32 uRoundedSize			= (u32)(uSize + uPad);
								u32 uNumSectors 			= (u32)(uRoundedSize / uSectorSize);

								pxInfo->m_uByteOffset		= 0;
								pxInfo->m_uByteSize			= uSize;
								pxInfo->m_uSectorOffset		= 0;
								pxInfo->m_uNumSectors		= uNumSectors;
								pxInfo->m_uSectorSize		= uSectorSize;
							}
						}
					}
				}
				if (boError)
				{
					if (pxInfo->m_nFileHandle != (u32)INVALID_FILE_HANDLE)
					{
						CloseFile(pxInfo);
					}
				}

				pAsync->m_nStatus	= FILE_OP_STATUS_DONE;
			}
			else if(pAsync->m_nStatus == FILE_OP_STATUS_CLOSE_PENDING)
			{
				pAsync->m_nStatus	= FILE_OP_STATUS_CLOSING;

				bool boClose = CloseFile(pxInfo);
				if (!boClose)
				{
					x_printf ("CloseFile failed on file %s\n", x_va_list(pxInfo->m_szFilename));
				}
								
				pxInfo->m_nFileHandle	= (u32)INVALID_FILE_HANDLE;
				pAsync->m_nStatus	= FILE_OP_STATUS_DONE;
			}
			else if(pAsync->m_nStatus == FILE_OP_STATUS_DELETE_PENDING)
			{
				pAsync->m_nStatus	= FILE_OP_STATUS_DELETING;

				bool boClose = CloseFile(pxInfo);
				if (!boClose)
				{
					x_printf ("CloseFile failed on file %s\n", x_va_list(pxInfo->m_szFilename));
				}
				else
				{
					bool boDelete = DeleteFile(pxInfo);
					if (!boDelete)
					{
						x_printf ("DeleteFile failed on file %s\n", x_va_list(pxInfo->m_szFilename));
					}
				}

				pxInfo->m_nFileHandle	= (u32)INVALID_FILE_HANDLE;
				pAsync->m_nStatus	= FILE_OP_STATUS_DONE;
			}
			else if(pAsync->m_nStatus == FILE_OP_STATUS_STAT_PENDING)
			{
				pAsync->m_nStatus	= FILE_OP_STATUS_STATING;

				// Stats(pxInfo);

				pAsync->m_nStatus	= FILE_OP_STATUS_DONE;
			}
			else if(pAsync->m_nStatus == FILE_OP_STATUS_READ_PENDING)
			{
				u64	nPos;
				bool boSeek = Seek(pxInfo, __SEEK_ORIGIN, pAsync->m_uReadWriteOffset, nPos);
				if (!boSeek)
				{
					x_printf ("Seek failed on file %s\n", x_va_list(pxInfo->m_szFilename));
				}

				pAsync->m_nStatus	= FILE_OP_STATUS_READING;

				u64 nReadSize;
				bool boRead = ReadFile(pxInfo, pAsync->m_pReadAddress, (u32)pAsync->m_uReadWriteSize, nReadSize);
				if (!boRead)
				{
					x_printf ("ReadFile failed on file %s\n", x_va_list(pxInfo->m_szFilename));
				}

				pAsync->m_nStatus	= FILE_OP_STATUS_DONE;
			}
			else if(pAsync->m_nStatus == FILE_OP_STATUS_WRITE_PENDING)
			{
				u64	nPos;
				bool boSeek = Seek(pxInfo, __SEEK_ORIGIN, pAsync->m_uReadWriteOffset, nPos);
				if (!boSeek)
				{
					x_printf ("Seek failed on file %s\n", x_va_list(pxInfo->m_szFilename));
				}

				pAsync->m_nStatus	= FILE_OP_STATUS_WRITING;

				u64 nWriteSize;
				bool boWrite = WriteFile(pxInfo, pAsync->m_pWriteAddress, (u32)pAsync->m_uReadWriteSize, nWriteSize);
				if (!boWrite)
				{
					x_printf ("WriteFile failed on file %s\n", x_va_list(pxInfo->m_szFilename));
				}

				if (pxInfo->m_eSource != FS_SOURCE_HOST)
				{
					bool boSyncResult = Sync(pxInfo);
					if (!boSyncResult)
					{
						x_printf ("Sync failed on file %s\n", x_va_list(pxInfo->m_szFilename));
					}
				}

				pAsync->m_nStatus	= FILE_OP_STATUS_DONE;
			}
		}

		bool xfiledevice_ps3system::OpenOrCreateFile(FileInfo* pInfo)
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

		bool xfiledevice_ps3system::LengthOfFile(FileInfo* pInfo, u64& outLength)
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

		bool xfiledevice_ps3system::CloseFile(FileInfo* pInfo)
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

		bool xfiledevice_ps3system::DeleteFile(FileInfo* pInfo)
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

		bool xfiledevice_ps3system::ReadFile(FileInfo* pInfo, void* buffer, uintfs count, u64& outNumBytesRead)
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
		bool xfiledevice_ps3system::WriteFile(FileInfo* pInfo, const void* buffer, uintfs count, u64& outNumBytesWritten)
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

		bool xfiledevice_ps3system::Seek(FileInfo* pInfo, EPs3SeekMode mode, u64 pos, u64& newPos)
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
		bool	xfiledevice_ps3system::SeekOrigin(FileInfo* pInfo, u64 pos, u64& newPos)
		{
			return Seek(pInfo, __SEEK_ORIGIN, pos, newPos);
		}

		bool	xfiledevice_ps3system::SeekCurrent(FileInfo* pInfo, u64 pos, u64& newPos)
		{
			return Seek(pInfo, __SEEK_CURRENT, pos, newPos);
		}

		bool	xfiledevice_ps3system::SeekEnd(FileInfo* pInfo, u64 pos, u64& newPos)
		{
			return Seek(pInfo, __SEEK_END, pos, newPos);
		}

		bool xfiledevice_ps3system::Sync(FileInfo* pInfo)
		{
			s32 nResult = cellFsFsync(pInfo->m_nFileHandle);
			return nResult == CELL_OK;
		}

		bool xfiledevice_ps3system::GetBlockSize(FileInfo* pInfo, u64& outSectorSize)
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
	};

	//==============================================================================
	// END xCore namespace
	//==============================================================================
};

#endif // TARGET_PS3