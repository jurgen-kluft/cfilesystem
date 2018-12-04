#ifndef __X_FILESYSTEM_FILEDEVICE_H__
#define __X_FILESYSTEM_FILEDEVICE_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

//==============================================================================
// INCLUDES
//==============================================================================
#include "xfilesystem/x_enumerator.h"

#define MAX_ENUM_SEARCH_FILES 32
#define MAX_ENUM_SEARCH_DIRS 16

namespace xcore
{
	class xdatetime;

	class xfiledevice;
	class xfileinfo;
	class xdirinfo;
	struct xfileattrs;
	class xstream;

	// System file device
	extern xfiledevice* x_CreateFileDevice(const char* pDrivePath, xbool boCanWrite);
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
		virtual ~xfiledevice() {}

	public:
		virtual bool canWrite() const = 0;
		virtual bool canSeek() const  = 0;

		virtual bool getDeviceInfo(u64& totalSpace, u64& freeSpace) const = 0;

		virtual bool openFile(xfilepath const& szFilename, bool boRead, bool boWrite, void*& outHandle) const		 = 0;
		virtual bool createFile(xfilepath const& szFilename, bool boRead, bool boWrite, void*& outHandle) const		 = 0;
		virtual bool readFile(void* pHandle, u64 pos, void* buffer, u64 count, u64& outNumBytesRead) const			 = 0;
		virtual bool writeFile(void* pHandle, u64 pos, const void* buffer, u64 count, u64& outNumBytesWritten) const = 0;
		virtual bool closeFile(void* pHandle) const																	 = 0;

		virtual bool createStream(xfilepath const& szFilename, bool boRead, bool boWrite, xstream*& strm) const = 0;
		virtual bool closeStream(xstream* strm) const															= 0;

		virtual bool setLengthOfFile(void* pHandle, u64 inLength) const   = 0;
		virtual bool getLengthOfFile(void* pHandle, u64& outLength) const = 0;

		virtual bool setFileTime(xfilepath const& szFilename, const xfiletimes& times) const = 0;
		virtual bool getFileTime(xfilepath const& szFilename, xfiletimes& outTimes) const	= 0;
		virtual bool setFileAttr(xfilepath const& szFilename, const xfileattrs& attr) const  = 0;
		virtual bool getFileAttr(xfilepath const& szFilename, xfileattrs& attr) const		 = 0;

		virtual bool setFileTime(void* pHandle, const xfiletimes& times) const = 0;
		virtual bool getFileTime(void* pHandle, xfiletimes& outTimes) const	= 0;
		virtual bool setFileAttr(void* pHandle, const xfileattrs& attr) const  = 0;
		virtual bool getFileAttr(void* pHandle, xfileattrs& attr) const		   = 0;

		virtual bool hasFile(xfilepath const& szFilename) const													  = 0;
		virtual bool moveFile(xfilepath const& szFilename, xfilepath const& szToFilename) const					  = 0;
		virtual bool copyFile(xfilepath const& szFilename, xfilepath const& szToFilename, bool boOverwrite) const = 0;
		virtual bool deleteFile(xfilepath const& szFilename) const												  = 0;

		virtual bool hasDir(xdirpath const& szDirPath) const												 = 0;
		virtual bool moveDir(xdirpath const& szDirPath, xdirpath const& szToDirPath) const					 = 0;
		virtual bool copyDir(xdirpath const& szDirPath, xdirpath const& szToDirPath, bool boOverwrite) const = 0;
		virtual bool createDir(xdirpath const& szDirPath) const												 = 0;
		virtual bool deleteDir(xdirpath const& szDirPath) const												 = 0;

		virtual bool setDirTime(xdirpath const& szDirPath, const xfiletimes& ftimes) const = 0;
		virtual bool getDirTime(xdirpath const& szDirPath, xfiletimes& ftimes) const	   = 0;
		virtual bool setDirAttr(xdirpath const& szDirPath, const xfileattrs& attr) const   = 0;
		virtual bool getDirAttr(xdirpath const& szDirPath, xfileattrs& attr) const		   = 0;

		virtual bool enumerate(xdirpath const& szDirPath, enumerate_delegate& enumerator) const = 0;
	};
};

#endif