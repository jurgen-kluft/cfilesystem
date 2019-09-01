#include "xbase/x_target.h"

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

namespace xcore
{
    class xfiledevice_null : public xfiledevice
    {
    public:
        xfiledevice_null() {}
        virtual ~xfiledevice_mull() {}

        virtual bool canSeek() { return false; }
        virtual bool canWrite() { return false; }

        virtual bool getDeviceInfo(u64& totalSpace, u64& freeSpace)
        {
            totalSpace = 0;
            freeSpace  = 0;
            return true;
        }

        virtual bool openFile(const xfilepath& szFilename, EFileMode mode, EFileAccess access, EFileOp op, void*& nFileHandle)
        {
            nFileHandle = INVALID_FILE_HANDLE;
            return false;
        }
        virtual bool createFile(const xfilepath& szFilename, bool boRead, bool boWrite, void*& nFileHandle)
        {
            nFileHandle = INVALID_FILE_HANDLE;
            return false;
        }

        virtual bool readFile(void* nFileHandle, u64 pos, void* buffer, u64 count, u64& outNumBytesRead)
        {
            outNumBytesRead = 0;
            return false;
        }

        virtual bool writeFile(void* nFileHandle, u64 pos, const void* buffer, u64 count, u64& outNumBytesWritten)
        {
            outNumBytesWritten = 0;
            return false;
        }

        virtual bool closeFile(void* nFileHandle) { return false; }

        virtual bool createStream(xfilepath const& szFilename, bool boRead, bool boWrite, xstream*& strm)
        {
            strm = nullptr;
            return false;
        }
        virtual bool closeStream(xstream* strm) { return true; }

        virtual bool setLengthOfFile(void* nFileHandle, u64 inLength) { return false; }
        virtual bool getLengthOfFile(void* nFileHandle, u64& outLength)
        {
            outLength = 0;
            return false;
        }

        virtual bool setFileTime(const xfilepath& szFilename, const xfiletimes& ftimes) { return false; }
        virtual bool getFileTime(const xfilepath& szFilename, xfiletimes& ftimes)
        {
            ftimes = xfiletimes();
            return false;
        }
        virtual bool setFileAttr(const xfilepath& szFilename, const xfileattrs& attr) { return false; }
        virtual bool getFileAttr(const xfilepath& szFilename, xfileattrs& attr)
        {
            attr = xfileattrs();
            return false;
        }

        virtual bool setFileTime(void* pHandle, xfiletimes const& times) { return false; }
        virtual bool getFileTime(void* pHandle, xfiletimes& outTimes)
        {
            outTimes = xfiletimes();
            return false;
        }

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
        virtual bool getDirTime(const xdirpath& szDirPath, xfiletimes& ftimes)
        {
            ftimes = xfiletimes();
            return false;
        }
        virtual bool setDirAttr(const xdirpath& szDirPath, const xfileattrs& attr) { return false; }
        virtual bool getDirAttr(const xdirpath& szDirPath, xfileattrs& attr)
        {
            attr = xfileattrs();
            return false;
        }

        virtual bool enumerate(const xdirpath& szDirPath, enumerate_delegate& enumerator) { return false; }

        enum ESeekMode
        {
            __SEEK_ORIGIN  = 1,
            __SEEK_CURRENT = 2,
            __SEEK_END     = 3,
        };
        bool seek(void* nFileHandle, ESeekMode mode, u64 pos, u64& newPos)
        {
            newPos = pos;
            return false;
        }

        bool seekOrigin(void* nFileHandle, u64 pos, u64& newPos)
        {
            newPos = pos;
            return false;
        }
        bool seekCurrent(void* nFileHandle, u64 pos, u64& newPos);
        {
            newPos = pos;
            return false;
        }
        bool seekEnd(void* nFileHandle, u64 pos, u64& newPos);
        {
            newPos = pos;
            return false;
        }

        static HANDLE sOpenDir(xdirpath const& szDirPath) { return nullptr; }
    };

    xfiledevice* x_NullFileDevice()
    {
        static xfiledevice_null filedevice_null();
        return &filedevice_null;
    }

}; // namespace xcore

#endif
