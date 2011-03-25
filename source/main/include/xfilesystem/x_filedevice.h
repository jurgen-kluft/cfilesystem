#ifndef __X_FILESYSTEM_FILEDEVICE_H__
#define __X_FILESYSTEM_FILEDEVICE_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase\x_types.h"
#include "xfilesystem\x_enumerator.h"

//==============================================================================
// xcore namespace
//==============================================================================
namespace xcore
{
	namespace xfilesystem
	{
		///< File device
		///< 
		///< This interface exists to present a way to implement different types
		///< of file devices. The most obvious one is a file device that uses the
		///< system to manipulate files. To support HTTP based file manipulation
		///< the user could implement a file device that is using HTTP for data
		///< transfer.
		class xfiledevice
		{
		protected:
			virtual					~xfiledevice() {}

		public:
			virtual bool			canWrite() const = 0;
			virtual bool			canSeek() const = 0;

			virtual bool			hasFile(const char* szFilename) = 0;
			virtual bool			openFile(const char* szFilename, bool boRead, bool boWrite, u32& outFileHandle) = 0;
			virtual bool			createFile(const char* szFilename, bool boRead, bool boWrite, u32& outFileHandle) = 0;
			virtual bool			readFile(u32 nFileHandle, u64 pos, void* buffer, u64 count, u64& outNumBytesRead) = 0;
			virtual bool			writeFile(u32 nFileHandle, u64 pos, const void* buffer, u64 count, u64& outNumBytesWritten) = 0;
			virtual bool			closeFile(u32 nFileHandle) = 0;
			virtual bool			renameFile(const char* szFilename, const char* szToFilename) = 0;
			virtual bool			copyFile(const char* szFilename, const char* szToFilename) = 0;
			virtual bool			deleteFile(const char* szFilename) = 0;
			virtual void			setFileTime(const char* szFilename, const xdatetime& lastAccessTime, const xdatetime& lastWriteTime) const = 0;
			virtual void			getFileTime(const char* szFilename, xdatetime& outCreationTime, xdatetime& outLastAccessTime, xdatetime& outLastWriteTime) const = 0;

			virtual bool			hasDir(const char* szDirPath) = 0;
			virtual bool			createDir(const char* szDirPath) = 0;
			virtual bool			renameDir(const char* szDirPath, const char* szToDirPath) = 0;
			virtual bool			copyDir(const char* szDirPath, const char* szToDirPath) = 0;
			virtual bool			removeDir(const char* szDirPath) = 0;
			virtual bool			setDirTime(const char* szDirPath, const xdatetime& lastAccessTime, const xdatetime& lastWriteTime) = 0;
			virtual bool			getDirTime(const char* szDirPath, xdatetime& outCreationTime, xdatetime& outLastAccessTime, xdatetime& outLastWriteTime) = 0;

			virtual bool			enumerate(const char* szDirPath, bool boSearchSubDirectories, enumerate_delegate<xfileinfo>* file_enumerator, enumerate_delegate<xdirinfo>* dir_enumerator);

			virtual bool			setLengthOfFile(u32 nFileHandle, u64 inLength) = 0;
			virtual bool			getLengthOfFile(u32 nFileHandle, u64& outLength) const = 0;
		
		};
	};

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

//==============================================================================
// END __X_FILESYSTEM_FILEDEVICE_H__
//==============================================================================
#endif