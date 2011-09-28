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
#define MAX_ENUM_SEARCH_FILES 32
#define MAX_ENUM_SEARCH_DIRS 16
//==============================================================================
// xcore namespace
//==============================================================================
namespace xcore
{
	class xdatetime;

	namespace xfilesystem
	{
		class xfileinfo;
		class xdirinfo;
		struct xattributes;

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

			virtual bool			getDeviceInfo(u64& totalSpace, u64& freeSpace) const = 0;

			virtual bool			hasFile(const char* szFilename) const = 0;
			virtual bool			openFile(const char* szFilename, bool boRead, bool boWrite, u32& outFileHandle) const = 0;
			virtual bool			createFile(const char* szFilename, bool boRead, bool boWrite, u32& outFileHandle) const = 0;
			virtual bool			readFile(u32 nFileHandle, u64 pos, void* buffer, u64 count, u64& outNumBytesRead) const = 0;
			virtual bool			writeFile(u32 nFileHandle, u64 pos, const void* buffer, u64 count, u64& outNumBytesWritten) const = 0;
			virtual bool			closeFile(u32 nFileHandle) const = 0;
			virtual bool			moveFile(const char* szFilename, const char* szToFilename) const = 0;
			virtual bool			copyFile(const char* szFilename, const char* szToFilename, bool boOverwrite) const = 0;
			virtual bool			deleteFile(const char* szFilename) const = 0;
			virtual bool			setFilePos(u32 nFileHandle, u64& ioPos) const = 0;
			virtual bool			getFilePos(u32 nFileHandle, u64& outPos) const = 0;
			virtual bool			setLengthOfFile(u32 nFileHandle, u64 inLength) const = 0;
			virtual bool			getLengthOfFile(u32 nFileHandle, u64& outLength) const = 0;
			virtual bool			setFileTime(const char* szFilename, const xdatetime& creationTime, const xdatetime& lastAccessTime, const xdatetime& lastWriteTime) const = 0;
			virtual bool			getFileTime(const char* szFilename, xdatetime& outCreationTime, xdatetime& outLastAccessTime, xdatetime& outLastWriteTime) const = 0;
			virtual bool			setFileAttr(const char* szFilename, const xattributes& attr) const = 0;
			virtual bool			getFileAttr(const char* szFilename, xattributes& attr) const = 0;

			virtual bool			hasDir(const char* szDirPath) const = 0;
			virtual bool			createDir(const char* szDirPath) const = 0;
			virtual bool			moveDir(const char* szDirPath, const char* szToDirPath) const = 0;
			virtual bool			copyDir(const char* szDirPath, const char* szToDirPath, bool boOverwrite) const = 0;
			virtual bool			deleteDir(const char* szDirPath) const = 0;
			virtual bool			setDirTime(const char* szDirPath, const xdatetime& creationTime, const xdatetime& lastAccessTime, const xdatetime& lastWriteTime) const = 0;
			virtual bool			getDirTime(const char* szDirPath, xdatetime& outCreationTime, xdatetime& outLastAccessTime, xdatetime& outLastWriteTime) const = 0;
			virtual bool			setDirAttr(const char* szDirPath, const xattributes& attr) const = 0;
			virtual bool			getDirAttr(const char* szDirPath, xattributes& attr) const = 0;

			virtual bool			enumerate(const char* szDirPath, bool boSearchSubDirectories, enumerate_delegate<xfileinfo>* file_enumerator, enumerate_delegate<xdirinfo>* dir_enumerator, s32 depth=0) const = 0;
		
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