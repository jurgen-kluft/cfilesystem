#include "ccore/c_target.h"
#ifdef TARGET_MAC

#include "cbase/c_allocator.h"
#include "ccore/c_debug.h"
#include "cbase/c_limits.h"
#include "cbase/c_memory.h"
#include "cbase/c_runes.h"
#include "cbase/c_va_list.h"
#include "cbase/c_integer.h"

#include "ctime/c_datetime.h"

#include "cfilesystem/private/c_filedevice.h"
#include "cfilesystem/private/c_filesystem.h"
#include "cfilesystem/c_attributes.h"
#include "cfilesystem/c_fileinfo.h"
#include "cfilesystem/c_dirinfo.h"
#include "cfilesystem/c_filepath.h"
#include "cfilesystem/c_dirpath.h"
#include "cfilesystem/c_filesystem.h"

//@TODO: When filedevice_pc_t is working OK we can copy and implement the Mac/OSX version

namespace ncore
{
    class xfiledevice_mac : public filedevice_t
    {
    public:
        alloc_t*  mAllocator;
        dirpath_t mDrivePath;
        bool      mCanWrite;

        DCORE_CLASS_PLACEMENT_NEW_DELETE

        xfiledevice_mac(alloc_t* alloc, const dirpath_t& pDrivePath, bool boCanWrite) : mAllocator(alloc), mDrivePath(pDrivePath), mCanWrite(boCanWrite) {}
        virtual ~xfiledevice_mac() {}

        virtual bool canSeek() const { return true; }
        virtual bool canWrite() const { return mCanWrite; }

        virtual bool getDeviceInfo(pathdevice_t* device, u64& totalSpace, u64& freeSpace) const { return false; }

        virtual bool openFile(const filepath_t& szFilename, EFileMode mode, EFileAccess access, EFileOp op, void*& nFileHandle) { return false; }
        virtual bool createFile(const filepath_t& szFilename, bool boRead, bool boWrite, void*& nFileHandle) { return false; }
        virtual bool readFile(void* nFileHandle, u64 pos, void* buffer, u64 count, u64& outNumBytesRead) { return false; }
        virtual bool writeFile(void* nFileHandle, u64 pos, const void* buffer, u64 count, u64& outNumBytesWritten) { return false; }
        virtual bool closeFile(void* nFileHandle) { return false; }

        virtual bool createStream(filepath_t const& szFilename, bool boRead, bool boWrite, stream_t& strm) { return false; }
        virtual bool closeStream(stream_t& strm) { return false; }

        virtual bool setLengthOfFile(void* nFileHandle, u64 inLength) { return false; }
        virtual bool getLengthOfFile(void* nFileHandle, u64& outLength) { return false; }

        virtual bool setFileTime(const filepath_t& szFilename, const filetimes_t& ftimes) { return false; }
        virtual bool getFileTime(const filepath_t& szFilename, filetimes_t& ftimes) { return false; }
        virtual bool setFileAttr(const filepath_t& szFilename, const fileattrs_t& attr) { return false; }
        virtual bool getFileAttr(const filepath_t& szFilename, fileattrs_t& attr) { return false; }

        virtual bool setFileTime(void* pHandle, filetimes_t const& times) { return false; }
        virtual bool getFileTime(void* pHandle, filetimes_t& outTimes) { return false; }

        virtual bool hasFile(const filepath_t& szFilename) { return false; }
        virtual bool moveFile(const filepath_t& szFilename, const filepath_t& szToFilename, bool boOverwrite) { return false; }
        virtual bool copyFile(const filepath_t& szFilename, const filepath_t& szToFilename, bool boOverwrite) { return false; }
        virtual bool deleteFile(const filepath_t& szFilename) { return false; }

        virtual bool hasDir(const dirpath_t& szDirPath) { return false; }
        virtual bool createDir(const dirpath_t& szDirPath) { return false; }
        virtual bool moveDir(const dirpath_t& szDirPath, const dirpath_t& szToDirPath, bool boOverwrite) { return false; }
        virtual bool copyDir(const dirpath_t& szDirPath, const dirpath_t& szToDirPath, bool boOverwrite) { return false; }
        virtual bool deleteDir(const dirpath_t& szDirPath) { return false; }

        virtual bool setDirTime(const dirpath_t& szDirPath, const filetimes_t& ftimes) { return false; }
        virtual bool getDirTime(const dirpath_t& szDirPath, filetimes_t& ftimes) { return false; }
        virtual bool setDirAttr(const dirpath_t& szDirPath, const fileattrs_t& attr) { return false; }
        virtual bool getDirAttr(const dirpath_t& szDirPath, fileattrs_t& attr) { return false; }

        virtual bool enumerate(const dirpath_t& szDirPath, enumerate_delegate_t& enumerator) { return false; }

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

        static void* sOpenDir(dirpath_t const& szDirPath);
    };

    filedevice_t* x_CreateFileDeviceMac(alloc_t* alloc, bool boCanWrite)
    {
        xfiledevice_mac* file_device = alloc->construct<xfiledevice_mac>(alloc, boCanWrite);
        return file_device;
    }

    void x_DestroyFileDeviceMac(filedevice_t* device)
    {
        xfiledevice_mac* file_device = (xfiledevice_mac*)device;
        file_device->mAllocator->destruct(file_device);
    }

    filedevice_t* x_CreateFileDevice(alloc_t* allocator, bool boCanWrite)
    {
        dirpath_t drivePath = filesystem_t::dirpath(pDrivePath);
        return x_CreateFileDeviceMac(allocator, drivePath, boCanWrite);
    }

    void x_DestroyFileDevice(filedevice_t* fd) { x_DestroyFileDeviceMac(fd); }
} // namespace ncore
#endif // TARGET_MAC