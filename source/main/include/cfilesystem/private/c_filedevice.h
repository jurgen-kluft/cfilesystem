#ifndef __C_FILESYSTEM_FILEDEVICE_H__
#define __C_FILESYSTEM_FILEDEVICE_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

//==============================================================================
#include "cfilesystem/private/c_enumerations.h"

#define MAX_ENUM_SEARCH_FILES 32
#define MAX_ENUM_SEARCH_DIRS 16

namespace ncore
{
	class alloc_t;
    struct runes_t;
    struct crunes_t;

    class enumerate_delegate_t;

    class filedevice_t;
    class filepath_t;
    class dirpath_t;
    class fileattrs_t;
    class filetimes_t;
    class stream_t;
    struct pathdevice_t;

    // System file device
    extern filedevice_t* gCreateFileDevice(bool boCanWrite);
    extern void          gDestroyFileDevice(filedevice_t*);
    extern filedevice_t* gNullFileDevice();

    // File device
    //
    // This interface exists to present a way to implement different types
    // of file devices. The most obvious one is a file device that uses the
    // system to manipulate files. To support HTTP based file manipulation
    // the user could implement a file device that is using HTTP for data
    // transfer.
    class filedevice_t
    {
    protected:
        virtual ~filedevice_t() {}

    public:
        virtual void destruct(alloc_t* allocator) = 0;

        virtual bool canWrite() const = 0;
        virtual bool canSeek() const = 0;

        virtual bool getDeviceInfo(pathdevice_t* device, u64& totalSpace, u64& freeSpace) const = 0;

        virtual bool openFile(filepath_t const& szFilename, EFileMode mode, EFileAccess access, EFileOp op, void*& outHandle) = 0;
        virtual bool createFile(const filepath_t& szFilename, bool boRead, bool boWrite, void*& nFileHandle) = 0;
        virtual bool readFile(void* pHandle, u64 pos, void* buffer, u64 count, u64& outNumBytesRead)           = 0;
        virtual bool writeFile(void* pHandle, u64 pos, void const* buffer, u64 count, u64& outNumBytesWritten) = 0;
        virtual bool flushFile(void* pHandle)                                                                  = 0;
        virtual bool closeFile(void* pHandle)                                                                  = 0;

        virtual bool createStream(filepath_t const& szFilename, bool boRead, bool boWrite, stream_t& strm) = 0;
        virtual bool closeStream(stream_t& strm)                                                           = 0;

        virtual bool setLengthOfFile(void* pHandle, u64 inLength)   = 0;
        virtual bool getLengthOfFile(void* pHandle, u64& outLength) = 0;

        virtual bool setFileTime(filepath_t const& szFilename, filetimes_t const& times) = 0;
        virtual bool getFileTime(filepath_t const& szFilename, filetimes_t& outTimes)    = 0;
        virtual bool setFileAttr(filepath_t const& szFilename, fileattrs_t const& attr)  = 0;
        virtual bool getFileAttr(filepath_t const& szFilename, fileattrs_t& attr)        = 0;

        virtual bool setFileTime(void* pHandle, filetimes_t const& times) = 0;
        virtual bool getFileTime(void* pHandle, filetimes_t& outTimes)    = 0;

        virtual bool hasFile(filepath_t const& szFilename)                                                   = 0;
        virtual bool moveFile(filepath_t const& szFilename, filepath_t const& szToFilename, bool boOverwrite) = 0;
        virtual bool copyFile(filepath_t const& szFilename, filepath_t const& szToFilename, bool boOverwrite) = 0;
        virtual bool deleteFile(filepath_t const& szFilename)                                                = 0;

        virtual bool openDir(dirpath_t const& szDirPath, void*& nDirHandle)                             = 0;
        virtual bool hasDir(dirpath_t const& szDirPath)                                                 = 0;
        virtual bool moveDir(dirpath_t const& szDirPath, dirpath_t const& szToDirPath, bool boOverwrite) = 0;
        virtual bool copyDir(dirpath_t const& szDirPath, dirpath_t const& szToDirPath, bool boOverwrite) = 0;
        virtual bool createDir(dirpath_t const& szDirPath)                                              = 0;
        virtual bool deleteDir(dirpath_t const& szDirPath)                                              = 0;

        virtual bool setDirTime(dirpath_t const& szDirPath, filetimes_t const& ftimes) = 0;
        virtual bool getDirTime(dirpath_t const& szDirPath, filetimes_t& ftimes)       = 0;
        virtual bool setDirAttr(dirpath_t const& szDirPath, fileattrs_t const& attr)   = 0;
        virtual bool getDirAttr(dirpath_t const& szDirPath, fileattrs_t& attr)         = 0;

        virtual bool enumerate(dirpath_t const& szDirPath, enumerate_delegate_t& enumerator) = 0;
    };
}; // namespace ncore

#endif