#include "xbase/x_target.h"
#ifdef TARGET_MAC

#include "xbase/x_allocator.h"
#include "xbase/x_debug.h"
#include "xbase/x_limits.h"
#include "xbase/x_memory.h"
#include "xbase/x_runes.h"
#include "xbase/x_va_list.h"
#include "xbase/x_integer.h"

#include "xtime/x_datetime.h"

#include "xfilesystem/private/x_filedevice.h"
#include "xfilesystem/private/x_filesystem.h"
#include "xfilesystem/x_attributes.h"
#include "xfilesystem/x_fileinfo.h"
#include "xfilesystem/x_dirinfo.h"
#include "xfilesystem/x_filepath.h"
#include "xfilesystem/x_dirpath.h"
#include "xfilesystem/x_filesystem.h"

//@TODO: When xfiledevice_pc is working OK we can copy and implement the Mac/OSX version

namespace xcore
{
    class xfiledevice_mac : public xfiledevice
    {
    public:
        xalloc*  mAllocator;
        xdirpath mDrivePath;
        xbool    mCanWrite;

        XCORE_CLASS_PLACEMENT_NEW_DELETE

        xfiledevice_mac(xalloc* alloc, const xdirpath& pDrivePath, xbool boCanWrite) : mAllocator(alloc), mDrivePath(pDrivePath), mCanWrite(boCanWrite) {}
        virtual ~xfiledevice_mac() {}

        virtual bool canSeek() const { return true; }
        virtual bool canWrite() const { return mCanWrite; }

        virtual bool getDeviceInfo(u64& totalSpace, u64& freeSpace) const { return false; }

        virtual bool openFile(const xfilepath& szFilename, EFileMode mode, EFileAccess access, EFileOp op, void*& nFileHandle) { return false; }
        virtual bool createFile(const xfilepath& szFilename, bool boRead, bool boWrite, void*& nFileHandle) { return false; }
        virtual bool readFile(void* nFileHandle, u64 pos, void* buffer, u64 count, u64& outNumBytesRead) { return false; }
        virtual bool writeFile(void* nFileHandle, u64 pos, const void* buffer, u64 count, u64& outNumBytesWritten) { return false; }
        virtual bool closeFile(void* nFileHandle) { return false; }

        virtual bool createStream(xfilepath const& szFilename, bool boRead, bool boWrite, xstream*& strm) { return false; }
        virtual bool closeStream(xstream* strm) { return false; }

        virtual bool setLengthOfFile(void* nFileHandle, u64 inLength) { return false; }
        virtual bool getLengthOfFile(void* nFileHandle, u64& outLength) { return false; }

        virtual bool setFileTime(const xfilepath& szFilename, const xfiletimes& ftimes) { return false; }
        virtual bool getFileTime(const xfilepath& szFilename, xfiletimes& ftimes) { return false; }
        virtual bool setFileAttr(const xfilepath& szFilename, const xfileattrs& attr) { return false; }
        virtual bool getFileAttr(const xfilepath& szFilename, xfileattrs& attr) { return false; }

        virtual bool setFileTime(void* pHandle, xfiletimes const& times) { return false; }
        virtual bool getFileTime(void* pHandle, xfiletimes& outTimes) { return false; }

        virtual bool hasFile(const xfilepath& szFilename) { return false; }
        virtual bool moveFile(const xfilepath& szFilename, const xfilepath& szToFilename, bool boOverwrite) { return false; }
        virtual bool copyFile(const xfilepath& szFilename, const xfilepath& szToFilename, bool boOverwrite) { return false; }
        virtual bool deleteFile(const xfilepath& szFilename) { return false; }

        virtual bool hasDir(const xdirpath& szDirPath) { return false; }
        virtual bool createDir(const xdirpath& szDirPath) { return false; }
        virtual bool moveDir(const xdirpath& szDirPath, const xdirpath& szToDirPath, bool boOverwrite) { return false; }
        virtual bool copyDir(const xdirpath& szDirPath, const xdirpath& szToDirPath, bool boOverwrite) { return false; }
        virtual bool deleteDir(const xdirpath& szDirPath) { return false; }

        virtual bool setDirTime(const xdirpath& szDirPath, const xfiletimes& ftimes) { return false; }
        virtual bool getDirTime(const xdirpath& szDirPath, xfiletimes& ftimes) { return false; }
        virtual bool setDirAttr(const xdirpath& szDirPath, const xfileattrs& attr) { return false; }
        virtual bool getDirAttr(const xdirpath& szDirPath, xfileattrs& attr) { return false; }

        virtual bool enumerate(const xdirpath& szDirPath, enumerate_delegate& enumerator) { return false; }

        enum ESeekMode
        {
            __SEEK_ORIGIN  = 1,
            __SEEK_CURRENT = 2,
            __SEEK_END     = 3,
        };
        bool seek(void* nFileHandle, ESeekMode mode, u64 pos, u64& newPos);
        bool seekOrigin(void* nFileHandle, u64 pos, u64& newPos);
        bool seekCurrent(void* nFileHandle, u64 pos, u64& newPos);
        bool seekEnd(void* nFileHandle, u64 pos, u64& newPos);

        static void* sOpenDir(xdirpath const& szDirPath);
    };

    xfiledevice* x_CreateFileDeviceMac(xalloc* alloc, const xdirpath& pDrivePath, xbool boCanWrite)
    {
        xfiledevice_mac* file_device = alloc->construct<xfiledevice_mac>(alloc, pDrivePath, boCanWrite);
        return file_device;
    }

    void x_DestroyFileDeviceMac(xfiledevice* device)
    {
        xfiledevice_mac* file_device = (xfiledevice_mac*)device;
        file_device->mAllocator->destruct(file_device);
    }

    xfiledevice* x_CreateFileDevice(xalloc* allocator, utf32::crunes& pDrivePath, xbool boCanWrite)
    {
        xdirpath drivePath = xfilesystem::dirpath(pDrivePath);
        return x_CreateFileDeviceMac(allocator, drivePath, boCanWrite);
    }

    void x_DestroyFileDevice(xfiledevice* fd) { x_DestroyFileDeviceMac(fd); }
} // namespace xcore
#endif // TARGET_MAC