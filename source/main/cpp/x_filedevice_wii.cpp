#include "xbase/x_target.h"
#ifdef TARGET_WII

//==============================================================================
// INCLUDES
//==============================================================================
#include <revolution.h>
#include <revolution\hio2.h>
#include <revolution\nand.h>
#include <revolution\dvd.h>

#include "xbase/x_debug.h"
#include "xbase/x_limits.h"
#include "xbase/x_string_std.h"
#include "xbase/x_va_list.h"

#include "xtime/x_datetime.h"

#include "xfilesystem/private/x_filesystem_common.h"
#include "xfilesystem/private/x_filesystem_wii.h"
#include "xfilesystem/x_filedevice.h"
#include "xfilesystem/private/x_filedata.h"
#include "xfilesystem/private/x_fileasync.h"
#include "xfilesystem/x_attributes.h"

#include "xatomic/x_queue.h"
#include "xbase/x_allocator.h"

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
			u32					mMaxNumOpenFiles;

			mutable xcore::atomic::queue<u32> mFreeFileinfoIndecies;
			//xcore::atomic::queue<u32> mUsedFileinfoIdecies;

		public:
									FileDevice_WII_DVD();							
			virtual					~FileDevice_WII_DVD();								

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
			bool					seek(u32 nFileHandle, ESeekMode mode, u64 pos, u64& newPos) const;
			bool					seekOrigin(u32 nFileHandle, u64 pos, u64& newPos) const ;
			bool					seekCurrent(u32 nFileHandle, u64 pos, u64& newPos) const;
			bool					seekEnd(u32 nFileHandle, u64 pos, u64& newPos) const;



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

		FileDevice_WII_DVD::FileDevice_WII_DVD()
		{
			//mDvdFileInfo = new DVDFileInfo[xfs_common::s_instance()->getMaxOpenFiles()];
			mDvdFileInfo = (DVDFileInfo*)heapAlloc(sizeof(DVDFileInfo) * xfs_common::s_instance()->getMaxOpenFiles(), 4);
			mNumOpenFiles = 0;
			mMaxNumOpenFiles = xfs_common::s_instance()->getMaxOpenFiles();

			mFreeFileinfoIndecies.init(getAllocator(), xfs_common::s_instance()->getMaxOpenFiles());
			//mUsedFileinfoIdecies.init(getAllocator(), xfs_common::s_instance()->getMaxOpenFiles());

			for(s32 i = 0; i < xfs_common::s_instance()->getMaxOpenFiles(); i++)
			{
				u32 freeFileIndex = (u32)i;
				mFreeFileinfoIndecies.push(freeFileIndex);
			}
		}

		FileDevice_WII_DVD::~FileDevice_WII_DVD()
		{ 
			//delete [] mDvdFileInfo;
			heapFree(mDvdFileInfo);
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
			//ASSERT(0);
			nFileHandle = INVALID_FILE_HANDLE;
			return false;
		}

		bool FileDevice_WII_DVD::getDeviceInfo(u64& totalSpace, u64& freeSpace) const
		{
			return false;
		}

		bool FileDevice_WII_DVD::hasFile(const char* szFilename) const
		{
			u32 fileHandle;
			if(openFile(szFilename, true, false, fileHandle))
			{
				closeFile(fileHandle);
				return true;
			}

			return false;
		}

		bool FileDevice_WII_DVD::getLengthOfFile(u32 nFileHandle, u64& outLength) const
		{
			if(nFileHandle >= mMaxNumOpenFiles)
			{
				return false;
			}

			u64 l = (u64)DVDGetLength(&mDvdFileInfo[nFileHandle]);
			// if the file exists, dvdgetlength should already give us a 32-divisible number...
			//l = (l + 31) & 0xffffffe0;											///< DVDRead requires size to be a multiple of 32 so we better return a file size that fulfills this requirement
			outLength = l;
			return true;
		}

		bool FileDevice_WII_DVD::setLengthOfFile(u32 nFileHandle, u64 inLength) const
		{
			return false;
		}

		bool FileDevice_WII_DVD::closeFile(u32 nFileHandle) const
		{
			if(nFileHandle >= mMaxNumOpenFiles)
			{
				return false;
			}

			DVDFileInfo* dvdFileInfo = &mDvdFileInfo[nFileHandle];
			xbool boSuccess = DVDClose(dvdFileInfo);
			



			mFreeFileinfoIndecies.push(nFileHandle);
			
			nFileHandle = INVALID_FILE_HANDLE;



			return boSuccess;
		}

		bool FileDevice_WII_DVD::deleteFile(const char* szFilename) const
		{

			// cannot delete files from a dvd
			//ASSERT(0);
			return false;
		}

		bool FileDevice_WII_DVD::readFile(u32 nFileHandle, u64 pos, void* buffer,  u64 count, u64& outNumBytesRead) const
		{

			if(nFileHandle >= mMaxNumOpenFiles)
			{
				outNumBytesRead = 0;
				return false;
			}

			DVDFileInfo* dvdFileInfo = &mDvdFileInfo[nFileHandle];
			// For now, let the user deal with this.  May not be safe to modify size of output buffer provided anyway
			//count = (count + 31) & 0xffffffe0;									///< DVDRead requires count to be a multiple of 32
			
			xcore::x_printf("Reading from WII DVD length = %d, offset = %d\n", count, pos);


			s32 numBytesRead = DVDRead(dvdFileInfo, buffer, (s32)count, (s32)pos);
			bool boSuccess = numBytesRead >= 0;
			if (boSuccess)
			{
				outNumBytesRead = numBytesRead;
			}
			return boSuccess;
		}

		bool FileDevice_WII_DVD::writeFile(u32 nFileHandle, u64 pos, const void* buffer,  u64 count, u64& outNumBytesWritten) const
		{
			//DVDFileInfo* dvdFileInfo = &mDvdFileInfo[nFileHandle];
			// Unable to write
			
			//ASSERT(0);
			return false;
		}

		bool FileDevice_WII_DVD::seek(u32 nFileHandle, ESeekMode mode, u64 pos, u64& newPos) const
		{

			if(nFileHandle >= mMaxNumOpenFiles)
			{
				return false;
			}

			DVDFileInfo* dvdFileInfo = &mDvdFileInfo[nFileHandle];
			if (mode == __SEEK_END)
			{
				pos = (u64)DVDGetLength(dvdFileInfo) - pos;
				pos = (pos + 3) & 0xfffffffc; // handle multiple of 4?
			}
			else if(mode == __SEEK_CURRENT)
			{
				if(pos %4 != 0 && pos != 0)
				{
					pos = (pos + 3) & 0xfffffffc; // handle multiple of 4?
				}
				pos = dvdFileInfo->cb.offset + pos;
			}
			else
			{
				pos = (pos + 3) & 0xfffffffc; // handle multiple of 4?
			}

			
			s32 nResult = DVDSeek(dvdFileInfo, (s32)pos);
			if (nResult==0)
				newPos = pos;
			return nResult==0;
		}
		bool	FileDevice_WII_DVD::seekOrigin(u32 nFileHandle, u64 pos, u64& newPos) const
		{
			return seek(nFileHandle, __SEEK_ORIGIN, pos, newPos);
		}

		bool	FileDevice_WII_DVD::seekCurrent(u32 nFileHandle, u64 pos, u64& newPos) const
		{
			return seek(nFileHandle, __SEEK_CURRENT, pos, newPos);
		}

		bool	FileDevice_WII_DVD::seekEnd(u32 nFileHandle, u64 pos, u64& newPos) const
		{
			return seek(nFileHandle, __SEEK_END, pos, newPos);
		}


		bool FileDevice_WII_DVD::moveFile(const char* szFilename, const char* szToFilename) const
		{
			return false;
		}

		bool FileDevice_WII_DVD::copyFile(const char* szFilename, const char* szToFilename, bool boOverwrite) const
		{
				return false;


		}

		bool FileDevice_WII_DVD::setFilePos(u32 nFileHandle, u64& ioPos) const
		{

			return false;
		}

		bool FileDevice_WII_DVD::getFilePos(u32 nFileHandle, u64& outPos) const
		{
			if (canSeek())
			{
				u64 seekOutPos;
				bool nResult = seekCurrent(nFileHandle, 0, seekOutPos);
				outPos = seekOutPos;
				return nResult;
			}
			return false;
		}

		bool FileDevice_WII_DVD::setFileTime(const char* szFilename, const xdatetime& creationTime, const xdatetime& lastAccessTime, const xdatetime& lastWriteTime) const
		{

			return false;
		}

		bool FileDevice_WII_DVD::getFileTime(const char* szFilename, xdatetime& outCreationTime, xdatetime& outLastAccessTime, xdatetime& outLastWriteTime) const
		{

			return false;
		}

		bool FileDevice_WII_DVD::setFileAttr(const char* szFilename, const xattributes& attr) const
		{
			return false;
		}

		bool FileDevice_WII_DVD::getFileAttr(const char* szFilename, xattributes& attr) const
		{
			attr.setArchive(true);
			attr.setHidden(false);
			attr.setSystem(false);
			return true;
		}

		bool FileDevice_WII_DVD::hasDir(const char* szDirPath) const
		{
			DVDDir dvdDirInfo;

			BOOL result = DVDOpenDir(szDirPath, &dvdDirInfo);

			if(result)
				DVDCloseDir(&dvdDirInfo);

			return result;
		}

		bool FileDevice_WII_DVD::createDir(const char* szDirPath) const
		{
			return false;
		}



		/*
		static bool sIsDots(const char* str)
		{
			return (x_strcmp(str,".")==0 || x_strcmp(str,"..")==0);
		}

		struct enumerate_delegate_files_delete_dir : public enumerate_delegate<xfileinfo>
		{
			virtual void operator () (s32 depth, const xfileinfo& inf, bool& terminate)
			{
				terminate = false;
				char systemFileBuffer[xdirpath::XDIRPATH_MAX];
				xcstring systemFile(systemFileBuffer, sizeof(systemFileBuffer));
				formatPath(inf.getFullName().c_str_device(),systemFile);
				cellFsUnlink(systemFile.c_str());
			}

			using enumerate_delegate::operator();
		};

		struct enumerate_delegate_dirs_delete_dir : public enumerate_delegate<xdirinfo>
		{
			virtual void operator () (s32 depth, const xdirinfo& inf, bool& terminate)
			{
				terminate = false;
				char systemDirBuffer[xfilepath::XFILEPATH_MAX];
				xcstring systemDir(systemDirBuffer,sizeof(systemDirBuffer));
				formatPath(inf.getFullName().c_str_device(),systemDir);
				cellFsRmdir(systemDir.c_str()); // remove the empty directory
			}

			using enumerate_delegate::operator();
		};

		struct enumerate_delegate_dirs_copy_dir : public enumerate_delegate<xdirinfo>
		{
			cstack<const xdirinfo* > dirStack;
			enumerate_delegate_dirs_copy_dir() { dirStack.init(getAllocator(),MAX_ENUM_SEARCH_DIRS); }
			virtual ~enumerate_delegate_dirs_copy_dir() { dirStack.clear(); }
 			virtual void operator () (s32 depth, const xdirinfo& inf, bool& terminate)		{ }
			virtual void operator() (s32 depth, const xdirinfo* inf, bool& terminate )
			{
				terminate = false;
				dirStack.push(inf);
			}
		};

		struct enumerate_delegate_files_copy_dir : public enumerate_delegate<xfileinfo>
		{
			cstack<const xfileinfo* > fileStack;
			enumerate_delegate_files_copy_dir() { fileStack.init(getAllocator(),MAX_ENUM_SEARCH_FILES); }
			virtual ~enumerate_delegate_files_copy_dir() { fileStack.clear(); }
			virtual void operator () (s32 depth, const xfileinfo& inf, bool& terminate)	{ }
			virtual void operator() (s32 depth, const xfileinfo* inf, bool& terminate ) 
			{
				terminate = false;
				fileStack.push(inf);
			}
		};

		static void changeDirPath(const char* szDirPath,const char* szToDirPath,const xdirinfo* szDirinfo,xdirpath& outDirPath)
		{
			xdirpath nDirpath_from(szDirPath);
			xdirpath nDirpath_to(szToDirPath);
			s32 depth1 = nDirpath_from.getLevels();
			xdirpath parent,child;
			szDirinfo->getFullName().split(depth1,parent,child);
			nDirpath_to.getSubDir(child.c_str(),outDirPath);
		}

		static void changeFilePath(const char* szDirPath,const char* szToDirPath,const xfileinfo* szFileinfo,xfilepath& outFilePath)
		{
			xdirpath nDir;
			szFileinfo->getFullName().getDirPath(nDir);
			xdirpath nDirpath_from(szDirPath);
			xdirpath nDirpath_to(szToDirPath);

			xfilepath fileName = szFileinfo->getFullName();
			fileName.onlyFilename();
			s32 depth = nDirpath_from.getLevels();
			xdirpath parent,child,copyDirPath_To;
			nDir.split(depth,parent,child);
			nDirpath_to.getSubDir(child.c_str(),copyDirPath_To);
			outFilePath = xfilepath(copyDirPath_To,fileName);
		}

		static bool enumerateCopyDir(const char* szDirPath, bool boSearchSubDirectories, enumerate_delegate<xfileinfo>* file_enumerator, enumerate_delegate<xdirinfo>* dir_enumerator, s32 depth)
		{
			char DirPathBuffer[CELL_FS_MAX_FS_PATH_LENGTH+2];
			char FileNameBuffer[CELL_FS_MAX_FS_PATH_LENGTH+2];
			xcstring DirPath(DirPathBuffer,sizeof(DirPathBuffer),szDirPath);
			DirPath += "/";

			xcstring FileName(FileNameBuffer,sizeof(FileNameBuffer),szDirPath);
			FileName +="/";
			s32 nDirHandle;
			cellFsOpendir(DirPath.c_str(),&nDirHandle);

			DirPath = FileName;
			bool bSearch = true;
			while(bSearch)
			{
				uint64_t readSize;
				CellFsDirent item;
				s32 nResult = cellFsReaddir(nDirHandle,&item,&readSize);
				if(nResult == CELL_FS_SUCCEEDED)
				{
					if (readSize == 0)
						break;

					if (sIsDots(item.d_name))
						continue;

					FileName += item.d_name;
					if (item.d_type == CELL_FS_TYPE_DIRECTORY)
					{
						if (!enumerateCopyDir(FileName.c_str(),boSearchSubDirectories,file_enumerator,dir_enumerator,depth+1))
						{
							cellFsClosedir(nDirHandle);
							return false;
						}

						FileName = DirPath;
					}
					else if (item.d_type == CELL_FS_TYPE_REGULAR)
					{
						xfileinfo* fi = new xfileinfo(FileName.c_str());
						if (file_enumerator!=NULL && fi!=NULL)
						{
							bool terminate;
							(*file_enumerator)(depth,fi,terminate);
							bSearch = !terminate;
						}
						else
						{
							delete fi;		fi = NULL;
						}
						FileName = DirPath;
					}
				}
				else
				{
					if (nResult == CELL_FS_EFAULT)
					{
							bSearch = false;
					}
					else
					{
						cellFsClosedir(nDirHandle);
						return false;
					}
				}
			}
			cellFsClosedir(nDirHandle);

			if (dir_enumerator!=NULL)
			{
				xdirinfo* di = new xdirinfo(FileName.c_str());
				bool terminate;
				if (di != NULL)
				{
					(*dir_enumerator)(depth,di,terminate);
				}
				else
				{
					delete di;			di = NULL;
				}
				bSearch = !terminate;
			}

			return true;
		}
		*/

		bool FileDevice_WII_DVD::copyDir(const char* szDirPath, const char* szToDirPath, bool boOverwrite) const
		{

			return false;
		}

		bool FileDevice_WII_DVD::deleteDir(const char* szDirPath) const
		{
			return false;			
		}

		bool FileDevice_WII_DVD::moveDir(const char* szDirPath, const char* szToDirPath) const
		{

			return false;
		}

		bool FileDevice_WII_DVD::setDirTime(const char* szDirPath, const xdatetime& creationTime, const xdatetime& lastAccessTime, const xdatetime& lastWriteTime) const
		{
			// invoke cellFsUtime
			// This API is not supported
			return false;
		}

		bool FileDevice_WII_DVD::getDirTime(const char* szDirPath, xdatetime& outCreationTime, xdatetime& outLastAccessTime, xdatetime& outLastWriteTime) const
		{

			return false;
		}

		bool FileDevice_WII_DVD::setDirAttr(const char* szDirPath, const xattributes& attr) const
		{
			return false;
		}

		bool FileDevice_WII_DVD::getDirAttr(const char* szDirPath, xattributes& attr) const
		{
			return false;
		}

		bool FileDevice_WII_DVD::enumerate(const char* szDirPath, bool boSearchSubDirectories, enumerate_delegate<xfileinfo>* file_enumerator, enumerate_delegate<xdirinfo>* dir_enumerator, s32 depth/* =0 */) const
		{
			return false;
		}


		//------------------------------------------------------------------------------------------
		//----------------------------------     NAND           ------------------------------------
		//------------------------------------------------------------------------------------------

		class FileDevice_WII_NAND : public xfiledevice
		{
			NANDFileInfo*		mNandFileInfo;
			u32					mNumOpenFiles;
			u32					mMaxNumOpenFiles;

			mutable xcore::atomic::queue<u32> mFreeFileinfoIndecies;
			//xcore::atomic::queue<u32> mUsedFileinfoIdecies;
		public:

			FileDevice_WII_NAND()
			{
				mFreeFileinfoIndecies.init(getAllocator(), xfs_common::s_instance()->getMaxOpenFiles());
				//mNandFileInfo = new NANDFileInfo[xfs_common::s_instance()->getMaxOpenFiles()];
				mNandFileInfo = (NANDFileInfo*)heapAlloc(xfs_common::s_instance()->getMaxOpenFiles() * sizeof(NANDFileInfo), 4);
				mNumOpenFiles = 0;
				mMaxNumOpenFiles = xfs_common::s_instance()->getMaxOpenFiles();

				for(s32 i = 0; i < mMaxNumOpenFiles; i++)
				{
					u32 freeFileIndex = (u32)i;
					mFreeFileinfoIndecies.push(freeFileIndex);
				}

			}
			virtual					~FileDevice_WII_NAND()								
			{
				//delete [] mNandFileInfo;
				heapFree(mNandFileInfo);
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
			bool					seek(u32 nFileHandle, ESeekMode mode, u64 pos, u64& newPos) const;
			bool					seekOrigin(u32 nFileHandle, u64 pos, u64& newPos) const;
			bool					seekCurrent(u32 nFileHandle, u64 pos, u64& newPos) const;
			bool					seekEnd(u32 nFileHandle, u64 pos, u64& newPos) const;

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


		bool FileDevice_WII_NAND::createFile( const char* szFilename, bool boRead, bool boWrite, u32& nFileHandle) const
		{

			
			return openFile(szFilename, true, true, nFileHandle);
		}

		bool FileDevice_WII_NAND::getDeviceInfo(u64& totalSpace, u64& freeSpace) const
		{
			return false;
		}

		bool FileDevice_WII_NAND::hasFile(const char* szFilename) const
		{
			u32 fileHandle;
			if(openFile(szFilename, true, false, fileHandle))
			{
				closeFile(fileHandle);
				return true;
			}

			return false;
		}

		bool FileDevice_WII_NAND::getLengthOfFile(u32 nFileHandle, u64& outLength) const
		{
			if(nFileHandle >= mMaxNumOpenFiles)
			{
				return false;
			}

			NANDFileInfo* fileInfo = &mNandFileInfo[nFileHandle];

			unsigned long length = 0;
			s32 result = NANDGetLength(fileInfo, &length);
			__CheckNANDError(result);

			outLength = (u64)length;

			return (result == NAND_RESULT_OK);
		}

		bool FileDevice_WII_NAND::setLengthOfFile(u32 nFileHandle, u64 inLength) const
		{
			return false;

			// unfinished...

			if(nFileHandle >= mMaxNumOpenFiles)
			{
				return false;
			}

			NANDFileInfo* fileInfo = &mNandFileInfo[nFileHandle];
		}

		bool FileDevice_WII_NAND::closeFile(u32 nFileHandle) const
		{

			if(nFileHandle >= mMaxNumOpenFiles)
			{
				return false;
			}

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

			if(nFileHandle >= mMaxNumOpenFiles)
			{
				outNumBytesRead = 0;
				return false;
			}

			NANDFileInfo* fileInfo = &mNandFileInfo[nFileHandle];

			outNumBytesRead = 0;

			// make sure size is multiple of 32
			//count = (count + 31) & 0xffffffe0;									///< NANDread requires count to be a multiple of 32
			{
				s32 result = NANDRead(fileInfo, buffer, (u32)count);
				__CheckNANDError(result);

				if (result >= 0)
					outNumBytesRead = (u64)result;
			}

			return (outNumBytesRead != 0);
		}
		bool FileDevice_WII_NAND::writeFile(u32 nFileHandle, u64 pos, const void* buffer, u64 count, u64& outNumBytesWritten) const
		{
			if(nFileHandle >= mMaxNumOpenFiles)
			{
				outNumBytesWritten = 0;
				return false;
			}

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
				ASSERT(0);
			}

			return (outNumBytesWritten != 0);
		}

		bool FileDevice_WII_NAND::seek(u32 nFileHandle, ESeekMode mode, u64 pos, u64& newPos) const
		{

			if(nFileHandle >= mMaxNumOpenFiles)
			{
				return false;
			}

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

			::u32 fileCurrPos = 0;
			result = NANDTell(fileInfo, &fileCurrPos);

			newPos = fileCurrPos;

			return (result >= 0);
		}
		bool	FileDevice_WII_NAND::seekOrigin(u32 nFileHandle, u64 pos, u64& newPos) const
		{
			return seek(nFileHandle, __SEEK_ORIGIN, pos, newPos);
		}

		bool	FileDevice_WII_NAND::seekCurrent(u32 nFileHandle, u64 pos, u64& newPos) const
		{
			return seek(nFileHandle, __SEEK_CURRENT, pos, newPos);
		}

		bool	FileDevice_WII_NAND::seekEnd(u32 nFileHandle, u64 pos, u64& newPos) const
		{
			return seek(nFileHandle, __SEEK_END, pos, newPos);
		}


		bool FileDevice_WII_NAND::moveFile(const char* szFilename, const char* szToFilename) const
		{
			return false;
		}

		bool FileDevice_WII_NAND::copyFile(const char* szFilename, const char* szToFilename, bool boOverwrite) const
		{
				return false;


		}

		bool FileDevice_WII_NAND::setFilePos(u32 nFileHandle, u64& ioPos) const
		{
			u64 newPos;
			return seek(nFileHandle, __SEEK_CURRENT, ioPos, newPos);
		}

		bool FileDevice_WII_NAND::getFilePos(u32 nFileHandle, u64& outPos) const
		{
			if(nFileHandle >= mMaxNumOpenFiles)
			{
				return false;
			}

			NANDFileInfo* fileInfo = &mNandFileInfo[nFileHandle];

			::u32 seekOutPos = 0;
			::s32 result = NANDTell(fileInfo, &seekOutPos);

			outPos = seekOutPos;

			__CheckNANDError(result);

			return result == NAND_RESULT_OK;
		}

		bool FileDevice_WII_NAND::setFileTime(const char* szFilename, const xdatetime& creationTime, const xdatetime& lastAccessTime, const xdatetime& lastWriteTime) const
		{

			return false;
		}

		bool FileDevice_WII_NAND::getFileTime(const char* szFilename, xdatetime& outCreationTime, xdatetime& outLastAccessTime, xdatetime& outLastWriteTime) const
		{

			return false;
		}

		bool FileDevice_WII_NAND::setFileAttr(const char* szFilename, const xattributes& attr) const
		{
			return false;
		}

		bool FileDevice_WII_NAND::getFileAttr(const char* szFilename, xattributes& attr) const
		{
			attr.setArchive(true);
			attr.setHidden(false);
			attr.setSystem(false);
			return true;
		}

		bool FileDevice_WII_NAND::hasDir(const char* szDirPath) const
		{
			DVDDir dvdDirInfo;

			s32 result = NANDChangeDir(szDirPath);

			return result == NAND_RESULT_OK;
		}

		bool FileDevice_WII_NAND::createDir(const char* szDirPath) const
		{
			return false;
		}



		/*
		static bool sIsDots(const char* str)
		{
			return (x_strcmp(str,".")==0 || x_strcmp(str,"..")==0);
		}

		struct enumerate_delegate_files_delete_dir : public enumerate_delegate<xfileinfo>
		{
			virtual void operator () (s32 depth, const xfileinfo& inf, bool& terminate)
			{
				terminate = false;
				char systemFileBuffer[xdirpath::XDIRPATH_MAX];
				xcstring systemFile(systemFileBuffer, sizeof(systemFileBuffer));
				formatPath(inf.getFullName().c_str_device(),systemFile);
				cellFsUnlink(systemFile.c_str());
			}

			using enumerate_delegate::operator();
		};

		struct enumerate_delegate_dirs_delete_dir : public enumerate_delegate<xdirinfo>
		{
			virtual void operator () (s32 depth, const xdirinfo& inf, bool& terminate)
			{
				terminate = false;
				char systemDirBuffer[xfilepath::XFILEPATH_MAX];
				xcstring systemDir(systemDirBuffer,sizeof(systemDirBuffer));
				formatPath(inf.getFullName().c_str_device(),systemDir);
				cellFsRmdir(systemDir.c_str()); // remove the empty directory
			}

			using enumerate_delegate::operator();
		};

		struct enumerate_delegate_dirs_copy_dir : public enumerate_delegate<xdirinfo>
		{
			cstack<const xdirinfo* > dirStack;
			enumerate_delegate_dirs_copy_dir() { dirStack.init(getAllocator(),MAX_ENUM_SEARCH_DIRS); }
			virtual ~enumerate_delegate_dirs_copy_dir() { dirStack.clear(); }
 			virtual void operator () (s32 depth, const xdirinfo& inf, bool& terminate)		{ }
			virtual void operator() (s32 depth, const xdirinfo* inf, bool& terminate )
			{
				terminate = false;
				dirStack.push(inf);
			}
		};

		struct enumerate_delegate_files_copy_dir : public enumerate_delegate<xfileinfo>
		{
			cstack<const xfileinfo* > fileStack;
			enumerate_delegate_files_copy_dir() { fileStack.init(getAllocator(),MAX_ENUM_SEARCH_FILES); }
			virtual ~enumerate_delegate_files_copy_dir() { fileStack.clear(); }
			virtual void operator () (s32 depth, const xfileinfo& inf, bool& terminate)	{ }
			virtual void operator() (s32 depth, const xfileinfo* inf, bool& terminate ) 
			{
				terminate = false;
				fileStack.push(inf);
			}
		};

		static void changeDirPath(const char* szDirPath,const char* szToDirPath,const xdirinfo* szDirinfo,xdirpath& outDirPath)
		{
			xdirpath nDirpath_from(szDirPath);
			xdirpath nDirpath_to(szToDirPath);
			s32 depth1 = nDirpath_from.getLevels();
			xdirpath parent,child;
			szDirinfo->getFullName().split(depth1,parent,child);
			nDirpath_to.getSubDir(child.c_str(),outDirPath);
		}

		static void changeFilePath(const char* szDirPath,const char* szToDirPath,const xfileinfo* szFileinfo,xfilepath& outFilePath)
		{
			xdirpath nDir;
			szFileinfo->getFullName().getDirPath(nDir);
			xdirpath nDirpath_from(szDirPath);
			xdirpath nDirpath_to(szToDirPath);

			xfilepath fileName = szFileinfo->getFullName();
			fileName.onlyFilename();
			s32 depth = nDirpath_from.getLevels();
			xdirpath parent,child,copyDirPath_To;
			nDir.split(depth,parent,child);
			nDirpath_to.getSubDir(child.c_str(),copyDirPath_To);
			outFilePath = xfilepath(copyDirPath_To,fileName);
		}

		static bool enumerateCopyDir(const char* szDirPath, bool boSearchSubDirectories, enumerate_delegate<xfileinfo>* file_enumerator, enumerate_delegate<xdirinfo>* dir_enumerator, s32 depth)
		{
			char DirPathBuffer[CELL_FS_MAX_FS_PATH_LENGTH+2];
			char FileNameBuffer[CELL_FS_MAX_FS_PATH_LENGTH+2];
			xcstring DirPath(DirPathBuffer,sizeof(DirPathBuffer),szDirPath);
			DirPath += "/";

			xcstring FileName(FileNameBuffer,sizeof(FileNameBuffer),szDirPath);
			FileName +="/";
			s32 nDirHandle;
			cellFsOpendir(DirPath.c_str(),&nDirHandle);

			DirPath = FileName;
			bool bSearch = true;
			while(bSearch)
			{
				uint64_t readSize;
				CellFsDirent item;
				s32 nResult = cellFsReaddir(nDirHandle,&item,&readSize);
				if(nResult == CELL_FS_SUCCEEDED)
				{
					if (readSize == 0)
						break;

					if (sIsDots(item.d_name))
						continue;

					FileName += item.d_name;
					if (item.d_type == CELL_FS_TYPE_DIRECTORY)
					{
						if (!enumerateCopyDir(FileName.c_str(),boSearchSubDirectories,file_enumerator,dir_enumerator,depth+1))
						{
							cellFsClosedir(nDirHandle);
							return false;
						}

						FileName = DirPath;
					}
					else if (item.d_type == CELL_FS_TYPE_REGULAR)
					{
						xfileinfo* fi = new xfileinfo(FileName.c_str());
						if (file_enumerator!=NULL && fi!=NULL)
						{
							bool terminate;
							(*file_enumerator)(depth,fi,terminate);
							bSearch = !terminate;
						}
						else
						{
							delete fi;		fi = NULL;
						}
						FileName = DirPath;
					}
				}
				else
				{
					if (nResult == CELL_FS_EFAULT)
					{
							bSearch = false;
					}
					else
					{
						cellFsClosedir(nDirHandle);
						return false;
					}
				}
			}
			cellFsClosedir(nDirHandle);

			if (dir_enumerator!=NULL)
			{
				xdirinfo* di = new xdirinfo(FileName.c_str());
				bool terminate;
				if (di != NULL)
				{
					(*dir_enumerator)(depth,di,terminate);
				}
				else
				{
					delete di;			di = NULL;
				}
				bSearch = !terminate;
			}

			return true;
		}
		*/

		bool FileDevice_WII_NAND::copyDir(const char* szDirPath, const char* szToDirPath, bool boOverwrite) const
		{
			
			return false;
		}

		bool FileDevice_WII_NAND::deleteDir(const char* szDirPath) const
		{
			s32 result = NANDDelete(szDirPath);

			return result == NAND_RESULT_OK;			
		}

		bool FileDevice_WII_NAND::moveDir(const char* szDirPath, const char* szToDirPath) const
		{
			s32 result = NANDMove(szDirPath, szToDirPath);
			return result == NAND_RESULT_OK;
		}

		bool FileDevice_WII_NAND::setDirTime(const char* szDirPath, const xdatetime& creationTime, const xdatetime& lastAccessTime, const xdatetime& lastWriteTime) const
		{
			// invoke cellFsUtime
			// This API is not supported
			return false;
		}

		bool FileDevice_WII_NAND::getDirTime(const char* szDirPath, xdatetime& outCreationTime, xdatetime& outLastAccessTime, xdatetime& outLastWriteTime) const
		{

			return false;
		}

		bool FileDevice_WII_NAND::setDirAttr(const char* szDirPath, const xattributes& attr) const
		{
			return false;
		}

		bool FileDevice_WII_NAND::getDirAttr(const char* szDirPath, xattributes& attr) const
		{
			return false;
		}

		bool FileDevice_WII_NAND::enumerate(const char* szDirPath, bool boSearchSubDirectories, enumerate_delegate<xfileinfo>* file_enumerator, enumerate_delegate<xdirinfo>* dir_enumerator, s32 depth/* =0 */) const
		{
			return false;
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
