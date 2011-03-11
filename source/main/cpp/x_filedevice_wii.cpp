#include "xbase\x_target.h"
#ifdef TARGET_WII

//==============================================================================
// INCLUDES
//==============================================================================
#include <revolution.h>
#include <revolution\hio2.h>
#include <revolution\nand.h>
#include <revolution\dvd.h>

#include "xbase\x_debug.h"
#include "xbase\x_limits.h"
#include "xbase\x_string_std.h"
#include "xbase\x_va_list.h"

#include "xfilesystem\private\x_filesystem_common.h"
#include "xfilesystem\private\x_filesystem_wii.h"
#include "xfilesystem\private\x_filedevice.h"
#include "xfilesystem\private\x_fileinfo.h"
#include "xfilesystem\private\x_fileasync.h"

namespace xcore
{
	//------------------------------------------------------------------------------------------
	//---------------------------------- Nintendo WII IO Functions -----------------------------
	//------------------------------------------------------------------------------------------
	namespace xfilesystem
	{
		static DVDFileInfo		mDvdFileInfo[FS_MAX_OPENED_FILES];
		static NANDFileInfo		mNandFileInfo[FS_MAX_OPENED_FILES];

		static DVDFileInfo*		GetDvdFileInfo(u32 uHandle)						{ return &mDvdFileInfo[uHandle]; }
		static NANDFileInfo*	GetNandFileInfo(u32 uHandle)					{ return &mNandFileInfo[uHandle]; }


		static void HandleAsyncShared(xfiledevice* device, xfileasync* pAsync, xfileinfo* pInfo)
		{
			if(pAsync==NULL || pInfo==NULL)
				return;

			if(pAsync->getStatus() == FILE_OP_STATUS_OPEN_PENDING)
			{
				pAsync->m_nStatus = FILE_OP_STATUS_OPENING;

				bool boError   = false;
				bool boSuccess = device->OpenOrCreateFile(pInfo);
				if (!boSuccess)
				{
					x_printf ("OpenOrCreateFile failed on file %s\n", x_va_list(pInfo->m_szFilename));
					boError = true;
				}
				else
				{
					u64 uSize; 
					boSuccess = device->LengthOfFile(pInfo, uSize);
					if (!boSuccess)
					{
						x_printf ("LengthOfFile failed on file %s\n", x_va_list(pInfo->m_szFilename));
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
							pInfo->m_uSectorSize		= 0;		///< uSectorSize
							pInfo->m_uNumSectors		= uNumSectors;
						}
					}
				}

				if (boError)
				{
					if (pInfo->m_nFileHandle != (u32)INVALID_FILE_HANDLE)
					{
						bool boClose = device->CloseFile(pInfo);
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

				bool boClose = device->CloseFile(pInfo);
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

				bool boClose = device->CloseFile(pInfo);
				if (!boClose)
				{
					x_printf ("CloseFile failed on file %s\n", x_va_list(pInfo->m_szFilename));
				}
				else
				{
					bool boDelete = device->DeleteFile(pInfo);
					if (!boDelete)
					{
						x_printf ("DeleteFile failed on file %s\n", x_va_list(pInfo->m_szFilename));
					}
				}

				pInfo->m_nFileHandle	= (u32)INVALID_FILE_HANDLE;
				pAsync->m_nStatus	    = FILE_OP_STATUS_DONE;
			}
			else if(pAsync->getStatus() == FILE_OP_STATUS_STAT_PENDING)
			{
				pAsync->m_nStatus	= FILE_OP_STATUS_STATING;

				//@TODO: use stats

				pAsync->m_nStatus	= FILE_OP_STATUS_DONE;
			}
			else if(pAsync->getStatus() == FILE_OP_STATUS_READ_PENDING)
			{
				/* WII read function already accepts file offset
				u64	nPos;
				bool boSeek = device->Seek(pInfo, __SEEK_ORIGIN, pAsync->m_uReadWriteOffset, nPos);
				if (!boSeek)
				{
					x_printf ("Seek failed on file %s\n", x_va_list(pInfo->m_szFilename));
				}
				*/
				pAsync->m_nStatus	= FILE_OP_STATUS_READING;

				u64 nReadSize;
				bool boRead = device->ReadFile(pInfo, pAsync->m_pReadAddress, (u32)pAsync->m_uReadWriteSize, pAsync->m_uReadWriteOffset, nReadSize);
				if (!boRead)
				{
					x_printf ("device->ReadFile failed on file %s\n", x_va_list(pInfo->m_szFilename));
				}

				pAsync->m_nStatus	= FILE_OP_STATUS_DONE;
			}
			else if(pAsync->getStatus() == FILE_OP_STATUS_WRITE_PENDING)
			{
				u64	nPos;
				bool boSeek = device->Seek(pInfo, __SEEK_ORIGIN, pAsync->m_uReadWriteOffset, nPos);
				if (!boSeek)
				{
					x_printf ("Seek failed on file %s\n", x_va_list(pInfo->m_szFilename));
				}

				pAsync->m_nStatus	= FILE_OP_STATUS_WRITING;

				u64 nWriteSize;
				bool boWrite = device->WriteFile(pInfo, pAsync->m_pWriteAddress, (u32)pAsync->m_uReadWriteSize, nWriteSize);
				if (!boWrite)
				{
					x_printf ("device->WriteFile failed on file %s\n", x_va_list(pInfo->m_szFilename));
				}

				pAsync->m_nStatus	= FILE_OP_STATUS_DONE;
			}			
		}


		//------------------------------------------------------------------------------------------
		//----------------------------------     DVD            ------------------------------------
		//------------------------------------------------------------------------------------------

		class FileDevice_WII_DVD : public xfiledevice
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

			enum EWiiSeekMode
			{
				__SEEK_ORIGIN = 1,
				__SEEK_CURRENT = 2,
				__SEEK_END = 3,
			};
			virtual bool			Seek(xfileinfo* pInfo, EWiiSeekMode mode, u64 pos, u64& newPos);
			virtual bool			GetBlockSize(xfileinfo* pInfo, u64& outSectorSize);

			void*					operator new (size_t size, void *p)					{ return p; }
			void					operator delete(void* mem, void* )					{ }	
		};

		xfiledevice*		x_CreateFileDeviceWII(EDeviceType type)
		{
			void* mem = heapAlloc(sizeof(FileDevice_WII_System), 16);
			FileDevice_WII_System* file_device = new (mem) FileDevice_WII_System();
			file_device->SetType(type);
			file_device->Init();
			return file_device;
		}

		void			x_DestroyFileDeviceWII(xfiledevice* device)
		{
			FileDevice_WII_System* file_device = (FileDevice_WII_System*)device;
			file_device->Exit();
			file_device->~FileDevice_WII_System();
			heapFree(file_device);
		}

		bool FileDevice_WII_DVD::Init()
		{
			return true;
		}

		bool FileDevice_WII_DVD::Exit()
		{
			return true;
		}

		void FileDevice_WII_DVD::HandleAsync(xfileasync* pAsync, xfileinfo* pInfo)
		{
			xfiledevice* device = pInfo->m_pFileDevice;
			HandleAsyncShared(device, pAsync, pInfo);
		}

		bool FileDevice_WII_DVD::OpenOrCreateFile(xfileinfo* pInfo)
		{
			DVDFileInfo* dvdFileInfo = GetDvdFileInfo(pInfo->m_nFileIndex);
			pInfo->m_nFileHandle = (u32)dvdFileInfo;
			xbool boSuccess = DVDOpen(pInfo->m_szFilename, dvdFileInfo);
			return boSuccess;
		}

		bool FileDevice_WII_DVD::LengthOfFile(xfileinfo* pInfo, u64& outLength)
		{
			DVDFileInfo* dvdFileInfo = GetDvdFileInfo(pInfo->m_nFileIndex);
			u64 l = (u64)DVDGetLength(dvdFileInfo);
			l = (l + 31) & 0xffffffe0;											///< DVDRead requires size to be a multiple of 32 so we better return a file size that fulfills this requirement
			outLength = l;
			return true;
		}

		bool FileDevice_WII_DVD::CloseFile(xfileinfo* pInfo)
		{
			DVDFileInfo* dvdFileInfo = GetDvdFileInfo(pInfo->m_nFileIndex);
			xbool boSuccess = DVDClose(dvdFileInfo);
			pInfo->m_nFileHandle = INVALID_FILE_HANDLE;
			return boSuccess;
		}

		bool FileDevice_WII_DVD::DeleteFile(xfileinfo* pInfo)
		{
			return false;
		}

		bool FileDevice_WII_DVD::ReadFile(xfileinfo* pInfo, void* buffer, uintfs count, u64& outNumBytesRead)
		{
			DVDFileInfo* dvdFileInfo = GetDvdFileInfo(pInfo->m_nFileIndex);
			size = (size + 31) & 0xffffffe0;									///< DVDRead requires size to be a multiple of 32
			s32 numBytesRead = DVDRead(dvdFileInfo, buffer, size, pInfo->);
			bool boSuccess = numBytesRead >= 0;
			if (boSuccess)
			{
				outNumBytesRead = numBytesRead;
			}
			return boSuccess;
		}
		bool FileDevice_WII_DVD::WriteFile(xfileinfo* pInfo, const void* buffer, uintfs count, u64& outNumBytesWritten)
		{
			return false;
		}

		bool FileDevice_WII_DVD::Seek(xfileinfo* pInfo, EWiiSeekMode mode, u64 pos, u64& newPos)
		{
			DVDFileInfo* dvdFileInfo = GetDvdFileInfo(pInfo->m_nFileIndex);
			if (mode == __SEEK_END)
				pos = (u64)DVDGetLength(dvdFileInfo) - pos;
			pos = (pos + 3) & 0xfffffffc;
			s32 nResult = DVDSeek(dvdFileInfo, (s32)pos);
			if (nResult==0)
				newPos = pos;
			return nResult==0;
		}
		bool	FileDevice_WII_DVD::SeekOrigin(xfileinfo* pInfo, u64 pos, u64& newPos)
		{
			return Seek(pInfo, __SEEK_ORIGIN, pos, newPos);
		}

		bool	FileDevice_WII_DVD::SeekCurrent(xfileinfo* pInfo, u64 pos, u64& newPos)
		{
			return Seek(pInfo, __SEEK_CURRENT, pos, newPos);
		}

		bool	FileDevice_WII_DVD::SeekEnd(xfileinfo* pInfo, u64 pos, u64& newPos)
		{
			return Seek(pInfo, __SEEK_END, pos, newPos);
		}

		bool FileDevice_WII_DVD::Sync(xfileinfo* pInfo)
		{
			return true;
		}

		bool FileDevice_WII_DVD::GetBlockSize(xfileinfo* pInfo, u64& outSectorSize)
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



		//------------------------------------------------------------------------------------------
		//----------------------------------     NAND           ------------------------------------
		//------------------------------------------------------------------------------------------

		class FileDevice_WII_NAND : public xfiledevice
		{
		public:
			virtual bool	Init();

			virtual void	HandleAsync(xfileasync* pAsync, xfileinfo* pInfo);

			virtual bool	OpenOrCreateFile(xfileinfo* pInfo);
			virtual bool	LengthOfFile(xfileinfo* pInfo, u64& outLength);
			virtual bool	CloseFile(xfileinfo* pInfo);
			virtual bool	DeleteFile(xfileinfo* pInfo);
			virtual bool	ReadFile(xfileinfo* pInfo, void* buffer, u64 count, u64& outNumBytesRead);
			virtual bool	WriteFile(xfileinfo* pInfo, const void* buffer, u64 count, u64& outNumBytesWritten);

			virtual bool	SeekOrigin(xfileinfo* pInfo, u64 pos, u64& newPos);
			virtual bool	SeekCurrent(xfileinfo* pInfo, u64 pos, u64& newPos);
			virtual bool	SeekEnd(xfileinfo* pInfo, u64 pos, u64& newPos);

			virtual bool	Sync(xfileinfo* pInfo);

			enum EWiiSeekMode
			{
				__SEEK_ORIGIN = CELL_FS_SEEK_SET,
				__SEEK_CURRENT = CELL_FS_SEEK_CUR,
				__SEEK_END = CELL_FS_SEEK_END,
			};
			virtual bool	Seek(xfileinfo* pInfo, EWiiSeekMode mode, u64 pos, u64& newPos);

			virtual bool	GetBlockSize(xfileinfo* pInfo, u64& outSectorSize);
		};

		static GXColor sErrorPageBGColor = {   0,   0,   0, 0 };
		static GXColor sErrorPageFGColor = { 255, 255, 255, 0 };

		static void __CheckNANDError(s32 errorCode)
		{
			switch (errorCode)
			{
			case NAND_RESULT_UNKNOWN:
				OSFatal(sErrorPageFGColor, sErrorPageBGColor, "NAND_RESULT_UNKNOWN");
				break;
			case NAND_RESULT_BUSY:
			case NAND_RESULT_ALLOC_FAILED:
				OSFatal(sErrorPageFGColor, sErrorPageBGColor, "NAND_RESULT_BUSY || NAND_RESULT_ALLOC_FAILED");
				break;
			case NAND_RESULT_CORRUPT:
				OSFatal(sErrorPageFGColor, sErrorPageBGColor, "NAND_RESULT_CORRUPT");
				break;
			}
		}

		bool FileDevice_WII_NAND::Init()
		{
			return (NANDInit() == NAND_RESULT_OK);
		}

		void FileDevice_WII_NAND::HandleAsync(xfileasync* pAsync, xfileinfo* pInfo)
		{
			HandleAsyncShared(pAsync, pInfo);
		}

		bool FileDevice_WII_NAND::OpenOrCreateFile(xfileinfo* pInfo)
		{
			NANDFileInfo* fileInfo = GetNandFileInfo(pInfo->m_nFileIndex);
			pInfo->m_nFileHandle = (u32)fileInfo;

			u8 accType = pInfo->m_boWriting ? NAND_ACCESS_RW : NAND_ACCESS_READ;

			s32 result = NANDOpen(pInfo->m_szFilename, fileInfo, accType);
			__CheckNANDError(result);
			
			if (result == NAND_RESULT_NOEXISTS && pInfo->m_boWriting)
			{
				result = NANDCreate(pInfo->m_szFilename, NAND_PERM_OWNER_MASK, accType);
				__CheckNANDError(result);

				if (result == NAND_RESULT_OK)
				{
					result = NANDOpen(pInfo->m_szFilename, fileInfo, accType);
					__CheckNANDError(result);
				}
			}

			return (result == NAND_RESULT_OK);
		}

		bool FileDevice_WII_NAND::LengthOfFile(xfileinfo* pInfo, u64& outLength)
		{
			NANDFileInfo* fileInfo = GetNandFileInfo(pInfo->m_nFileIndex);

			unsigned long length = 0;
			s32 result = NANDGetLength(fileInfo, &length);
			__CheckNANDError(result);

			outLength = (u64)length;

			return (result == NAND_RESULT_OK);
		}

		bool FileDevice_WII_NAND::CloseFile(xfileinfo* pInfo)
		{
			NANDFileInfo* fileInfo = GetNandFileInfo(pInfo->m_nFileIndex);

			s32 result = NANDClose(fileInfo);
			__CheckNANDError(result);

			pInfo->m_nFileHandle = INVALID_FILE_HANDLE;

			return (result == NAND_RESULT_OK);
		}

		bool FileDevice_WII_NAND::DeleteFile(xfileinfo* pInfo)
		{
			s32 result = NANDDelete(pInfo->m_szFilename);
			__CheckNANDError(result);

			return (result == NAND_RESULT_OK);
		}

		bool FileDevice_WII_NAND::ReadFile(xfileinfo* pInfo, void* buffer, uintfs count, u64& outNumBytesRead)
		{
			NANDFileInfo* fileInfo = GetNandFileInfo(pInfo->m_nFileIndex);

			outNumBytesRead = 0;

			// make sure size is multiple of 32
			if (size % 32 == 0)
			{
				s32 result = NANDRead(fileInfo, buffer, (u32)size);
				__CheckNANDError(result);

				if (result >= 0)
					outNumBytesRead = (u64)result;
			}

			return (outNumBytesRead != 0);
		}
		bool FileDevice_WII_NAND::WriteFile(xfileinfo* pInfo, const void* buffer, uintfs count, u64& outNumBytesWritten)
		{
			NANDFileInfo* fileInfo = GetNandFileInfo(pInfo->m_nFileIndex);

			outNumBytesWritten = 0;

			// make sure size is multiple of 32
			if (size % 32 == 0)
			{
				s32 result = NANDWrite(fileInfo, buffer, (u32)size);
				__CheckNANDError(result);

				if (result >= 0)
					outNumBytesWritten = (u64)result;
			}

			return (outNumBytesWritten != 0);
		}

		bool FileDevice_WII_NAND::Seek(xfileinfo* pInfo, EWiiSeekMode mode, u64 pos, u64& newPos)
		{
			NANDFileInfo* fileInfo = GetNandFileInfo(pInfo->m_nFileIndex);

			s32 whence = NAND_SEEK_CUR;

			if (mode == __SEEK_ORIGIN)
				whence = NAND_SEEK_SET;
			else if (mode == __SEEK_END)
				whence = NAND_SEEK_END;
			else
				whence = NAND_SEEK_CUR;

			s32 result = NANDSeek(fileInfo, (s32)pos, whence);
			__CheckNANDError(result);

			if (result >= 0)
				newPos = result;
			else
				newPos = pos;

			return (result >= 0);
		}
		bool	FileDevice_WII_NAND::SeekOrigin(xfileinfo* pInfo, u64 pos, u64& newPos)
		{
			return Seek(pInfo, __SEEK_ORIGIN, pos, newPos);
		}

		bool	FileDevice_WII_NAND::SeekCurrent(xfileinfo* pInfo, u64 pos, u64& newPos)
		{
			return Seek(pInfo, __SEEK_CURRENT, pos, newPos);
		}

		bool	FileDevice_WII_NAND::SeekEnd(xfileinfo* pInfo, u64 pos, u64& newPos)
		{
			return Seek(pInfo, __SEEK_END, pos, newPos);
		}

		bool FileDevice_WII_NAND::Sync(xfileinfo* pInfo)
		{
			return true;
		}

		bool FileDevice_WII_NAND::GetBlockSize(xfileinfo* pInfo, u64& outSectorSize)
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

#endif // TARGET_WII
