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

	class xfiledevice;
	class xfileinfo;
	class xdirinfo;
	struct xattributes;

	// System file device
	extern xfiledevice*	x_CreateFileDevice(const char* pDrivePath, xbool boCanWrite);
	extern void			x_DestroyFileDevice(xfiledevice*);

	// File device
	// 
	// This interface exists to present a way to implement different types
	// of file devices. The most obvious one is a file device that uses the
	// system to manipulate files. To support HTTP based file manipulation
	// the user could implement a file device that is using HTTP for data
	// transfer.
	class xfiledevice
	{
	protected:
		virtual					~xfiledevice() {}

	public:
		virtual bool			canWrite() const = 0;
		virtual bool			canSeek() const = 0;

		virtual bool			getDeviceInfo(u64& totalSpace, u64& freeSpace) const = 0;

		virtual bool			openFile(xstring const& szFilename, bool boRead, bool boWrite, void*& outHandle) const = 0;
		virtual bool			createFile(xstring const& szFilename, bool boRead, bool boWrite, void*& outHandle) const = 0;
		virtual bool			readFile(void* pHandle, u64 pos, void* buffer, u64 count, u64& outNumBytesRead) const = 0;
		virtual bool			writeFile(void* pHandle, u64 pos, const void* buffer, u64 count, u64& outNumBytesWritten) const = 0;
		virtual bool			closeFile(void* pHandle) const = 0;

		virtual bool			setLengthOfFile(void* pHandle, u64 inLength) const = 0;
		virtual bool			getLengthOfFile(void* pHandle, u64& outLength) const = 0;

		virtual bool			setFileTime(void* pHandle, const xfiletimes& times) const = 0;
		virtual bool			getFileTime(void* pHandle, xfiletimes& outTimes) const = 0;
		virtual bool			setFileAttr(void* pHandle, const xattributes& attr) const = 0;
		virtual bool			getFileAttr(void* pHandle, xattributes& attr) const = 0;

		virtual bool			hasFile(xstring const& szFilename) const = 0;
		virtual bool			moveFile(xstring const& szFilename, xstring const& szToFilename) const = 0;
		virtual bool			copyFile(xstring const& szFilename, xstring const& szToFilename, bool boOverwrite) const = 0;
		virtual bool			deleteFile(xstring const& szFilename) const = 0;

		virtual bool			hasDir(xstring const& szDirPath) const = 0;
		virtual bool			moveDir(xstring const& szDirPath, xstring const& szToDirPath) const = 0;
		virtual bool			copyDir(xstring const& szDirPath, xstring const& szToDirPath, bool boOverwrite) const = 0;
		virtual bool			createDir(xstring const& szDirPath) const = 0;
		virtual bool			deleteDir(xstring const& szDirPath) const = 0;

		virtual bool			setDirTime(xstring const& szDirPath, const xdatetime& creationTime, const xdatetime& lastAccessTime, const xdatetime& lastWriteTime) const = 0;
		virtual bool			getDirTime(xstring const& szDirPath, xdatetime& outCreationTime, xdatetime& outLastAccessTime, xdatetime& outLastWriteTime) const = 0;
		virtual bool			setDirAttr(xstring const& szDirPath, const xattributes& attr) const = 0;
		virtual bool			getDirAttr(xstring const& szDirPath, xattributes& attr) const = 0;

		virtual bool			enumerate(xstring const& szDirPath, enumerate_delegate* enumerator) const = 0;
	
	};

};

#endif