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

#include "xatomic\x_queue.h"

namespace xcore
{
	//------------------------------------------------------------------------------------------
	//---------------------------------- Nintendo WII IO Functions -----------------------------
	//------------------------------------------------------------------------------------------
	namespace xfilesystem
	{
		

		NANDFileInfo*		mNandFileInfo;
		

		NANDFileInfo*		GetNandFileInfo(u32 uHandle)				{ return &mNandFileInfo[uHandle]; }



		//------------------------------------------------------------------------------------------
		//----------------------------------     DVD            ------------------------------------
		//------------------------------------------------------------------------------------------

		class FileDevice_WII_DVD : public xfiledevice
		{

			DVDFileInfo*		mDvdFileInfo;
			u32					mNumOpenFiles;

			mutable xcore::atomic::queue<u32> mFreeFileinfoIndecies;
			//xcore::atomic::queue<u32> mUsedFileinfoIdecies;

		public:
			FileDevice_WII_DVD()								
			{
				mDvdFileInfo = new DVDFileInfo[xfs_common::s_instance()->getMaxOpenFiles()];
				mNumOpenFiles = 0;

				mFreeFileinfoIndecies.init(getAllocator(), xfs_common::s_instance()->getMaxOpenFiles());
				//mUsedFileinfoIdecies.init(getAllocator(), xfs_common::s_instance()->getMaxOpenFiles());

				for(s32 i = 0; i < xfs_common::s_instance()->getMaxOpenFiles(); i++)
				{
					u32 freeFileIndex = (u32)i;
					mFreeFileinfoIndecies.push(freeFileIndex);
				}
			}


			virtual					~FileDevice_WII_DVD()								
			{ 
				delete [] mDvdFileInfo;
			}

			virtual bool			canSeek() const										{ return true; }
			virtual bool			canWrite() const									{ return false; }

			virtual bool			getDeviceInfo(u64& totalSpace, u64& freeSpace) const;

			virtual bool			hasFile(const char* szFilename) const;
			virtual bool			openFile(const char* szFilename, bool boRead, bool boWrite, u32& nFileHandle) const;
			virtual bool			createFile(const char* szFilename, bool boRead, bool boWrite, u32& nFileHandle) const;
			virtual bool			readFile(u32 nFileHandle, u64 pos, void* buffer, u64 count, u64& outNumBytesRead) const;
			virtual bool			writeFile(u32 nFileHandle, u64 pos, const void* buffer, u64 count, u64& outNumBytesWritten) const;
			virtual bool			closeFile(u32 nFileHandle) const;
			virtual bool			moveFile(const char* szFilename, const char* szToFilename) const;
			virtual bool			copyFile(const char* szFilename, const char* szToFilename, bool boOverwrite) const;
			virtual bool			deleteFile(const char* szFilename) const;
			virtual bool			setFilePos(u32 nFileHandle, u64& ioPos) const;
			virtual bool			getFilePos(u32 nFileHandle, u64& outPos) const;
			virtual bool			setLengthOfFile(u32 nFileHandle, u64 inLength) const;
			virtual bool			getLengthOfFile(u32 nFileHandle, u64& outLength) const;
			virtual bool			setFileTime(const char* szFilename, const xdatetime& creationTime, const xdatetime& lastAccessTime, const xdatetime& lastWriteTime) const;
			virtual bool			getFileTime(const char* szFilename, xdatetime& outCreationTime, xdatetime& outLastAccessTime, xdatetime& outLastWriteTime) const;
			virtual bool			setFileAttr(const char* szFilename, const xattributes& attr) const;
			virtual bool			getFileAttr(const char* szFilename, xattributes& attr) const;

			virtual bool			hasDir(const char* szDirPath) const;
			virtual bool			createDir(const char* szDirPath) const;
			virtual bool			moveDir(const char* szDirPath, const char* szToDirPath) const;
			virtual bool			copyDir(const char* szDirPath, const char* szToDirPath, bool boOverwrite) const;
			virtual bool			deleteDir(const char* szDirPath) const;
			virtual bool			setDirTime(const char* szDirPath, const xdatetime& creationTime, const xdatetime& lastAccessTime, const xdatetime& lastWriteTime) const;
			virtual bool			getDirTime(const char* szDirPath, xdatetime& outCreationTime, xdatetime& outLastAccessTime, xdatetime& outLastWriteTime) const;
			virtual bool			setDirAttr(const char* szDirPath, const xattributes& attr) const;
			virtual bool			getDirAttr(const char* szDirPath, xattributes& attr) const;

			virtual bool			enumerate(const char* szDirPath, bool boSearchSubDirectories, enumerate_delegate<xfileinfo>* file_enumerator, enumerate_delegate<xdirinfo>* dir_enumerator, s32 depth = 0) const;

			enum ESeekMode { __SEEK_ORIGIN = 1, __SEEK_CURRENT = 2, __SEEK_END = 3, };
			bool					seek(u32 nFileHandle, ESeekMode mode, u64 pos, u64& newPos);
			bool					seekOrigin(u32 nFileHandle, u64 pos, u64& newPos);
			bool					seekCurrent(u32 nFileHandle, u64 pos, u64& newPos);
			bool					seekEnd(u32 nFileHandle, u64 pos, u64& newPos);



			DVDFileInfo*			GetDvdFileInfo(u32 uHandle)					{ return &mDvdFileInfo[uHandle]; }

			XFILESYSTEM_OBJECT_NEW_DELETE()
		};

		xfiledevice*		x_CreateFileDeviceWII()
		{
			FileDevice_WII_DVD* file_device = new FileDevice_WII_DVD();
			return file_device;
		}

		void			x_DestroyFileDeviceWII(xfiledevice* device)
		{
			FileDevice_WII_DVD* file_device = (FileDevice_WII_DVD*)device;
			delete file_device;
		}

		bool FileDevice_WII_DVD::openFile(const char* szFilename, bool boRead, bool boWrite, u32& nFileHandle) const
		{
			//u32 fileInfoIndex = 0;
			bool result = mFreeFileinfoIndecies.pop(nFileHandle);
			
			ASSERT(result);

			xbool boSuccess = DVDOpen(szFilename, &mDvdFileInfo[nFileHandle]);

			if(boSuccess)
			{
				//nFileHandle = fileInfoIndex;
			}

			else
			{
				nFileHandle =  INVALID_FILE_HANDLE;
				mFreeFileinfoIndecies.push(nFileHandle);
			}

			return boSuccess;
		}

		bool FileDevice_WII_DVD::createFile( const char* szFilename, bool boRead, bool boWrite, u32& nFileHandle) const
		{
			// You can't create a file on a dvd
			ASSERT(0);
			nFileHandle = INVALID_FILE_HANDLE;
			return false;
		}

		bool FileDevice_WII_DVD::getLengthOfFile(u32 nFileHandle, u64& outLength) const
		{
			u64 l = (u64)DVDGetLength(&mDvdFileInfo[nFileHandle]);
			l = (l + 31) & 0xffffffe0;											///< DVDRead requires size to be a multiple of 32 so we better return a file size that fulfills this requirement
			outLength = l;
			return true;
		}

		bool FileDevice_WII_DVD::closeFile(u32 nFileHandle) const
		{
			DVDFileInfo* dvdFileInfo = &mDvdFileInfo[nFileHandle];
			xbool boSuccess = DVDClose(dvdFileInfo);
			



			mFreeFileinfoIndecies.push(nFileHandle);
			
			nFileHandle = INVALID_FILE_HANDLE;



			return boSuccess;
		}

		bool FileDevice_WII_DVD::deleteFile(const char* szFilename) const
		{

			// cannot delete files from a dvd
			ASSERT(0);
			return false;
		}

		bool FileDevice_WII_DVD::readFile(u32 nFileHandle, u64 pos, void* buffer,  u64 count, u64& outNumBytesRead) const
		{
			DVDFileInfo* dvdFileInfo = &mDvdFileInfo[nFileHandle];
			count = (count + 31) & 0xffffffe0;									///< DVDRead requires count to be a multiple of 32
			s32 numBytesRead = DVDRead(dvdFileInfo, buffer, count, (s32)pos);
			bool boSuccess = numBytesRead >= 0;
			if (boSuccess)
			{
				outNumBytesRead = numBytesRead;
			}
			return boSuccess;
		}

		bool FileDevice_WII_DVD::writeFile(u32 nFileHandle, u64 pos, const void* buffer,  u64 count, u64& outNumBytesWritten) const
		{
			DVDFileInfo* dvdFileInfo = &mDvdFileInfo[nFileHandle];
			// Unable to write
			
			ASSERT(0);
			return false;
		}

		bool FileDevice_WII_DVD::seek(u32 nFileHandle, ESeekMode mode, u64 pos, u64& newPos)
		{
			DVDFileInfo* dvdFileInfo = &mDvdFileInfo[nFileHandle];
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
			return seek(nFileHandle, __SEEK_ORIGIN, pos, newPos);
		}

		bool	FileDevice_WII_DVD::seekCurrent(u32 nFileHandle, u64 pos, u64& newPos)
		{
			return seek(nFileHandle, __SEEK_CURRENT, pos, newPos);
		}

		bool	FileDevice_WII_DVD::seekEnd(u32 nFileHandle, u64 pos, u64& newPos)
		{
			return seek(nFileHandle, __SEEK_END, pos, newPos);
		}

		//------------------------------------------------------------------------------------------
		//----------------------------------     NAND           ------------------------------------
		//------------------------------------------------------------------------------------------

		class FileDevice_WII_NAND : public xfiledevice
		{
			NANDFileInfo*		mNandFileInfo;
			u32					mNumOpenFiles;

			mutable xcore::atomic::queue<u32> mFreeFileinfoIndecies;
			//xcore::atomic::queue<u32> mUsedFileinfoIdecies;
		public:

			FileDevice_WII_NAND()
			{
				mFreeFileinfoIndecies.init(getAllocator(), xfs_common::s_instance()->getMaxOpenFiles());
				mNandFileInfo = new NANDFileInfo[xfs_common::s_instance()->getMaxOpenFiles()];
				mNumOpenFiles = 0;

				for(s32 i = 0; i < xfs_common::s_instance()->getMaxOpenFiles(); i++)
				{
					u32 freeFileIndex = (u32)i;
					mFreeFileinfoIndecies.push(freeFileIndex);
				}

			}
			virtual					~FileDevice_WII_NAND()								
			{
				delete [] mNandFileInfo;
			}

			virtual bool			canSeek() const										{ return true; }
			virtual bool			canWrite() const									{ return true; }


			virtual bool			getDeviceInfo(u64& totalSpace, u64& freeSpace) const;

			virtual bool			hasFile(const char* szFilename) const;
			virtual bool			openFile(const char* szFilename, bool boRead, bool boWrite, u32& nFileHandle) const;
			virtual bool			createFile(const char* szFilename, bool boRead, bool boWrite, u32& nFileHandle) const;
			virtual bool			readFile(u32 nFileHandle, u64 pos, void* buffer, u64 count, u64& outNumBytesRead) const;
			virtual bool			writeFile(u32 nFileHandle, u64 pos, const void* buffer, u64 count, u64& outNumBytesWritten) const;
			virtual bool			closeFile(u32 nFileHandle) const;
			virtual bool			moveFile(const char* szFilename, const char* szToFilename) const;
			virtual bool			copyFile(const char* szFilename, const char* szToFilename, bool boOverwrite) const;
			virtual bool			deleteFile(const char* szFilename) const;
			virtual bool			setFilePos(u32 nFileHandle, u64& ioPos) const;
			virtual bool			getFilePos(u32 nFileHandle, u64& outPos) const;
			virtual bool			setLengthOfFile(u32 nFileHandle, u64 inLength) const;
			virtual bool			getLengthOfFile(u32 nFileHandle, u64& outLength) const;
			virtual bool			setFileTime(const char* szFilename, const xdatetime& creationTime, const xdatetime& lastAccessTime, const xdatetime& lastWriteTime) const;
			virtual bool			getFileTime(const char* szFilename, xdatetime& outCreationTime, xdatetime& outLastAccessTime, xdatetime& outLastWriteTime) const;
			virtual bool			setFileAttr(const char* szFilename, const xattributes& attr) const;
			virtual bool			getFileAttr(const char* szFilename, xattributes& attr) const;

			virtual bool			hasDir(const char* szDirPath) const;
			virtual bool			createDir(const char* szDirPath) const;
			virtual bool			moveDir(const char* szDirPath, const char* szToDirPath) const;
			virtual bool			copyDir(const char* szDirPath, const char* szToDirPath, bool boOverwrite) const;
			virtual bool			deleteDir(const char* szDirPath) const;
			virtual bool			setDirTime(const char* szDirPath, const xdatetime& creationTime, const xdatetime& lastAccessTime, const xdatetime& lastWriteTime) const;
			virtual bool			getDirTime(const char* szDirPath, xdatetime& outCreationTime, xdatetime& outLastAccessTime, xdatetime& outLastWriteTime) const;
			virtual bool			setDirAttr(const char* szDirPath, const xattributes& attr) const;
			virtual bool			getDirAttr(const char* szDirPath, xattributes& attr) const;

			virtual bool			enumerate(const char* szDirPath, bool boSearchSubDirectories, enumerate_delegate<xfileinfo>* file_enumerator, enumerate_delegate<xdirinfo>* dir_enumerator, s32 depth = 0) const;


			enum ESeekMode { __SEEK_ORIGIN = 1, __SEEK_CURRENT = 2, __SEEK_END = 3, };
			bool					seek(u32 nFileHandle, ESeekMode mode, u64 pos, u64& newPos);
			bool					seekOrigin(u32 nFileHandle, u64 pos, u64& newPos);
			bool					seekCurrent(u32 nFileHandle, u64 pos, u64& newPos);
			bool					seekEnd(u32 nFileHandle, u64 pos, u64& newPos);

			bool					init();
			bool					exit();

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
			return true; //NANDExit();
		}

		bool FileDevice_WII_NAND::openFile(const char* szFilename, bool boRead, bool boWrite, u32& nFileHandle) const
		{

			u32 freeFileIndex = 0;

			bool popresult = mFreeFileinfoIndecies.pop(freeFileIndex);

			ASSERT(popresult);

			NANDFileInfo *fileInfo = &mNandFileInfo[freeFileIndex];

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

			if(result == NAND_RESULT_OK)
			{
				nFileHandle = freeFileIndex;
			}
			else
			{
				nFileHandle = INVALID_FILE_HANDLE;
				mFreeFileinfoIndecies.push(freeFileIndex);
			}

			return (result == NAND_RESULT_OK);
		}

		bool FileDevice_WII_NAND::getLengthOfFile(u32 nFileHandle, u64& outLength) const
		{
			NANDFileInfo* fileInfo = &mNandFileInfo[nFileHandle];

			unsigned long length = 0;
			s32 result = NANDGetLength(fileInfo, &length);
			__CheckNANDError(result);

			outLength = (u64)length;

			return (result == NAND_RESULT_OK);
		}

		bool FileDevice_WII_NAND::closeFile(u32 nFileHandle) const
		{
			NANDFileInfo* fileInfo = &mNandFileInfo[nFileHandle];

			s32 result = NANDClose(fileInfo);
			__CheckNANDError(result);

			mFreeFileinfoIndecies.push(nFileHandle);

			nFileHandle = INVALID_FILE_HANDLE;

			return (result == NAND_RESULT_OK);
		}

		bool FileDevice_WII_NAND::deleteFile(const char* szFilename) const
		{
			s32 result = NANDDelete(szFilename);
			__CheckNANDError(result);

			return (result == NAND_RESULT_OK);
		}

		bool FileDevice_WII_NAND::readFile(u32 nFileHandle, u64 pos, void* buffer, u64 count, u64& outNumBytesRead) const
		{
			NANDFileInfo* fileInfo = &mNandFileInfo[nFileHandle];

			outNumBytesRead = 0;

			// make sure size is multiple of 32
			if (count % 32 == 0)
			{
				s32 result = NANDRead(fileInfo, buffer, (u32)count);
				__CheckNANDError(result);

				if (result >= 0)
					outNumBytesRead = (u64)result;
			}

			else
			{
				x_printf("ERROR: Reading from file %d on Wii NAND but size was %d not multiple of 32 (remainder=%d)\n", nFileHandle, count, count % 32);
			}

			return (outNumBytesRead != 0);
		}
		bool FileDevice_WII_NAND::writeFile(u32 nFileHandle, u64 pos, const void* buffer, u64 count, u64& outNumBytesWritten) const
		{
			NANDFileInfo* fileInfo = &mNandFileInfo[nFileHandle];

			outNumBytesWritten =  0;

			// make sure size is multiple of 32
			if (count % 32 == 0)
			{
				s32 result = NANDWrite(fileInfo, buffer, (u32)count);
				__CheckNANDError(result);

				if (result >= 0)
					outNumBytesWritten = (u64)result;
			}			
			else
			{
				x_printf("ERROR: Writing to file %d on Wii NAND but size was %d not multiple of 32 (remainder=%d)\n", nFileHandle, count, count % 32);
			}

			return (outNumBytesWritten != 0);
		}

		bool FileDevice_WII_NAND::seek(u32 nFileHandle, ESeekMode mode, u64 pos, u64& newPos)
		{
			NANDFileInfo* fileInfo = &mNandFileInfo[nFileHandle];

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
			return seek(nFileHandle, __SEEK_ORIGIN, pos, newPos);
		}

		bool	FileDevice_WII_NAND::seekCurrent(u32 nFileHandle, u64 pos, u64& newPos)
		{
			return seek(nFileHandle, __SEEK_CURRENT, pos, newPos);
		}

		bool	FileDevice_WII_NAND::seekEnd(u32 nFileHandle, u64 pos, u64& newPos)
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

#endif // TARGET_WII
