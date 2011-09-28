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

#include "xtime\x_datetime.h"

#include "xfilesystem\private\x_filesystem_common.h"
#include "xfilesystem\private\x_filesystem_ps3.h"
#include "xfilesystem\x_attributes.h"
#include "xfilesystem\x_filedevice.h"
#include "xfilesystem\x_fileinfo.h"
#include "xfilesystem\x_dirinfo.h"
#include "xfilesystem\private\x_filedata.h"
#include "xfilesystem\private\x_fileasync.h"
#include "xfilesystem\private\x_filesystem_cqueue.h"
#include "xfilesystem\private\x_filesystem_cstack.h"

extern xcore::x_iallocator* sAtomicAllocator;
namespace xcore
{
	//------------------------------------------------------------------------------------------
	//---------------------------------- PS3 IO Functions ------------------------------------
	//------------------------------------------------------------------------------------------
	namespace xfilesystem
	{
		class FileDevice_PS3_System : public xfiledevice
		{
			xbool					mCanWrite;

		public:
									FileDevice_PS3_System(xbool boCanWrite) 
										: mCanWrite(boCanWrite)							{ }
			virtual					~FileDevice_PS3_System()							{ }

			virtual bool			canSeek() const										{ return true; }
			virtual bool			canWrite() const									{ return mCanWrite; }

			virtual bool			openOrCreateFile(u32 nFileIndex, const char* szFilename, bool boRead, bool boWrite, u32& nFileHandle);
			virtual bool			setLengthOfFile(u32 nFileHandle, u64 inLength) const;
			virtual bool			getLengthOfFile(u32 nFileHandle, u64& outLength) const;
			virtual bool			closeFile(u32 nFileHandle) const;
			virtual bool			readFile(u32 nFileHandle, u64 pos, void* buffer, u64 count, u64& outNumBytesRead) const;
			virtual bool			writeFile(u32 nFileHandle, u64 pos, const void* buffer, u64 count, u64& outNumBytesWritten) const;

			enum ESeekMode { __SEEK_ORIGIN = 1, __SEEK_CURRENT = 2, __SEEK_END = 3, };
			bool					seek(u32 nFileHandle, ESeekMode mode, u64 pos, u64& newPos) const;
			bool					seekOrigin(u32 nFileHandle, u64 pos, u64& newPos);
			bool					seekCurrent(u32 nFileHandle, u64 pos, u64& newPos);
			bool					seekEnd(u32 nFileHandle, u64 pos, u64& newPos);
			
			//lxq
			virtual bool         deleteFile(const char* szFilename) const;
			virtual bool			getDeviceInfo(u64& totalSpace, u64& freeSpace) const;
			virtual bool			hasFile(const char* szFilename) const ;
			virtual bool			openFile(const char* szFilename, bool boRead, bool boWrite, u32& outFileHandle) const ;
			virtual bool			createFile(const char* szFilename, bool boRead, bool boWrite, u32& outFileHandle) const ;
			virtual bool			moveFile(const char* szFilename, const char* szToFilename) const ;
			virtual bool			copyFile(const char* szFilename, const char* szToFilename, bool boOverwrite) const ;
			virtual bool	        setFilePos(u32 nFileHandle, u64& ioPos) const ;
			virtual bool         getFilePos(u32 nFileHandle, u64& outPos) const ;
			virtual bool			setFileTime(const char* szFilename, const xdatetime& creationTime, const xdatetime& lastAccessTime, const xdatetime& lastWriteTime) const ;
			virtual bool			getFileTime(const char* szFilename, xdatetime& outCreationTime, xdatetime& outLastAccessTime, xdatetime& outLastWriteTime) const ;
			virtual bool			setFileAttr(const char* szFilename, const xattributes& attr) const ;
			virtual bool			getFileAttr(const char* szFilename, xattributes& attr) const ;

			virtual bool			hasDir(const char* szDirPath) const ;
			virtual bool			createDir(const char* szDirPath) const ;
			virtual bool			moveDir(const char* szDirPath, const char* szToDirPath) const ;
			virtual bool			copyDir(const char* szDirPath, const char* szToDirPath, bool boOverwrite) const ;
			virtual bool			deleteDir(const char* szDirPath) const ;
			virtual bool			setDirTime(const char* szDirPath, const xdatetime& creationTime, const xdatetime& lastAccessTime, const xdatetime& lastWriteTime) const ;
			virtual bool			getDirTime(const char* szDirPath, xdatetime& outCreationTime, xdatetime& outLastAccessTime, xdatetime& outLastWriteTime) const ;
			virtual bool			setDirAttr(const char* szDirPath, const xattributes& attr) const ;
			virtual bool			getDirAttr(const char* szDirPath, xattributes& attr) const ;

			virtual bool			enumerate(const char* szDirPath, bool boSearchSubDirectories, enumerate_delegate<xfileinfo>* file_enumerator, enumerate_delegate<xdirinfo>* dir_enumerator, s32 depth=0) const ;
			//
			XFILESYSTEM_OBJECT_NEW_DELETE()
		};

		xfiledevice*		x_CreateFileDevicePS3(xbool boCanWrite)
		{
			FileDevice_PS3_System* file_device = new FileDevice_PS3_System(boCanWrite);
			return file_device;
		}

		void			x_DestroyFileDevicePS3(xfiledevice* device)
		{
			FileDevice_PS3_System* file_device = (FileDevice_PS3_System*)device;
			delete file_device;
		}

		bool FileDevice_PS3_System::openOrCreateFile(u32 nFileIndex, const char* szFilename, bool boRead, bool boWrite, u32& nFileHandle)
		{
			s32	nFlags;
			if(boWrite)
			{
				nFlags	= CELL_FS_O_CREAT | CELL_FS_O_RDWR;
			}
			else
			{
				nFlags	= CELL_FS_O_RDONLY;
			}

			s32	hFile	= INVALID_FILE_HANDLE;
			s32	nResult = cellFsOpen (szFilename, nFlags, &hFile, NULL, 0);
			nFileHandle = (u32)hFile;

			bool boSuccess = (nResult == CELL_OK);
			return boSuccess;
		}

		bool FileDevice_PS3_System::hasFile(const char* szFilename) const
		{
			s32 nFlags;
			nFlags = CELL_FS_O_EXCL;
			s32 hFile = INVALID_FILE_HANDLE;
			s32 nResult = cellFsOpen(szFilename,nFlags,&hFile,NULL,0);
			if (nResult == CELL_OK)
			{
				cellFsClose(hFile);
				cellFsUnlink(szFilename);
				return false;
			}
			return true;
		}

		bool FileDevice_PS3_System::openFile(const char* szFilename, bool boRead, bool boWrite, u32& outFileHandle) const
		{
			s32 nFlags;
			nFlags = CELL_FS_O_EXCL;
			s32 hFile = INVALID_FILE_HANDLE;
			s32 nResult = cellFsOpen(szFilename,nFlags,&hFile,NULL,0);
			if (nResult == CELL_OK)
			{
				cellFsClose(hFile);
				cellFsUnlink(szFilename);
				outFileHandle = (u32)hFile;
				return false;
			}
			else
			{
				if (boWrite)
				{
					nFlags = CELL_FS_O_RDWR | CELL_FS_O_CREAT;
				}
				else
				{
					nFlags = CELL_FS_O_RDONLY | CELL_FS_O_CREAT;
				}
				nResult = cellFsOpen(szFilename,nFlags,&hFile,NULL,0);
				outFileHandle = (u32)hFile;
				bool boSuccess = (nResult == CELL_OK);
				return boSuccess;
			}
		}

		bool FileDevice_PS3_System::createFile(const char* szFilename, bool boRead, bool boWrite, u32& outFileHandle) const
		{
			s32 nFlags;
			if (boWrite)
			{
				nFlags = CELL_FS_O_EXCL | CELL_FS_O_RDWR;
			}
			else
			{
				nFlags = CELL_FS_O_EXCL | CELL_FS_O_RDONLY;
			}
			s32 hFile = INVALID_FILE_HANDLE;
			s32 nResult = cellFsOpen(szFilename,nFlags,&hFile,NULL,0);
			outFileHandle = (u32)hFile;

			bool boSuccess = (nResult == CELL_OK);
			return boSuccess;
		}

		bool FileDevice_PS3_System::getLengthOfFile(u32 nFileHandle, u64& outLength) const
		{
			CellFsStat stats;
			CellFsErrno nResult = cellFsFstat(nFileHandle,&stats);
			bool boSuccess = (nResult == CELL_OK);
			if (boSuccess)
				outLength = stats.st_size;
			else
				outLength = 0;

			return boSuccess;
		}

		bool FileDevice_PS3_System::setLengthOfFile(u32 nFileHandle, u64 inLength) const
		{
			CellFsStat stats;
			CellFsErrno nResult = cellFsFstat(nFileHandle,&stats);
			bool boSuccess = (nResult == CELL_OK);
			if (boSuccess)
				stats.st_size = inLength;

			return boSuccess;
		}

		bool FileDevice_PS3_System::closeFile(u32 nFileHandle) const
		{
			s32 nResult = cellFsClose (nFileHandle);
			bool boSuccess = false;
			if (nResult == CELL_OK)
			{
				nFileHandle = (u32)INVALID_FILE_HANDLE;
				boSuccess = true;
			}
			return boSuccess;
		}

		bool FileDevice_PS3_System::deleteFile(const char* szFilename) const
		{
			s32 nResult = cellFsUnlink(szFilename);
			bool boSuccess = false;
			if (nResult == CELL_OK)
			{
				boSuccess = true;
			}
			return boSuccess;
		}

		bool FileDevice_PS3_System::getDeviceInfo(u64& totalSpace, u64& freeSpace) const
		{
			return false;
		}

		bool FileDevice_PS3_System::readFile(u32 nFileHandle, u64 pos, void* buffer, u64 count, u64& outNumBytesRead) const
		{
			u64 newPos;
			if (seek(nFileHandle, __SEEK_ORIGIN, pos, newPos))
			{
				u64 numBytesRead;
				s32 nResult = cellFsRead (nFileHandle, buffer, count, &numBytesRead);

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
			return false;
		}
		bool FileDevice_PS3_System::writeFile(u32 nFileHandle, u64 pos, const void* buffer, u64 count, u64& outNumBytesWritten) const
		{
			u64 newPos;
			if (seek(nFileHandle, __SEEK_ORIGIN, pos, newPos))
			{
				u64 numBytesWritten;
				s32 nResult = cellFsWrite (nFileHandle, buffer, count, &numBytesWritten);

				bool boSuccess = (nResult == CELL_OK);
				outNumBytesWritten = 0;
				if (boSuccess)
				{
					outNumBytesWritten = (u32)numBytesWritten;
				}
				return boSuccess;
			}
			return false;
		}

		bool FileDevice_PS3_System::seek(u32 nFileHandle, ESeekMode mode, u64 pos, u64& newPos) const
		{
			u64	nPos;
			s32 nResult = cellFsLseek (nFileHandle, pos, mode, &nPos);
			bool boSuccess = (nResult == CELL_OK);
			newPos = boSuccess ? nPos : pos;
			return boSuccess;
		}
		bool	FileDevice_PS3_System::seekOrigin(u32 nFileHandle, u64 pos, u64& newPos)
		{
			return seek(nFileHandle, __SEEK_ORIGIN, pos, newPos);
		}

		bool	FileDevice_PS3_System::seekCurrent(u32 nFileHandle, u64 pos, u64& newPos)
		{
			return seek(nFileHandle, __SEEK_CURRENT, pos, newPos);
		}

		bool	FileDevice_PS3_System::seekEnd(u32 nFileHandle, u64 pos, u64& newPos)
		{
			return seek(nFileHandle, __SEEK_END, pos, newPos);
		}

		bool FileDevice_PS3_System::moveFile(const char* szFilename, const char* szToFilename) const
		{
			if (!canWrite())
				return false;
			s32 nResult = cellFsRename(szFilename,szToFilename) ;
			bool boSuccess = ( nResult == CELL_OK );
			return boSuccess;
		}

		bool FileDevice_PS3_System::copyFile(const char* szFilename, const char* szToFilename, bool boOverwrite) const
		{
			if (!canWrite())
				return false;

			u32 nFileHandle,nFileHandleTo;
			u64 fileLength,readLength,writeLength;
			if (boOverwrite)
			{
				if (!openFile(szFilename,true,true,nFileHandle))
					return false;
				if (!const_cast<FileDevice_PS3_System* >(this)->openOrCreateFile((u32)0,szToFilename,true,true,nFileHandleTo))
					return false;
			}
			else
			{
				if (!openFile(szFilename,true,true,nFileHandle))
					return false;
				if (!createFile(szToFilename,true,true,nFileHandleTo))
					return false;
			}
			if(getLengthOfFile(nFileHandle,fileLength))
				return false;
			void* buffer = (void*)heapAlloc(u32(fileLength) ,X_ALIGNMENT_DEFAULT);
			if(readFile(nFileHandle,0,buffer,fileLength,readLength))
				return false;
			if(writeFile(nFileHandleTo,0,buffer,fileLength,writeLength))
				return false;
			heapFree(buffer);
			return true;
		}

		bool FileDevice_PS3_System::setFilePos(u32 nFileHandle, u64& ioPos) const
		{
			if (canSeek())
			{
				u64 newPos;
				s32 nResult = cellFsLseek (nFileHandle, ioPos, __SEEK_ORIGIN, &newPos);
				bool boSuccess = (nResult == CELL_OK);
				newPos = boSuccess ? ioPos : 0;
				ioPos = newPos;
				return boSuccess;
			}
			return false;
		}

		bool FileDevice_PS3_System::getFilePos(u32 nFileHandle, u64& outPos) const
		{
			if (canSeek())
			{
				s32 nResult = cellFsLseek(nFileHandle,0,__SEEK_CURRENT,&outPos);
				bool boSuccess = (nResult == CELL_OK);
				return boSuccess;
			}
			return false;
		}

		bool FileDevice_PS3_System::setFileTime(const char* szFilename, const xdatetime& creationTime, const xdatetime& lastAccessTime, const xdatetime& lastWriteTime) const
		{
			CellFsUtimbuf buffer;
			buffer.actime = lastAccessTime.toFileTime();
			buffer.modtime = lastWriteTime.toFileTime();
			return (cellFsUtime(szFilename,&buffer) == CELL_OK );
		}

		bool FileDevice_PS3_System::getFileTime(const char* szFilename, xdatetime& outCreationTime, xdatetime& outLastAccessTime, xdatetime& outLastWriteTime) const
		{
			CellFsStat stats;
			u32 nFileHandle;
			if (openFile(szFilename,true,false,nFileHandle))
			{
				CellFsErrno nResult = cellFsFstat(nFileHandle,&stats);
				outCreationTime = xdatetime::sFromFileTime(stats.st_ctime);
				outLastAccessTime = xdatetime::sFromFileTime(stats.st_atime);
				outLastWriteTime = xdatetime::sFromFileTime(stats.st_mtime);
				closeFile(nFileHandle);
				return true;
			}
			return false;
		}

		bool FileDevice_PS3_System::setFileAttr(const char* szFilename, const xattributes& attr) const
		{
			return false;
		}

		bool FileDevice_PS3_System::getFileAttr(const char* szFilename, xattributes& attr) const
		{
			attr.setArchive(true);
			attr.setHidden(false);
			attr.setSystem(false);
			return true;
		}

		bool FileDevice_PS3_System::hasDir(const char* szDirPath) const
		{
			s32 hFile = INVALID_FILE_HANDLE;
			s32 nResult = cellFsOpendir(szDirPath,&hFile);
			if (nResult == CELL_OK)
			{
				cellFsClosedir(hFile);
				return true;
			}
			return false;
		}

		bool FileDevice_PS3_System::createDir(const char* szDirPath) const
		{
			s32 hFile = INVALID_FILE_HANDLE;
			if(cellFsOpendir(szDirPath,&hFile) == CELL_OK)
			{
				cellFsClosedir(hFile);
				return false;
			}
			else
			{
				s32 nResult = cellFsOpen(szDirPath,CELL_FS_O_CREAT | CELL_FS_O_RDONLY,&hFile,NULL,0);
				if (nResult == CELL_OK)
				{
					cellFsClose(hFile);
					return true;
				}
				return false;
			}
		}

		static bool sIsDots(const char* str)
		{
			return (x_strcmp(str,".")==0 || x_strcmp(str,"..")==0);
		}

		struct enumerate_delegate_files_delete_dir : public enumerate_delegate<xfileinfo>
		{
			virtual void operator () (s32 depth, const xfileinfo& inf, bool& terminate)
			{
				terminate = false;
				cellFsUnlink(inf.getFullName().c_str());
			}
		};

		struct enumerate_delegate_dirs_delete_dir : public enumerate_delegate<xdirinfo>
		{
			virtual void operator () (s32 depth, const xdirinfo& inf, bool& terminate)
			{
				terminate = false;
				cellFsRmdir(inf.getFullName().c_str()); // remove the empty directory
			}
		};

		struct enumerate_delegate_dirs_copy_dir : public enumerate_delegate<xdirinfo>
		{
			cstack<const xdirinfo* > dirStack;
			enumerate_delegate_dirs_copy_dir() { dirStack.init(sAtomicAllocator,16); }
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
			enumerate_delegate_files_copy_dir() { fileStack.init(sAtomicAllocator,16); }
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

		static bool enumerateCopyDir(const char* szDirPath, bool boSearchSubDirectories, enumerate_delegate<xfileinfo>* file_enumerator, enumerate_delegate<xdirinfo>* dir_enumerator, s32 depth/* =0 */)
		{
			char DirPathBuffer[CELL_FS_MAX_FS_PATH_LENGTH+2];
			char FileNameBuffer[CELL_FS_MAX_FS_PATH_LENGTH+2];
			xcstring DirPath(DirPathBuffer,sizeof(DirPathBuffer),szDirPath);
			DirPath += "//*";

			xcstring FileName(FileNameBuffer,sizeof(FileNameBuffer),szDirPath);
			FileName +="//";
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
					if (sIsDots(item.d_name))
						continue;

					if (readSize == 0)
						break;

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

		bool FileDevice_PS3_System::copyDir(const char* szDirPath, const char* szToDirPath, bool boOverwrite) const
		{
			enumerate_delegate_dirs_copy_dir dirs_copy_enum;
			enumerate_delegate_files_copy_dir files_copy_enum;
			enumerateCopyDir(szDirPath,true,&files_copy_enum,&dirs_copy_enum,0);
		
			const xdirinfo* dirInfo = NULL;
			while(dirs_copy_enum.dirStack.pop(dirInfo))
			{
				xdirpath copyDirPath_To;
				changeDirPath(szDirPath,szToDirPath,dirInfo,copyDirPath_To);
				// nDirinfo_From --------------------->   copyDirPath_To       ( copy dir)
				delete dirInfo;			dirInfo = NULL;
				if (cellFsMkdir(copyDirPath_To.c_str(),CELL_FS_OTH_CREATE_MODE_RW | CELL_FS_S_IFDIR) != CELL_FS_SUCCEEDED )
					return false;
			}

			const xfileinfo* fileInfo = NULL;
			while(files_copy_enum.fileStack.pop(fileInfo))
			{
				xfilepath copyFilePath_To;
				changeFilePath(szDirPath,szToDirPath,fileInfo,copyFilePath_To);
				//nFileinfo_From --------------------->  copyFilePath_To       (copy file)
				if (!moveFile(fileInfo->getFullName().c_str(),copyFilePath_To.c_str()))
				{
					delete fileInfo;		fileInfo = NULL;
					return false;
				}
				delete fileInfo;		fileInfo = NULL; 
			}

			return true;
		}

		bool FileDevice_PS3_System::deleteDir(const char* szDirPath) const
		{
			enumerate_delegate_files_delete_dir files_enum;
			enumerate_delegate_dirs_delete_dir dirs_enum;
			return enumerate(szDirPath,true,&files_enum,&dirs_enum);			
		}

		bool FileDevice_PS3_System::moveDir(const char* szDirPath, const char* szToDirPath) const
		{
			if (!copyDir(szDirPath,szToDirPath,true))
				return false;
			if (!deleteDir(szDirPath))
				return false;

			return true;
		}

		bool FileDevice_PS3_System::setDirTime(const char* szDirPath, const xdatetime& creationTime, const xdatetime& lastAccessTime, const xdatetime& lastWriteTime) const
		{
			CellFsUtimbuf buffer;
			buffer.actime = lastAccessTime.toFileTime();
			buffer.modtime = lastWriteTime.toFileTime();
			return (cellFsUtime(szDirPath,&buffer) == CELL_OK );
		}

		bool FileDevice_PS3_System::getDirTime(const char* szDirPath, xdatetime& outCreationTime, xdatetime& outLastAccessTime, xdatetime& outLastWriteTime) const
		{
			CellFsStat stats;
			s32 nFileHandle;
			if (cellFsOpendir(szDirPath,&nFileHandle))
			{
				CellFsErrno nResult = cellFsFstat(nFileHandle,&stats);
				outCreationTime = xdatetime::sFromFileTime(stats.st_ctime);
				outLastAccessTime = xdatetime::sFromFileTime(stats.st_atime);
				outLastWriteTime = xdatetime::sFromFileTime(stats.st_mtime);
				cellFsClosedir(nFileHandle);
				return true;
			}
			return false;
		}

		bool FileDevice_PS3_System::setDirAttr(const char* szDirPath, const xattributes& attr) const
		{
			return false;
		}

		bool FileDevice_PS3_System::getDirAttr(const char* szDirPath, xattributes& attr) const
		{
			attr.setArchive(true);
			attr.setHidden(false);
			attr.setReadOnly(true);
			attr.setSystem(false);
			return false;
		}

		bool FileDevice_PS3_System::enumerate(const char* szDirPath, bool boSearchSubDirectories, enumerate_delegate<xfileinfo>* file_enumerator, enumerate_delegate<xdirinfo>* dir_enumerator, s32 depth/* =0 */) const
		{
			char DirPathBuffer[CELL_FS_MAX_FS_PATH_LENGTH+2];
			char FileNameBuffer[CELL_FS_MAX_FS_PATH_LENGTH+2];
			xcstring DirPath(DirPathBuffer,sizeof(DirPathBuffer),szDirPath);
			DirPath += "//*";

			xcstring FileName(FileNameBuffer,sizeof(FileNameBuffer),szDirPath);
			FileName +="//";
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
					if (sIsDots(item.d_name))
						continue;

					if (readSize == 0)
						break;

					FileName += item.d_name;
					if (item.d_type == CELL_FS_TYPE_DIRECTORY)
					{
						if (!enumerate(FileName.c_str(),boSearchSubDirectories,file_enumerator,dir_enumerator,depth+1))
						{
							cellFsClosedir(nDirHandle);
							return false;
						}

						FileName = DirPath;
					}
					else if (item.d_type == CELL_FS_TYPE_REGULAR)
					{
						xfileinfo fi(FileName.c_str());
						if (file_enumerator!=NULL)
						{
							bool terminate;
							(*file_enumerator)(depth,fi,terminate);
							bSearch = !terminate;
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
				xdirinfo di(FileName.c_str());
				bool terminate;
				(*dir_enumerator)(depth,di,terminate);
				bSearch = !terminate;
			}

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

#endif // TARGET_PS3