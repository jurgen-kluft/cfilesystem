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
#include "xbase\x_memory_std.h"
#include "xbase\x_string_std.h"
#include "xbase\x_string.h"
#include "xbase\x_va_list.h"
#include "xbase\x_integer.h"

#include "xtime\x_time.h"

#include "xfilesystem\private\x_filesystem_common.h"
#include "xfilesystem\private\x_filesystem_ps3.h"
#include "xfilesystem\x_attributes.h"
#include "xfilesystem\x_filedevice.h"
#include "xfilesystem\x_fileinfo.h"
#include "xfilesystem\x_dirinfo.h"
#include "xfilesystem\private\x_filedata.h"
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
			const char*				mDrivePath;
			xbool					mCanWrite;

		public:
									FileDevice_PC_System(const char* pDrivePath, xbool boCanWrite)
										: mDrivePath(pDrivePath)
										, mCanWrite(boCanWrite)							{ } 
			virtual					~FileDevice_PC_System()								{ }

			virtual bool			canSeek() const										{ return true; }
			virtual bool			canWrite() const									{ return mCanWrite; }

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

			virtual bool			enumerate(const char* szDirPath, bool boSearchSubDirectories, enumerate_delegate<xfileinfo>* file_enumerator, enumerate_delegate<xdirinfo>* dir_enumerator, s32 depth) const;

			enum ESeekMode { __SEEK_ORIGIN = 1, __SEEK_CURRENT = 2, __SEEK_END = 3, };
			bool					seek(u32 nFileHandle, ESeekMode mode, u64 pos, u64& newPos) const;
			bool					seekOrigin(u32 nFileHandle, u64 pos, u64& newPos) const;
			bool					seekCurrent(u32 nFileHandle, u64 pos, u64& newPos) const;
			bool					seekEnd(u32 nFileHandle, u64 pos, u64& newPos) const;

			XFILESYSTEM_OBJECT_NEW_DELETE()
		};

		xfiledevice*		x_CreateFileDevicePC(const char* pDrivePath, xbool boCanWrite)
		{
			FileDevice_PC_System* file_device = new FileDevice_PC_System(pDrivePath, boCanWrite);
			return file_device;
		}

		void			x_DestroyFileDevicePC(xfiledevice* device)
		{
			FileDevice_PC_System* file_device = (FileDevice_PC_System*)device;
			delete file_device;
		}

		bool FileDevice_PC_System::getDeviceInfo(u64& totalSpace, u64& freeSpace) const
		{
			ULARGE_INTEGER totalbytes, freebytes;
			if (GetDiskFreeSpaceEx( mDrivePath, NULL, &totalbytes, &freebytes) == 0)
				return false;

			freeSpace = freebytes.QuadPart;
			totalSpace = totalbytes.QuadPart;
			return true;
		}

		bool FileDevice_PC_System::hasFile(const char* szFilename) const
		{
			u32 shareType	= FILE_SHARE_READ;
			u32 fileMode	= GENERIC_READ;
			u32 disposition	= OPEN_EXISTING;
			u32 attrFlags	= FILE_ATTRIBUTE_NORMAL;

			HANDLE nFileHandle = ::CreateFile(szFilename, fileMode, shareType, NULL, disposition, attrFlags, NULL);
			if (nFileHandle!=INVALID_HANDLE_VALUE)
			{
				::CloseHandle((HANDLE)nFileHandle);
				return true;
			}
			return false;
		}
		
		bool FileDevice_PC_System::openFile(const char* szFilename, bool boRead, bool boWrite, u32& nFileHandle) const
		{
			u32 shareType	= FILE_SHARE_READ;
			u32 fileMode	= !boWrite ? GENERIC_READ : GENERIC_WRITE|GENERIC_READ;
			u32 disposition	= OPEN_EXISTING;
			u32 attrFlags	= FILE_ATTRIBUTE_NORMAL;

			HANDLE handle = ::CreateFile(szFilename, fileMode, shareType, NULL, disposition, attrFlags, NULL);
			nFileHandle = (u32)handle;
			return nFileHandle != (u32)INVALID_HANDLE_VALUE;
		}

		bool FileDevice_PC_System::createFile(const char* szFilename, bool boRead, bool boWrite, u32& nFileHandle) const
		{
			u32 shareType	= FILE_SHARE_READ;
			u32 fileMode	= !boWrite ? GENERIC_READ : GENERIC_WRITE|GENERIC_READ;
			u32 disposition	= CREATE_ALWAYS;
			u32 attrFlags	= FILE_ATTRIBUTE_NORMAL;

			HANDLE handle = ::CreateFile(szFilename, fileMode, shareType, NULL, disposition, attrFlags, NULL);
			nFileHandle = (u32)handle;
			return nFileHandle != (u32)INVALID_HANDLE_VALUE;
		}

		bool FileDevice_PC_System::readFile(u32 nFileHandle, u64 pos, void* buffer, u64 count, u64& outNumBytesRead) const
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
		bool FileDevice_PC_System::writeFile(u32 nFileHandle, u64 pos, const void* buffer, u64 count, u64& outNumBytesWritten) const
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

		bool FileDevice_PC_System::moveFile(const char* szFilename, const char* szToFilename) const
		{
			if (!canWrite())
				return false;

			return ::MoveFileA(szFilename, szToFilename) != 0;
		}

		bool FileDevice_PC_System::copyFile(const char* szFilename, const char* szToFilename, bool boOverwrite) const
		{
			if (!canWrite())
				return false;

			const bool failIfExists = boOverwrite==false;
			return ::CopyFileA(szFilename, szToFilename, failIfExists) != 0;
		}

		bool FileDevice_PC_System::closeFile(u32 nFileHandle) const
		{
			if (!::CloseHandle((HANDLE)nFileHandle))
				return false;
			return true;
		}

		bool FileDevice_PC_System::deleteFile(const char* szFilename) const
		{
			if (!::DeleteFile(szFilename))
				return false;
			return true;
		}

		bool FileDevice_PC_System::getFilePos(u32 nFileHandle, u64& outPos) const
		{
			if (canSeek())
			{
				xsize_t distanceLow = 0;
				xsize_t distanceHigh;
				distanceLow = ::SetFilePointer((HANDLE)nFileHandle, (LONG)distanceLow, (PLONG)&distanceHigh, FILE_CURRENT );
				if ( distanceLow == INVALID_SET_FILE_POINTER &&  GetLastError() != NO_ERROR )
				{
					return false;
				}
				outPos = (u64)distanceHigh; outPos = outPos << 32; outPos = outPos | distanceLow;
				return true;
			}
			return false;
		}

		bool FileDevice_PC_System::setFilePos(u32 nFileHandle, u64& ioPos) const
		{
			if (canSeek())
			{
				xsize_t distanceLow = (xsize_t)ioPos;
				xsize_t distanceHigh = (xsize_t)(ioPos >> 32);
				distanceLow = ::SetFilePointer((HANDLE)nFileHandle, (LONG)distanceLow, (PLONG)&distanceHigh, FILE_BEGIN);
				if ( distanceLow == INVALID_SET_FILE_POINTER &&  GetLastError() != NO_ERROR )
				{
					return false;
				}
				ioPos = (u64)distanceHigh; ioPos = ioPos << 32; ioPos = ioPos | distanceLow;
				return true;
			}
			return false;
		}

		bool FileDevice_PC_System::setLengthOfFile(u32 nFileHandle, u64 inLength) const
		{
			xsize_t distanceLow = (xsize_t)inLength;
			xsize_t distanceHigh = (xsize_t)(inLength >> 32);
			::SetFilePointer((HANDLE)nFileHandle, (LONG)distanceLow, (PLONG)&distanceHigh, FILE_BEGIN);
			::SetEndOfFile((HANDLE)nFileHandle);
			return true;
		}

		bool FileDevice_PC_System::getLengthOfFile(u32 nFileHandle, u64& outLength) const
		{
			DWORD lowSize, highSize;
			lowSize = ::GetFileSize((HANDLE)nFileHandle, &highSize);
			outLength = highSize;
			outLength = outLength << 16;
			outLength = outLength << 16;
			outLength = outLength | lowSize;
			return true;
		}

		bool FileDevice_PC_System::setFileTime(const char* szFilename, const xdatetime& creationTime, const xdatetime& lastAccessTime, const xdatetime& lastWriteTime) const
		{
			u32 nFileHandle;
			if (openFile(szFilename, true, false, nFileHandle))
			{
				FILETIME _creationTime;
				u64 uCreationTime = creationTime.toFileTime();
				_creationTime.dwHighDateTime = xmem_utils::hiu32(uCreationTime);
				_creationTime.dwLowDateTime  = xmem_utils::lou32(uCreationTime);

				FILETIME _lastAccessTime;
				u64 uLastAccessTime = lastAccessTime.toFileTime();
				_lastAccessTime.dwHighDateTime = xmem_utils::hiu32(uLastAccessTime);
				_lastAccessTime.dwLowDateTime  = xmem_utils::lou32(uLastAccessTime);

				FILETIME _lastWriteTime;
				u64 uLastWriteTime = lastWriteTime.toFileTime();
				_lastWriteTime.dwHighDateTime = xmem_utils::hiu32(uLastWriteTime);
				_lastWriteTime.dwLowDateTime  = xmem_utils::lou32(uLastWriteTime);

				::SetFileTime((HANDLE)nFileHandle, &_creationTime, &_lastAccessTime, &_lastWriteTime);
				closeFile(nFileHandle);
				return true;
			}
			return false;
		}

		bool FileDevice_PC_System::getFileTime(const char* szFilename, xdatetime& outCreationTime, xdatetime& outLastAccessTime, xdatetime& outLastWriteTime) const
		{
			u32 nFileHandle;
			if (openFile(szFilename, true, false, nFileHandle))
			{
				FILETIME _creationTime;
				FILETIME _lastAccessTime;
				FILETIME _lastWriteTime;
				::GetFileTime((HANDLE)nFileHandle, &_creationTime, &_lastAccessTime, &_lastWriteTime);
				outCreationTime   = xdatetime::sFromFileTime((u64)xmem_utils::makeu64(_creationTime.dwLowDateTime, _creationTime.dwHighDateTime));
				outLastAccessTime = xdatetime::sFromFileTime((u64)xmem_utils::makeu64(_lastAccessTime.dwLowDateTime, _lastAccessTime.dwHighDateTime));
				outLastWriteTime  = xdatetime::sFromFileTime((u64)xmem_utils::makeu64(_lastWriteTime.dwLowDateTime, _lastWriteTime.dwHighDateTime));
				closeFile(nFileHandle);
				return true;
			}
			return false;
		}

		bool FileDevice_PC_System::setFileAttr(const char* szFilename, const xattributes& attr) const
		{
			DWORD dwFileAttributes = 0;
			if (attr.isArchive())
				dwFileAttributes = dwFileAttributes | FILE_ATTRIBUTE_ARCHIVE;
			if (attr.isReadOnly())
				dwFileAttributes = dwFileAttributes | FILE_ATTRIBUTE_READONLY;
			if (attr.isHidden())
				dwFileAttributes = dwFileAttributes | FILE_ATTRIBUTE_HIDDEN;
			if (attr.isSystem())
				dwFileAttributes = dwFileAttributes | FILE_ATTRIBUTE_SYSTEM;

			return ::SetFileAttributes(szFilename, dwFileAttributes) == TRUE;
		}

		bool FileDevice_PC_System::getFileAttr(const char* szFilename, xattributes& attr) const
		{
			DWORD dwFileAttributes = 0;
			dwFileAttributes = ::GetFileAttributes(szFilename);
			if (dwFileAttributes == INVALID_FILE_ATTRIBUTES)
				return false;

			attr.setArchive(dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE);
			attr.setReadOnly(dwFileAttributes & FILE_ATTRIBUTE_READONLY);
			attr.setHidden(dwFileAttributes & FILE_ATTRIBUTE_HIDDEN);
			attr.setSystem(dwFileAttributes & FILE_ATTRIBUTE_SYSTEM);

			return true;
		}


		static HANDLE	sOpenDir(const char* szDirPath)
		{
			u32 shareType	= FILE_SHARE_READ;
			u32 fileMode	= GENERIC_READ;
			u32 disposition	= OPEN_EXISTING;
			u32 attrFlags	= FILE_FLAG_BACKUP_SEMANTICS;

			HANDLE handle = ::CreateFile(szDirPath, fileMode, shareType, NULL, disposition, attrFlags, NULL);
			return handle;
		}

		static void		sCloseDir(HANDLE handle)
		{
			::CloseHandle(handle);
		}

		bool FileDevice_PC_System::hasDir(const char* szDirPath) const
		{
			HANDLE handle = sOpenDir(szDirPath);
			if (handle == INVALID_HANDLE_VALUE)
				return false;
			sCloseDir(handle);
			return true;
		}

		bool FileDevice_PC_System::createDir(const char* szDirPath) const
		{
			return ::CreateDirectoryA(szDirPath, NULL) != 0;
		}

		bool FileDevice_PC_System::moveDir(const char* szDirPath, const char* szToDirPath) const
		{
			u32 dwFlags = MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED;
			return ::MoveFileExA(szDirPath, szToDirPath, dwFlags) != 0;
		}

		bool FileDevice_PC_System::copyDir(const char* szDirPath, const char* szToDirPath, bool boOverwrite) const
		{
			
			return false;
		}

		static bool sIsDots(const TCHAR* str)
		{
			return (x_strcmp(str,".")==0 || x_strcmp(str,"..")==0);
		}

		struct enumerate_delegate_files_delete_dir : public enumerate_delegate<xfileinfo>
		{
			virtual void operator () (s32 depth, const xfileinfo& inf, bool& terminate)
			{
				terminate = false;
				DWORD dwFileAttributes = ::GetFileAttributes((LPCTSTR)inf.getFullName().c_str());
				if (dwFileAttributes & FILE_ATTRIBUTE_READONLY)	// change read-only file mode
					::SetFileAttributes((LPCTSTR)inf.getFullName().c_str(), dwFileAttributes & ~FILE_ATTRIBUTE_READONLY);
				::DeleteFile((LPCTSTR)inf.getFullName().c_str());
			}
		};

		struct enumerate_delegate_dirs_delete_dir : public enumerate_delegate<xdirinfo>
		{
			virtual void operator () (s32 depth, const xdirinfo& inf, bool& terminate)
			{
				terminate = false;
				::RemoveDirectory((LPCTSTR)inf.getFullName().c_str()); // remove the empty directory
			}
		};

		bool FileDevice_PC_System::deleteDir(const char* szDirPath) const
		{
			enumerate_delegate_files_delete_dir files_enum;
			enumerate_delegate_dirs_delete_dir dirs_enum;
			return enumerate(szDirPath, true, &files_enum, &dirs_enum, 0);
		}

		bool FileDevice_PC_System::setDirTime(const char* szDirPath, const xdatetime& creationTime, const xdatetime& lastAccessTime, const xdatetime& lastWriteTime) const
		{
			HANDLE handle = sOpenDir(szDirPath);
			if (handle == INVALID_HANDLE_VALUE)
			{
				FILETIME _creationTime;
				u64 uCreationTime = creationTime.toFileTime();
				_creationTime.dwHighDateTime = xmem_utils::hiu32(uCreationTime);
				_creationTime.dwLowDateTime  = xmem_utils::lou32(uCreationTime);

				FILETIME _lastAccessTime;
				u64 uLastAccessTime = lastAccessTime.toFileTime();
				_lastAccessTime.dwHighDateTime = xmem_utils::hiu32(uLastAccessTime);
				_lastAccessTime.dwLowDateTime  = xmem_utils::lou32(uLastAccessTime);

				FILETIME _lastWriteTime;
				u64 uLastWriteTime = lastWriteTime.toFileTime();
				_lastWriteTime.dwHighDateTime = xmem_utils::hiu32(uLastWriteTime);
				_lastWriteTime.dwLowDateTime  = xmem_utils::lou32(uLastWriteTime);

				::SetFileTime(handle, &_creationTime, &_lastAccessTime, &_lastWriteTime);
				sCloseDir(handle);
				return true;
			}
			return false;
		}

		bool FileDevice_PC_System::getDirTime(const char* szDirPath, xdatetime& outCreationTime, xdatetime& outLastAccessTime, xdatetime& outLastWriteTime) const
		{
			HANDLE handle = sOpenDir(szDirPath);
			if (handle == INVALID_HANDLE_VALUE)
			{
				FILETIME _creationTime;
				FILETIME _lastAccessTime;
				FILETIME _lastWriteTime;
				::GetFileTime(handle, &_creationTime, &_lastAccessTime, &_lastWriteTime);
				outCreationTime   = xdatetime::sFromFileTime((u64)xmem_utils::makeu64(_creationTime.dwLowDateTime, _creationTime.dwHighDateTime));
				outLastAccessTime = xdatetime::sFromFileTime((u64)xmem_utils::makeu64(_lastAccessTime.dwLowDateTime, _lastAccessTime.dwHighDateTime));
				outLastWriteTime  = xdatetime::sFromFileTime((u64)xmem_utils::makeu64(_lastWriteTime.dwLowDateTime, _lastWriteTime.dwHighDateTime));
				sCloseDir(handle);
				return true;
			}
			return false;
		}

		bool FileDevice_PC_System::setDirAttr(const char* szDirPath, const xattributes& attr) const
		{
			DWORD dwFileAttributes = 0;
			if (attr.isArchive())
				dwFileAttributes = dwFileAttributes | FILE_ATTRIBUTE_ARCHIVE;
			if (attr.isReadOnly())
				dwFileAttributes = dwFileAttributes | FILE_ATTRIBUTE_READONLY;
			if (attr.isHidden())
				dwFileAttributes = dwFileAttributes | FILE_ATTRIBUTE_HIDDEN;
			if (attr.isSystem())
				dwFileAttributes = dwFileAttributes | FILE_ATTRIBUTE_SYSTEM;

			return ::SetFileAttributes(szDirPath, dwFileAttributes) == TRUE;
		}

		bool FileDevice_PC_System::getDirAttr(const char* szDirPath, xattributes& attr) const
		{
			DWORD dwFileAttributes = 0;
			dwFileAttributes = ::GetFileAttributes(szDirPath);
			if (dwFileAttributes == INVALID_FILE_ATTRIBUTES)
				return false;

			attr.setArchive(dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE);
			attr.setReadOnly(dwFileAttributes & FILE_ATTRIBUTE_READONLY);
			attr.setHidden(dwFileAttributes & FILE_ATTRIBUTE_HIDDEN);
			attr.setSystem(dwFileAttributes & FILE_ATTRIBUTE_SYSTEM);

			return true;
		}

		bool FileDevice_PC_System::enumerate(const char* szDirPath, bool boSearchSubDirectories, enumerate_delegate<xfileinfo>* file_enumerator, enumerate_delegate<xdirinfo>* dir_enumerator, s32 depth) const
		{
			HANDLE hFind;  // file handle

			WIN32_FIND_DATA FindFileData;

			char DirPathBuffer[MAX_PATH+2];
			char FileNameBuffer[MAX_PATH+2];
			xcstring DirPath(DirPathBuffer, sizeof(DirPathBuffer), szDirPath);
			DirPath += "\\*";

			xcstring FileName(FileNameBuffer, sizeof(FileNameBuffer), szDirPath);		// searching all files
			FileName += "\\";

			hFind = ::FindFirstFile(DirPath.c_str(), &FindFileData);					// find the first file
			if (hFind == INVALID_HANDLE_VALUE)
				return false;

			DirPath = FileName;
        
			bool bSearch = true;
			while(bSearch)
			{	// until we find an entry
				if (::FindNextFile(hFind,&FindFileData))
				{
					if (sIsDots(FindFileData.cFileName))
						continue;

					FileName += FindFileData.cFileName;

					if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
					{
						if (dir_enumerator==NULL)
						{
							// We have found a directory, recurse
							if (!enumerate(FileName.c_str(), boSearchSubDirectories, file_enumerator, dir_enumerator, depth+1))
							{ 
								::FindClose(hFind); 
								return false;											// directory couldn't be enumerated
							}
							if (dir_enumerator!=NULL)
							{
								xdirinfo di(FileName.c_str());
								bool terminate;
								(*dir_enumerator)(depth, di, terminate);
								bSearch = !terminate;
							}
						}
						FileName = DirPath;
					}
					else
					{
						xfileinfo fi(FileName.c_str());
						if (file_enumerator!=NULL)
						{
							bool terminate;
							(*file_enumerator)(depth, fi, terminate);
							bSearch = !terminate;
						}
						FileName = DirPath;
					}
				}
				else
				{
					if (::GetLastError() == ERROR_NO_MORE_FILES)
					{
						// No more files here
						bSearch = false;
					}
					else
					{
						// Some error occured, close the handle and return FALSE
						::FindClose(hFind); 
						return false;
					}
				}
			}
			::FindClose(hFind);					// Closing file handle

			if (dir_enumerator!=NULL)
			{
				xdirinfo di(FileName.c_str());
				bool terminate;
				(*dir_enumerator)(depth, di, terminate);
				bSearch = !terminate;
			}

			return true;
		}



		bool FileDevice_PC_System::seek(u32 nFileHandle, ESeekMode mode, u64 pos, u64& newPos) const
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
		bool	FileDevice_PC_System::seekOrigin(u32 nFileHandle, u64 pos, u64& newPos) const
		{
			return seek(nFileHandle, __SEEK_ORIGIN, pos, newPos);
		}

		bool	FileDevice_PC_System::seekCurrent(u32 nFileHandle, u64 pos, u64& newPos) const
		{
			return seek(nFileHandle, __SEEK_CURRENT, pos, newPos);
		}

		bool	FileDevice_PC_System::seekEnd(u32 nFileHandle, u64 pos, u64& newPos) const
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
