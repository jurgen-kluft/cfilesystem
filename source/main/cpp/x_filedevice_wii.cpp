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

#include "xtime\x_datetime.h"

#include "xfilesystem\private\x_filesystem_common.h"
#include "xfilesystem\private\x_filesystem_wii.h"
#include "xfilesystem\x_filedevice.h"
#include "xfilesystem\private\x_filedata.h"
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

		//------------------------------------------------------------------------------------------
		//----------------------------------     DVD            ------------------------------------
		//------------------------------------------------------------------------------------------

		class FileDevice_WII_DVD : public xfiledevice
		{
		public:
			virtual					~FileDevice_WII_DVD()								{ }

			virtual bool			canSeek() const										{ return true; }
			virtual bool			canWrite() const									{ return false; }

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

		xfiledevice*		x_CreateFileDeviceWII()
		{
			FileDevice_WII_System* file_device = new FileDevice_WII_System();
			return file_device;
		}

		void			x_DestroyFileDeviceWII(xfiledevice* device)
		{
			FileDevice_WII_System* file_device = (FileDevice_WII_System*)device;
			delete file_device;
		}

		bool FileDevice_WII_DVD::openOrCreateFile(u32 nFileIndex, const char* szFilename, bool boRead, bool boWrite, u32& nFileHandle)
		{
			DVDFileInfo* dvdFileInfo = GetDvdFileInfo(nFileIndex);
			nFileHandle = reinterpret_cast<u32>(dvdFileInfo);
			xbool boSuccess = DVDOpen(szFilename, dvdFileInfo);
			return boSuccess;
		}

		bool FileDevice_WII_DVD::lengthOfFile(u32 nFileHandle, u64& outLength) const
		{
			DVDFileInfo* dvdFileInfo = reinterpret_cast<DVDFileInfo*>(nFileHandle);
			u64 l = (u64)DVDGetLength(dvdFileInfo);
			l = (l + 31) & 0xffffffe0;											///< DVDRead requires size to be a multiple of 32 so we better return a file size that fulfills this requirement
			outLength = l;
			return true;
		}

		bool FileDevice_WII_DVD::closeFile(u32 nFileHandle)
		{
			DVDFileInfo* dvdFileInfo = reinterpret_cast<DVDFileInfo*>(nFileHandle);
			xbool boSuccess = DVDClose(dvdFileInfo);
			nFileHandle = INVALID_FILE_HANDLE;
			return boSuccess;
		}

		bool FileDevice_WII_DVD::deleteFile(u32 nFileHandle, const char* szFilename)
		{
			DVDFileInfo* dvdFileInfo = reinterpret_cast<DVDFileInfo*>(nFileHandle);
			return false;
		}

		bool FileDevice_WII_DVD::readFile(u32 nFileHandle, void* buffer, u64 pos, u64 count, u64& outNumBytesRead)
		{
			DVDFileInfo* dvdFileInfo = reinterpret_cast<DVDFileInfo*>(nFileHandle);
			count = (count + 31) & 0xffffffe0;									///< DVDRead requires count to be a multiple of 32
			s32 numBytesRead = DVDRead(dvdFileInfo, buffer, count, (s32)pos);
			bool boSuccess = numBytesRead >= 0;
			if (boSuccess)
			{
				outNumBytesRead = numBytesRead;
			}
			return boSuccess;
		}

		bool FileDevice_WII_DVD::writeFile(u32 nFileHandle, const void* buffer, u64 pos, u64 count, u64& outNumBytesWritten)
		{
			DVDFileInfo* dvdFileInfo = reinterpret_cast<DVDFileInfo*>(nFileHandle);
			// Unable to write
			return false;
		}

		bool FileDevice_WII_DVD::seek(u32 nFileHandle, ESeekMode mode, u64 pos, u64& newPos)
		{
			DVDFileInfo* dvdFileInfo = reinterpret_cast<DVDFileInfo*>(nFileHandle);
			if (mode == __SEEK_END)
				pos = (u64)DVDGetLength(dvdFileInfo) - pos;
			pos = (pos + 3) & 0xfffffffc;
			s32 nResult = DVDSeek(dvdFileInfo, (s32)pos);
			if (nResult==0)
				newPos = pos;
			return nResult==0;
		}
		bool	FileDevice_WII_DVD::seekOrigin(u32 nFileHandle, u64 pos, u64& newPos)
		{
			return seek(pInfo, __SEEK_ORIGIN, pos, newPos);
		}

		bool	FileDevice_WII_DVD::seekCurrent(u32 nFileHandle, u64 pos, u64& newPos)
		{
			return seek(pInfo, __SEEK_CURRENT, pos, newPos);
		}

		bool	FileDevice_WII_DVD::seekEnd(u32 nFileHandle, u64 pos, u64& newPos)
		{
			return seek(pInfo, __SEEK_END, pos, newPos);
		}

		//------------------------------------------------------------------------------------------
		//----------------------------------     NAND           ------------------------------------
		//------------------------------------------------------------------------------------------

		class FileDevice_WII_NAND : public xfiledevice
		{
		public:
			virtual					~FileDevice_WII_NAND()								{ }

			virtual bool			canSeek() const										{ return true; }
			virtual bool			canWrite() const									{ return true; }

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

		xfiledevice*		x_CreateNandDeviceWII()
		{
			FileDevice_WII_NAND* file_device = new FileDevice_WII_NAND();
			file_device->init();
			return file_device;
		}

		void			x_DestroyNandDeviceWII(xfiledevice* device)
		{
			FileDevice_WII_NAND* file_device = (FileDevice_WII_NAND*)device;
			file_device->exit();
			delete file_device;
		}

		bool FileDevice_WII_NAND::init()
		{
			return (NANDInit() == NAND_RESULT_OK);
		}

		bool FileDevice_WII_NAND::exit()
		{
			return NANDExit();
		}

		bool FileDevice_WII_NAND::openOrCreateFile(u32 nFileHandle)
		{
			NANDFileInfo* fileInfo = GetNandFileInfo(pInfo->m_nFileIndex);
			nFileHandle = (u32)fileInfo;

			u8 accType = boWrite ? NAND_ACCESS_RW : NAND_ACCESS_READ;

			s32 result = NANDOpen(szFilename, fileInfo, accType);
			__CheckNANDError(result);
			
			if (result == NAND_RESULT_NOEXISTS && boWrite)
			{
				result = NANDCreate(szFilename, NAND_PERM_OWNER_MASK, accType);
				__CheckNANDError(result);

				if (result == NAND_RESULT_OK)
				{
					result = NANDOpen(szFilename, fileInfo, accType);
					__CheckNANDError(result);
				}
			}

			return (result == NAND_RESULT_OK);
		}

		bool FileDevice_WII_NAND::lengthOfFile(u32 nFileHandle, u64& outLength) const
		{
			NANDFileInfo* fileInfo = reinterpret_cast<NANDFileInfo*>(nFileHandle);

			unsigned long length = 0;
			s32 result = NANDGetLength(fileInfo, &length);
			__CheckNANDError(result);

			outLength = (u64)length;

			return (result == NAND_RESULT_OK);
		}

		bool FileDevice_WII_NAND::closeFile(u32 nFileHandle)
		{
			NANDFileInfo* fileInfo = reinterpret_cast<NANDFileInfo*>(nFileHandle);

			s32 result = NANDClose(fileInfo);
			__CheckNANDError(result);

			nFileHandle = INVALID_FILE_HANDLE;

			return (result == NAND_RESULT_OK);
		}

		bool FileDevice_WII_NAND::deleteFile(u32 nFileHandle, const char* szFilename)
		{
			s32 result = NANDDelete(szFilename);
			__CheckNANDError(result);

			return (result == NAND_RESULT_OK);
		}

		bool FileDevice_WII_NAND::readFile(u32 nFileHandle, void* buffer, u64 pos, u64 count, u64& outNumBytesRead)
		{
			NANDFileInfo* fileInfo = reinterpret_cast<NANDFileInfo*>(nFileHandle);

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
		bool FileDevice_WII_NAND::writeFile(u32 nFileHandle, const void* buffer, u64 pos, u64 count, u64& outNumBytesWritten)
		{
			NANDFileInfo* fileInfo = reinterpret_cast<NANDFileInfo*>(nFileHandle);

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

		bool FileDevice_WII_NAND::seek(u32 nFileHandle, EWiiSeekMode mode, u64 pos, u64& newPos)
		{
			NANDFileInfo* fileInfo = reinterpret_cast<NANDFileInfo*>(nFileHandle);

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
		bool	FileDevice_WII_NAND::seekOrigin(u32 nFileHandle, u64 pos, u64& newPos)
		{
			return seek(pInfo, __SEEK_ORIGIN, pos, newPos);
		}

		bool	FileDevice_WII_NAND::seekCurrent(u32 nFileHandle, u64 pos, u64& newPos)
		{
			return seek(pInfo, __SEEK_CURRENT, pos, newPos);
		}

		bool	FileDevice_WII_NAND::seekEnd(u32 nFileHandle, u64 pos, u64& newPos)
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

#endif // TARGET_WII
