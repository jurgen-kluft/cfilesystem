#ifndef __X_FILESYSTEM_FILEDEVICE_H__
#define __X_FILESYSTEM_FILEDEVICE_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase/x_types.h"
#include "xfilesystem/x_enumerator.h"
#define MAX_ENUM_SEARCH_FILES 32
#define MAX_ENUM_SEARCH_DIRS 16

namespace xcore
{
	class xdatetime;

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

		virtual bool			hasFile(xstring const& szFilename) const = 0;
		virtual bool			openFile(xstring const& szFilename, bool boRead, bool boWrite, u32& outFileHandle) const = 0;
		virtual bool			createFile(xstring const& szFilename, bool boRead, bool boWrite, u32& outFileHandle) const = 0;
		virtual bool			readFile(u32 nFileHandle, u64 pos, void* buffer, u64 count, u64& outNumBytesRead) const = 0;
		virtual bool			writeFile(u32 nFileHandle, u64 pos, const void* buffer, u64 count, u64& outNumBytesWritten) const = 0;
		virtual bool			closeFile(u32 nFileHandle) const = 0;
		virtual bool			moveFile(xstring const& szFilename, xstring const& szToFilename) const = 0;
		virtual bool			copyFile(xstring const& szFilename, xstring const& szToFilename, bool boOverwrite) const = 0;
		virtual bool			deleteFile(xstring const& szFilename) const = 0;
		virtual bool			setFilePos(u32 nFileHandle, u64& ioPos) const = 0;
		virtual bool			getFilePos(u32 nFileHandle, u64& outPos) const = 0;
		virtual bool			setLengthOfFile(u32 nFileHandle, u64 inLength) const = 0;
		virtual bool			getLengthOfFile(u32 nFileHandle, u64& outLength) const = 0;
		virtual bool			setFileTime(xstring const& szFilename, const xdatetime& creationTime, const xdatetime& lastAccessTime, const xdatetime& lastWriteTime) const = 0;
		virtual bool			getFileTime(xstring const& szFilename, xdatetime& outCreationTime, xdatetime& outLastAccessTime, xdatetime& outLastWriteTime) const = 0;
		virtual bool			setFileAttr(xstring const& szFilename, const xattributes& attr) const = 0;
		virtual bool			getFileAttr(xstring const& szFilename, xattributes& attr) const = 0;

		virtual bool			hasDir(xstring const& szDirPath) const = 0;
		virtual bool			createDir(xstring const& szDirPath) const = 0;
		virtual bool			moveDir(xstring const& szDirPath, xstring const& szToDirPath) const = 0;
		virtual bool			copyDir(xstring const& szDirPath, xstring const& szToDirPath, bool boOverwrite) const = 0;
		virtual bool			deleteDir(xstring const& szDirPath) const = 0;
		virtual bool			setDirTime(xstring const& szDirPath, const xdatetime& creationTime, const xdatetime& lastAccessTime, const xdatetime& lastWriteTime) const = 0;
		virtual bool			getDirTime(xstring const& szDirPath, xdatetime& outCreationTime, xdatetime& outLastAccessTime, xdatetime& outLastWriteTime) const = 0;
		virtual bool			setDirAttr(xstring const& szDirPath, const xattributes& attr) const = 0;
		virtual bool			getDirAttr(xstring const& szDirPath, xattributes& attr) const = 0;

		virtual bool			enumerate(xstring const& szDirPath, bool boSearchSubDirectories, enumerate_delegate<xfileinfo>* file_enumerator, enumerate_delegate<xdirinfo>* dir_enumerator, s32 depth=0) const = 0;
	
	};

};

#endif