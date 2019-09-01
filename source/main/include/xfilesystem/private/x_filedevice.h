#ifndef __X_FILESYSTEM_FILEDEVICE_H__
#define __X_FILESYSTEM_FILEDEVICE_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

//==============================================================================
#include "xbase/x_runes.h"
#include "xfilesystem/x_enumerator.h"
#include "xfilesystem/private/x_enumerations.h"

#define MAX_ENUM_SEARCH_FILES 32
#define MAX_ENUM_SEARCH_DIRS 16

namespace xcore
{
    class xfiledevice;
    class xfilepath;
    class xdirpath;
    class xfileinfo;
    class xdirinfo;
    class xfileattrs;
    class xfiletimes;
    class xstream;

    // System file device
    extern xfiledevice* x_CreateFileDevice(utf32::crunes& pDrivePath, xbool boCanWrite);
    extern void         x_DestroyFileDevice(xfiledevice*);

    extern xfiledevice* x_NullFileDevice();

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
        virtual bool canWrite() = 0;
        virtual bool canSeek()  = 0;

        virtual bool getDeviceInfo(u64& totalSpace, u64& freeSpace) = 0;

        virtual bool openFile(xfilepath const& szFilename, EFileMode mode, EFileAccess access, EFileOp op,
                              void*& outHandle)                                                                = 0;
        virtual bool readFile(void* pHandle, u64 pos, void* buffer, u64 count, u64& outNumBytesRead)           = 0;
        virtual bool writeFile(void* pHandle, u64 pos, void const* buffer, u64 count, u64& outNumBytesWritten) = 0;
        virtual bool closeFile(void* pHandle)                                                                  = 0;

        virtual bool createStream(xfilepath const& szFilename, bool boRead, bool boWrite, xstream*& strm) = 0;
        virtual bool closeStream(xstream* strm)                                                           = 0;

        virtual bool setLengthOfFile(void* pHandle, u64 inLength)   = 0;
        virtual bool getLengthOfFile(void* pHandle, u64& outLength) = 0;

        virtual bool setFileTime(xfilepath const& szFilename, xfiletimes const& times) = 0;
        virtual bool getFileTime(xfilepath const& szFilename, xfiletimes& outTimes)    = 0;
        virtual bool setFileAttr(xfilepath const& szFilename, xfileattrs const& attr)  = 0;
        virtual bool getFileAttr(xfilepath const& szFilename, xfileattrs& attr)        = 0;

        virtual bool setFileTime(void* pHandle, xfiletimes const& times) = 0;
        virtual bool getFileTime(void* pHandle, xfiletimes& outTimes)    = 0;

        virtual bool hasFile(xfilepath const& szFilename)                                                   = 0;
        virtual bool moveFile(xfilepath const& szFilename, xfilepath const& szToFilename, bool boOverwrite) = 0;
        virtual bool copyFile(xfilepath const& szFilename, xfilepath const& szToFilename, bool boOverwrite) = 0;
        virtual bool deleteFile(xfilepath const& szFilename)                                                = 0;

        virtual bool hasDir(xdirpath const& szDirPath)                                                 = 0;
        virtual bool moveDir(xdirpath const& szDirPath, xdirpath const& szToDirPath, bool boOverwrite) = 0;
        virtual bool copyDir(xdirpath const& szDirPath, xdirpath const& szToDirPath, bool boOverwrite) = 0;
        virtual bool createDir(xdirpath const& szDirPath)                                              = 0;
        virtual bool deleteDir(xdirpath const& szDirPath)                                              = 0;

        virtual bool setDirTime(xdirpath const& szDirPath, xfiletimes const& ftimes) = 0;
        virtual bool getDirTime(xdirpath const& szDirPath, xfiletimes& ftimes)       = 0;
        virtual bool setDirAttr(xdirpath const& szDirPath, xfileattrs const& attr)   = 0;
        virtual bool getDirAttr(xdirpath const& szDirPath, xfileattrs& attr)         = 0;

        virtual bool enumerate(xdirpath const& szDirPath, enumerate_delegate& enumerator) = 0;
    };
}; // namespace xcore

#endif