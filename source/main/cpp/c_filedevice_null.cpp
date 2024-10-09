#include "ccore/c_target.h"

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

namespace ncore
{
    namespace nfs
    {
        class filedevice_null_t : public filedevice_t
        {
        public:
            filedevice_null_t() {}
            virtual ~filedevice_null_t() {}

            virtual void destruct(alloc_t* allocator) { allocator->destruct(this); }

            virtual bool canSeek() const { return false; }
            virtual bool canWrite() const { return false; }

            virtual bool getDeviceInfo(filedevice_t* device, u64& totalSpace, u64& freeSpace) const
            {
                totalSpace = 0;
                freeSpace  = 0;
                return true;
            }

            virtual bool openFile(const filepath_t& szFilename, EFileMode mode, EFileAccess access, EFileOp op, void*& nFileHandle)
            {
                nFileHandle = INVALID_FILE_HANDLE;
                return false;
            }
            virtual bool createFile(const filepath_t& szFilename, bool boRead, bool boWrite, void*& nFileHandle)
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

            virtual bool flushFile(void* nFileHandle) { return false; }
            virtual bool closeFile(void* nFileHandle) { return false; }

            virtual bool createStream(filepath_t const& szFilename, bool boRead, bool boWrite, stream_t& strm) { return false; }
            virtual bool closeStream(stream_t& strm) { return true; }

            virtual bool setLengthOfFile(void* nFileHandle, u64 inLength) { return false; }
            virtual bool getLengthOfFile(void* nFileHandle, u64& outLength)
            {
                outLength = 0;
                return false;
            }

            virtual bool setFileTime(const filepath_t& szFilename, const filetimes_t& ftimes) { return false; }
            virtual bool getFileTime(const filepath_t& szFilename, filetimes_t& ftimes)
            {
                ftimes = filetimes_t();
                return false;
            }
            virtual bool setFileAttr(const filepath_t& szFilename, const fileattrs_t& attr) { return false; }
            virtual bool getFileAttr(const filepath_t& szFilename, fileattrs_t& attr)
            {
                attr = fileattrs_t();
                return false;
            }

            virtual bool setFileTime(void* pHandle, filetimes_t const& times) { return false; }
            virtual bool getFileTime(void* pHandle, filetimes_t& outTimes)
            {
                outTimes = filetimes_t();
                return false;
            }

            virtual bool hasFile(const filepath_t& szFilename) { return false; }
            virtual bool moveFile(const filepath_t& szFilename, const filepath_t& szToFilename, bool boOverwrite) { return false; }
            virtual bool copyFile(const filepath_t& szFilename, const filepath_t& szToFilename, bool boOverwrite) { return false; }
            virtual bool deleteFile(const filepath_t& szFilename) { return false; }

            virtual bool openDir(dirpath_t const& szDirPath, void*& nDirHandle)
            {
                nDirHandle = INVALID_DIR_HANDLE;
                return false;
            }
            virtual bool hasDir(const dirpath_t& szDirPath) { return false; }
            virtual bool createDir(const dirpath_t& szDirPath) { return false; }
            virtual bool moveDir(const dirpath_t& szDirPath, const dirpath_t& szToDirPath, bool boOverwrite) { return false; }
            virtual bool copyDir(const dirpath_t& szDirPath, const dirpath_t& szToDirPath, bool boOverwrite) { return false; }
            virtual bool deleteDir(const dirpath_t& szDirPath) { return false; }

            virtual bool setDirTime(const dirpath_t& szDirPath, const filetimes_t& ftimes) { return false; }
            virtual bool getDirTime(const dirpath_t& szDirPath, filetimes_t& ftimes)
            {
                ftimes = filetimes_t();
                return false;
            }
            virtual bool setDirAttr(const dirpath_t& szDirPath, const fileattrs_t& attr) { return false; }
            virtual bool getDirAttr(const dirpath_t& szDirPath, fileattrs_t& attr)
            {
                attr = fileattrs_t();
                return false;
            }

            virtual bool enumerate(const dirpath_t& szDirPath, enumerate_delegate_t& enumerator) { return false; }

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
            bool seekCurrent(void* nFileHandle, u64 pos, u64& newPos)
            {
                newPos = pos;
                return false;
            }
            bool seekEnd(void* nFileHandle, u64 pos, u64& newPos)
            {
                newPos = pos;
                return false;
            }
        };

        filedevice_t* x_NullFileDevice()
        {
            static filedevice_null_t filedevice_null;
            return &filedevice_null;
        }
    } // namespace nfs
}; // namespace ncore
