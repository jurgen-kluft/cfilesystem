#include "xbase/x_target.h"
#if defined TARGET_PC

//==============================================================================
// INCLUDES
//==============================================================================
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOKANJI
#include <windows.h>
#include <stdio.h>

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
#include "xfilesystem/x_filepath.h"
#include "xfilesystem/x_dirpath.h"
#include "xfilesystem/x_filesystem.h"

namespace ncore
{
    class filedevice_pc_t : public filedevice_t
    {
    public:

        XCORE_CLASS_PLACEMENT_NEW_DELETE

        filedevice_pc_t() {}
        virtual ~filedevice_pc_t() {}

        virtual void destruct(alloc_t* allocator)
        {
            
        }

        virtual bool canSeek() const { return true; }
        virtual bool canWrite() const { return true; }

        virtual bool getDeviceInfo(pathdevice_t* device, u64& totalSpace, u64& freeSpace) const;

        virtual bool openFile(const filepath_t& szFilename, EFileMode mode, EFileAccess access, EFileOp op, void*& nFileHandle);
        virtual bool createFile(const filepath_t& szFilename, bool boRead, bool boWrite, void*& nFileHandle);
        virtual bool readFile(void* nFileHandle, u64 pos, void* buffer, u64 count, u64& outNumBytesRead);
        virtual bool writeFile(void* nFileHandle, u64 pos, const void* buffer, u64 count, u64& outNumBytesWritten);
        virtual bool flushFile(void* nFileHandle);
        virtual bool closeFile(void* nFileHandle);

        virtual bool createStream(filepath_t const& szFilename, bool boRead, bool boWrite, stream_t& strm);
        virtual bool closeStream(stream_t& strm);

        virtual bool setLengthOfFile(void* nFileHandle, u64 inLength);
        virtual bool getLengthOfFile(void* nFileHandle, u64& outLength);

        virtual bool setFileTime(const filepath_t& szFilename, const filetimes_t& ftimes);
        virtual bool getFileTime(const filepath_t& szFilename, filetimes_t& ftimes);
        virtual bool setFileAttr(const filepath_t& szFilename, const fileattrs_t& attr);
        virtual bool getFileAttr(const filepath_t& szFilename, fileattrs_t& attr);

        virtual bool setFileTime(void* pHandle, filetimes_t const& times);
        virtual bool getFileTime(void* pHandle, filetimes_t& outTimes);

        virtual bool hasFile(const filepath_t& szFilename);
        virtual bool moveFile(const filepath_t& szFilename, const filepath_t& szToFilename, bool boOverwrite);
        virtual bool copyFile(const filepath_t& szFilename, const filepath_t& szToFilename, bool boOverwrite);
        virtual bool deleteFile(const filepath_t& szFilename);

        virtual bool openDir(const dirpath_t& szDirPath, void*& nDirHandle);
        virtual bool hasDir(const dirpath_t& szDirPath);
        virtual bool createDir(const dirpath_t& szDirPath);
        virtual bool moveDir(const dirpath_t& szDirPath, const dirpath_t& szToDirPath, bool boOverwrite);
        virtual bool copyDir(const dirpath_t& szDirPath, const dirpath_t& szToDirPath, bool boOverwrite);
        virtual bool deleteDir(const dirpath_t& szDirPath);

        virtual bool setDirTime(const dirpath_t& szDirPath, const filetimes_t& ftimes);
        virtual bool getDirTime(const dirpath_t& szDirPath, filetimes_t& ftimes);
        virtual bool setDirAttr(const dirpath_t& szDirPath, const fileattrs_t& attr);
        virtual bool getDirAttr(const dirpath_t& szDirPath, fileattrs_t& attr);

        virtual bool enumerate(const dirpath_t& szDirPath, enumerate_delegate_t& enumerator);

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
    };

    class filedevice_pc_ro_t : public filedevice_pc_t
    {
    public:
        virtual bool canWrite() const { return false; }
    };

    static filedevice_pc_t sFileDevicePCReadWrite;
    static filedevice_pc_ro_t sFileDevicePCReadOnly;

    filedevice_t* x_CreateFileDevicePC(bool boCanWrite)
    {
        if (boCanWrite)
            return &sFileDevicePCReadWrite;
        return &sFileDevicePCReadOnly;
    }

    void x_DestroyFileDevicePC(filedevice_t* device) {  }

    filedevice_t* x_CreateFileDevice(bool boCanWrite)
    {
        return x_CreateFileDevicePC(boCanWrite);
    }

    void x_DestroyFileDevice(filedevice_t* fd) { x_DestroyFileDevicePC(fd); }

    bool filedevice_pc_t::getDeviceInfo(pathdevice_t* device, u64& totalSpace, u64& freeSpace) const
    {
        ULARGE_INTEGER totalbytes, freebytes;
        
        alloc_t* allocator = device->m_root->m_allocator;

        s32 const drivepathstrlen = (device->m_devicePath->m_len + 1) ;
        utf16::prune drivepathstr = (utf16::prune)allocator->allocate(drivepathstrlen * sizeof(utf16::rune));
        runes_t drivepath(drivepathstr, drivepathstr + drivepathstrlen);
        device->to_string(drivepath);

        bool result = false;
        if (GetDiskFreeSpaceExW((LPCWSTR)drivepath.str16(), nullptr, &totalbytes, &freebytes) != 0)
        {
            freeSpace  = freebytes.QuadPart;
            totalSpace = totalbytes.QuadPart;
            result     = true;
        }

        allocator->deallocate(drivepathstr);
        return result;
    }

    bool filedevice_pc_t::hasFile(const filepath_t& szFilename) 
    {
        u32 shareType   = FILE_SHARE_READ;
        u32 fileMode    = GENERIC_READ;
        u32 disposition = OPEN_EXISTING;
        u32 attrFlags   = FILE_ATTRIBUTE_NORMAL;

        alloc_t* allocator = szFilename.m_dirpath.m_device->m_root->m_allocator;

        s32 const filenamestrlen = szFilename.to_strlen();
        utf16::prune filenamestr = (utf16::prune)allocator->allocate(filenamestrlen * sizeof(utf16::rune));
        runes_t filename16(filenamestr, filenamestr + filenamestrlen);
        szFilename.to_string(filename16);

        bool   result      = false;
        HANDLE nFileHandle = ::CreateFileW(LPCWSTR(filename16.str16()), fileMode, shareType, nullptr, disposition, attrFlags, nullptr);
        if (nFileHandle != INVALID_HANDLE_VALUE)
        {
            ::CloseHandle((HANDLE)nFileHandle);
            result = true;
        }

        allocator->deallocate(filenamestr);
        return result;
    }

    bool filedevice_pc_t::openFile(const filepath_t& szFilename, EFileMode mode, EFileAccess access, EFileOp op, void*& nFileHandle)
    {
        u32 shareType   = FILE_SHARE_READ;
        u32 fileMode    = (access == FileAccess_Read) ? GENERIC_READ : GENERIC_WRITE | GENERIC_READ;
        u32 disposition = OPEN_EXISTING;
        u32 attrFlags   = FILE_ATTRIBUTE_NORMAL;

        alloc_t* allocator = szFilename.m_dirpath.m_device->m_root->m_allocator;

        s32 const filenamestrlen = szFilename.to_strlen();
        utf16::prune filenamestr = (utf16::prune)allocator->allocate(filenamestrlen * sizeof(utf16::rune));
        runes_t filename16(filenamestr, filenamestr + filenamestrlen);
        szFilename.to_string(filename16);

        HANDLE handle = ::CreateFileW(LPCWSTR(filename16.str16()), fileMode, shareType, nullptr, disposition, attrFlags, nullptr);
        nFileHandle   = handle;

        allocator->deallocate(filenamestr);
        return nFileHandle != INVALID_HANDLE_VALUE;
    }

    bool filedevice_pc_t::createFile(const filepath_t& szFilename, bool boRead, bool boWrite, void*& nFileHandle)
    {
        u32 shareType   = FILE_SHARE_READ;
        u32 fileMode    = !boWrite ? GENERIC_READ : GENERIC_WRITE | GENERIC_READ;
        u32 disposition = CREATE_ALWAYS;
        u32 attrFlags   = FILE_ATTRIBUTE_NORMAL;

        alloc_t* allocator = szFilename.m_dirpath.m_device->m_root->m_allocator;

        s32 const filenamestrlen = szFilename.to_strlen();
        utf16::prune filenamestr = (utf16::prune)allocator->allocate(filenamestrlen * sizeof(utf16::rune));
        runes_t filename16(filenamestr, filenamestr + filenamestrlen);
        szFilename.to_string(filename16);

        HANDLE handle = ::CreateFileW((LPCWSTR)filename16.str16(), fileMode, shareType, nullptr, disposition, attrFlags, nullptr);
        nFileHandle   = handle;

        allocator->deallocate(filenamestr);
        return nFileHandle != INVALID_HANDLE_VALUE;
    }

    bool filedevice_pc_t::readFile(void* nFileHandle, u64 pos, void* buffer, u64 count, u64& outNumBytesRead)
    {
        u64 newPos;
        if (seek(nFileHandle, __SEEK_ORIGIN, pos, newPos))
        {
            DWORD numBytesRead;
            bool  boSuccess = ::ReadFile((HANDLE)nFileHandle, buffer, (DWORD)count, &numBytesRead, nullptr);

            if (boSuccess)
            {
                outNumBytesRead = numBytesRead;
            }

            if (!boSuccess)
            {
                outNumBytesRead = -1;

                DWORD dwError = ::GetLastError();
                switch (dwError)
                {
                    case ERROR_HANDLE_EOF: // We have reached the end of the FilePC during the call to ReadFile
                        return false;
                    case ERROR_IO_PENDING: return false;
                    default: return false;
                }
            }

            return true;
        }
        return false;
    }
    bool filedevice_pc_t::writeFile(void* nFileHandle, u64 pos, const void* buffer, u64 count, u64& outNumBytesWritten)
    {
        u64 newPos;
        if (seek(nFileHandle, __SEEK_ORIGIN, pos, newPos))
        {
            DWORD numBytesWritten;
            bool  boSuccess = ::WriteFile((HANDLE)nFileHandle, buffer, (DWORD)count, &numBytesWritten, nullptr);

            if (boSuccess)
            {
                outNumBytesWritten = numBytesWritten;
            }

            if (!boSuccess)
            {
                outNumBytesWritten = -1;

                DWORD dwError = ::GetLastError();
                switch (dwError)
                {
                    case ERROR_HANDLE_EOF: // We have reached the end of the FilePC during the call to WriteFile
                        return false;
                    case ERROR_IO_PENDING: return false;
                    default: return false;
                }
            }

            return true;
        }
        return false;
    }

    bool filedevice_pc_t::moveFile(const filepath_t& szFilename, const filepath_t& szToFilename, bool boOverwrite)
    {
        if (!canWrite())
            return false;

        alloc_t* allocator = szFilename.m_dirpath.m_device->m_root->m_allocator;

        s32 const filenamestrlen = szFilename.to_strlen();
        utf16::prune filenamestr = (utf16::prune)allocator->allocate(filenamestrlen * sizeof(utf16::rune));
        runes_t filename16(filenamestr, filenamestr + filenamestrlen);
        szFilename.to_string(filename16);

        s32 const tofilenamestrlen = szToFilename.to_strlen();
        utf16::prune tofilenamestr = (utf16::prune)allocator->allocate(tofilenamestrlen * sizeof(utf16::rune));
        runes_t tofilename16(tofilenamestr, tofilenamestr + tofilenamestrlen);
        szToFilename.to_string(tofilename16);

        BOOL result = ::MoveFileW((LPCWSTR)filename16.str16(), (LPCWSTR)tofilename16.m_utf16.m_str) != 0;

        allocator->deallocate(filenamestr);
        allocator->deallocate(tofilenamestr);
        return result;
    }

    bool filedevice_pc_t::copyFile(const filepath_t& szFilename, const filepath_t& szToFilename, bool boOverwrite)
    {
        if (!canWrite())
            return false;

        alloc_t* allocator = szFilename.m_dirpath.m_device->m_root->m_allocator;

        const bool failIfExists = boOverwrite == false;

        s32 const filenamestrlen = szFilename.to_strlen();
        utf16::prune filenamestr = (utf16::prune)allocator->allocate(filenamestrlen * sizeof(utf16::rune));
        runes_t filename16(filenamestr, filenamestr + filenamestrlen);
        szFilename.to_string(filename16);

        s32 const tofilenamestrlen = szToFilename.to_strlen();
        utf16::prune tofilenamestr = (utf16::prune)allocator->allocate(tofilenamestrlen * sizeof(utf16::rune));
        runes_t tofilename16(tofilenamestr, tofilenamestr + tofilenamestrlen);
        szToFilename.to_string(tofilename16);

        BOOL result = ::CopyFileW((LPCWSTR)filename16.str16(), (LPCWSTR)tofilename16.m_utf16.m_str, failIfExists) != 0;

        allocator->deallocate(filenamestr);
        allocator->deallocate(tofilenamestr);
        return result == TRUE;
    }

    bool filedevice_pc_t::flushFile(void* nFileHandle)
    {
        return true;
    }

    bool filedevice_pc_t::closeFile(void* nFileHandle)
    {
        if (!::CloseHandle((HANDLE)nFileHandle))
            return false;
        return true;
    }

    //@todo: implement create and close stream
    bool filedevice_pc_t::createStream(filepath_t const& szFilename, bool boRead, bool boWrite, stream_t& strm) { return false; }
    bool filedevice_pc_t::closeStream(stream_t& strm) { return false; }

    bool filedevice_pc_t::deleteFile(const filepath_t& szFilename)
    {
        alloc_t* allocator = szFilename.m_dirpath.m_device->m_root->m_allocator;

        s32 const filenamestrlen = szFilename.to_strlen();
        utf16::prune filenamestr = (utf16::prune)allocator->allocate(filenamestrlen * sizeof(utf16::rune));
        runes_t filename16(filenamestr, filenamestr + filenamestrlen);
        szFilename.to_string(filename16);

        bool result = false;
        if (::DeleteFileW(LPCWSTR(filename16.str16())))
        {
            result = true;
        }

        allocator->deallocate(filenamestr);
        return result;
    }

    bool filedevice_pc_t::setLengthOfFile(void* nFileHandle, u64 inLength)
    {
        uint_t distanceLow  = (uint_t)inLength;
        uint_t distanceHigh = (uint_t)(inLength >> 32);
        ::SetFilePointer((HANDLE)nFileHandle, (LONG)distanceLow, (PLONG)&distanceHigh, FILE_BEGIN);
        ::SetEndOfFile((HANDLE)nFileHandle);
        return true;
    }

    bool filedevice_pc_t::getLengthOfFile(void* nFileHandle, u64& outLength)
    {
        DWORD lowSize, highSize;
        lowSize   = ::GetFileSize((HANDLE)nFileHandle, &highSize);
        outLength = highSize;
        outLength = outLength << 16;
        outLength = outLength << 16;
        outLength = outLength | lowSize;
        return true;
    }

    bool filedevice_pc_t::setFileTime(const filepath_t& szFilename, const filetimes_t& ftimes)
    {
        void* nFileHandle;
        if (openFile(szFilename, FileMode_Open, FileAccess_Read, FileOp_Sync, nFileHandle))
        {
            datetime_t creationTime;
            ftimes.getCreationTime(creationTime);
            datetime_t lastAccessTime;
            ftimes.getLastAccessTime(lastAccessTime);
            datetime_t lastWriteTime;
            ftimes.getLastWriteTime(lastWriteTime);

            FILETIME _creationTime;
            u64      uCreationTime       = creationTime.toFileTime();
            _creationTime.dwHighDateTime = nmem::hiu32(uCreationTime);
            _creationTime.dwLowDateTime  = nmem::lou32(uCreationTime);

            FILETIME _lastAccessTime;
            u64      uLastAccessTime       = lastAccessTime.toFileTime();
            _lastAccessTime.dwHighDateTime = nmem::hiu32(uLastAccessTime);
            _lastAccessTime.dwLowDateTime  = nmem::lou32(uLastAccessTime);

            FILETIME _lastWriteTime;
            u64      uLastWriteTime       = lastWriteTime.toFileTime();
            _lastWriteTime.dwHighDateTime = nmem::hiu32(uLastWriteTime);
            _lastWriteTime.dwLowDateTime  = nmem::lou32(uLastWriteTime);

            ::SetFileTime((HANDLE)nFileHandle, &_creationTime, &_lastAccessTime, &_lastWriteTime);
            closeFile(nFileHandle);
            return true;
        }
        return false;
    }

    bool filedevice_pc_t::getFileTime(const filepath_t& szFilename, filetimes_t& ftimes)
    {
        void* nFileHandle;
        if (openFile(szFilename, FileMode_Open, FileAccess_Read, FileOp_Sync, nFileHandle))
        {
            FILETIME _creationTime;
            FILETIME _lastAccessTime;
            FILETIME _lastWriteTime;
            ::GetFileTime((HANDLE)nFileHandle, &_creationTime, &_lastAccessTime, &_lastWriteTime);
            datetime_t CreationTime   = datetime_t::sFromFileTime((u64)nmem::makeu64(_creationTime.dwLowDateTime, _creationTime.dwHighDateTime));
            datetime_t LastAccessTime = datetime_t::sFromFileTime((u64)nmem::makeu64(_lastAccessTime.dwLowDateTime, _lastAccessTime.dwHighDateTime));
            datetime_t LastWriteTime  = datetime_t::sFromFileTime((u64)nmem::makeu64(_lastWriteTime.dwLowDateTime, _lastWriteTime.dwHighDateTime));
            ftimes.setCreationTime(CreationTime);
            ftimes.setLastAccessTime(LastAccessTime);
            ftimes.setLastWriteTime(LastWriteTime);
            closeFile(nFileHandle);
            return true;
        }
        return false;
    }

    bool filedevice_pc_t::setFileAttr(const filepath_t& szFilename, const fileattrs_t& attr)
    {
        DWORD dwFileAttributes = 0;
        if (attr.isArchive())
            dwFileAttributes = dwFileAttributes | FILE_ATTRIBUTE_ARCHIVE;
        if (attr.isReadOnly())
            dwFileAttributes = dwFileAttributes | FILE_ATTRIBUTE_READONLY;
        if (attr.isHidden())
            dwFileAttributes = dwFileAttributes | FILE_ATTRIBUTE_HIDDEN;
        if (attr.isSystem())
            dwFileAttributes = dwFileAttributes | FILE_ATTRIBUTE_SYSTEM;

        alloc_t* allocator = szFilename.m_dirpath.m_device->m_root->m_allocator;

        s32 const filenamestrlen = szFilename.to_strlen();
        utf16::prune filenamestr = (utf16::prune)allocator->allocate(filenamestrlen * sizeof(utf16::rune));
        runes_t filename16(filenamestr, filenamestr + filenamestrlen);
        szFilename.to_string(filename16);

        bool result = ::SetFileAttributesW(LPCWSTR(filename16.str16()), dwFileAttributes) == TRUE;

        allocator->deallocate(filenamestr);
        return result;
    }

    bool filedevice_pc_t::getFileAttr(const filepath_t& szFilename, fileattrs_t& attr)
    {
        DWORD dwFileAttributes = 0;

        alloc_t* allocator = szFilename.m_dirpath.m_device->m_root->m_allocator;

        s32 const filenamestrlen = szFilename.to_strlen();
        utf16::prune filenamestr = (utf16::prune)allocator->allocate(filenamestrlen * sizeof(utf16::rune));
        runes_t filename16(filenamestr, filenamestr + filenamestrlen);
        szFilename.to_string(filename16);

        bool result      = false;
        dwFileAttributes = ::GetFileAttributesW(LPCWSTR(filename16.str16()));
        if (dwFileAttributes != INVALID_FILE_ATTRIBUTES)
        {
            result = true;
            attr.setArchive(dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE);
            attr.setReadOnly(dwFileAttributes & FILE_ATTRIBUTE_READONLY);
            attr.setHidden(dwFileAttributes & FILE_ATTRIBUTE_HIDDEN);
            attr.setSystem(dwFileAttributes & FILE_ATTRIBUTE_SYSTEM);
        }

        allocator->deallocate(filenamestr);
        return result;
    }

    bool filedevice_pc_t::setFileTime(void* nFileHandle, const filetimes_t& ftimes)
    {
        datetime_t creationTime;
        ftimes.getCreationTime(creationTime);
        datetime_t lastAccessTime;
        ftimes.getLastAccessTime(lastAccessTime);
        datetime_t lastWriteTime;
        ftimes.getLastWriteTime(lastWriteTime);

        FILETIME _creationTime;
        u64      uCreationTime       = creationTime.toFileTime();
        _creationTime.dwHighDateTime = nmem::hiu32(uCreationTime);
        _creationTime.dwLowDateTime  = nmem::lou32(uCreationTime);

        FILETIME _lastAccessTime;
        u64      uLastAccessTime       = lastAccessTime.toFileTime();
        _lastAccessTime.dwHighDateTime = nmem::hiu32(uLastAccessTime);
        _lastAccessTime.dwLowDateTime  = nmem::lou32(uLastAccessTime);

        FILETIME _lastWriteTime;
        u64      uLastWriteTime       = lastWriteTime.toFileTime();
        _lastWriteTime.dwHighDateTime = nmem::hiu32(uLastWriteTime);
        _lastWriteTime.dwLowDateTime  = nmem::lou32(uLastWriteTime);

        ::SetFileTime((HANDLE)nFileHandle, &_creationTime, &_lastAccessTime, &_lastWriteTime);
        return true;
    }

    bool filedevice_pc_t::getFileTime(void* nFileHandle, filetimes_t& ftimes)
    {
        FILETIME _creationTime;
        FILETIME _lastAccessTime;
        FILETIME _lastWriteTime;
        ::GetFileTime((HANDLE)nFileHandle, &_creationTime, &_lastAccessTime, &_lastWriteTime);
        datetime_t CreationTime   = datetime_t::sFromFileTime((u64)nmem::makeu64(_creationTime.dwLowDateTime, _creationTime.dwHighDateTime));
        datetime_t LastAccessTime = datetime_t::sFromFileTime((u64)nmem::makeu64(_lastAccessTime.dwLowDateTime, _lastAccessTime.dwHighDateTime));
        datetime_t LastWriteTime  = datetime_t::sFromFileTime((u64)nmem::makeu64(_lastWriteTime.dwLowDateTime, _lastWriteTime.dwHighDateTime));
        ftimes.setCreationTime(CreationTime);
        ftimes.setLastAccessTime(LastAccessTime);
        ftimes.setLastWriteTime(LastWriteTime);
        return true;
    }

    bool filedevice_pc_t::openDir(dirpath_t const& szDirPath, void*& nDirHandle)
    {
        u32 shareType   = FILE_SHARE_READ;
        u32 fileMode    = GENERIC_READ | GENERIC_WRITE;
        u32 disposition = OPEN_EXISTING;
        u32 attrFlags   = FILE_FLAG_BACKUP_SEMANTICS;

        alloc_t* allocator = szDirPath.m_device->m_root->m_allocator;

        s32 const pathstrlen = szDirPath.to_strlen();
        utf16::prune pathstr = (utf16::prune)allocator->allocate(pathstrlen * sizeof(utf16::rune));
        runes_t path16(pathstr, pathstr + pathstrlen);
        szDirPath.to_string(path16);

        nDirHandle = ::CreateFileW((LPCWSTR)path16.str16(), fileMode, shareType, nullptr, disposition, attrFlags, nullptr);

        allocator->deallocate(pathstr);
        return nDirHandle != INVALID_HANDLE_VALUE;
    }

    static void sCloseDir(HANDLE handle) { ::CloseHandle(handle); }

    bool filedevice_pc_t::hasDir(const dirpath_t& szDirPath)
    {
        void* handle;
        openDir(szDirPath, handle);
        if (handle == INVALID_HANDLE_VALUE)
            return false;
        sCloseDir(handle);
        return true;
    }

    bool filedevice_pc_t::createDir(const dirpath_t& szDirPath)
    {
        alloc_t* allocator = szDirPath.m_device->m_root->m_allocator;

        s32 const pathstrlen = szDirPath.to_strlen();
        utf16::prune pathstr = (utf16::prune)allocator->allocate(pathstrlen * sizeof(utf16::rune));
        runes_t path16(pathstr, pathstr + pathstrlen);
        szDirPath.to_string(path16);

        BOOL result = ::CreateDirectoryW(LPCWSTR(path16.str16()), nullptr) != 0;

        allocator->deallocate(pathstr);
        return result;
    }

    bool filedevice_pc_t::moveDir(const dirpath_t& szDirPath, const dirpath_t& szToDirPath, bool boOverwrite)
    {
        u32 const dwFlags = MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED;

        alloc_t* allocator = szDirPath.m_device->m_root->m_allocator;

        s32 const pathstrlen = szDirPath.to_strlen();
        utf16::prune pathstr = (utf16::prune)allocator->allocate(pathstrlen * sizeof(utf16::rune));
        runes_t path16(pathstr, pathstr + pathstrlen);
        szDirPath.to_string(path16);

        s32 const topathstrlen = szDirPath.to_strlen();
        utf16::prune topathstr = (utf16::prune)allocator->allocate(topathstrlen * sizeof(utf16::rune));
        runes_t topath16(topathstr, topathstr + topathstrlen);
        szToDirPath.to_string(topath16);

        BOOL result = ::MoveFileExW((LPCWSTR)path16.str16(), (LPCWSTR)topath16.m_utf16.m_str, dwFlags) != 0;
        return result;
    }

    static void changeDirPath(const dirpath_t& src, const dirpath_t& dst, dirpath_t const& src_dir, dirpath_t& result)
    {
        dirpath_t srcdirpath(src);
        dirpath_t curdirpath = src_dir;
        curdirpath.makeRelativeTo(srcdirpath);
        result = dst + curdirpath;
    }

    static void changeFilePath(const dirpath_t& src, const dirpath_t& dst, filepath_t const& src_filepath, filepath_t& result)
    {
        dirpath_t  srcdirpath(src);
        filepath_t curfilepath(src_filepath);
        curfilepath.makeRelativeTo(srcdirpath);
        result = dst + curfilepath;
    }

    static bool sIsDots(LPCWSTR str) { return (str[0] == '.' && str[1] == '\0') || (str[0] == '.' && str[1] == '.' && str[2] == '\0'); }

    struct dirwalker
    {
        class node
        {
        public:
            HANDLE           mFindHandle;
            WIN32_FIND_DATAW mFindData;
            node*            mPrev;

            XCORE_CLASS_PLACEMENT_NEW_DELETE
        };

        alloc_t* mNodeHeap;
        node*    mDirStack;
        s32      mLevel;

        filesys_t* mSysRoot;
        runes_t    mPathStr;

        dirpath_t  mDirInfo;
        
        filepath_t mFilePath;
        fileattrs_t mFileAttrs;
        filetimes_t mFileTimes;

        dirwalker(alloc_t* allocator, dirpath_t const& dirpath) : mNodeHeap(nullptr)
        {
            mNodeHeap = allocator;
            mFilePath.setDirpath(dirpath);
        }

        bool next() { return ::FindNextFileW(mDirStack->mFindHandle, &mDirStack->mFindData); }
        inline bool is_dots() const { return (sIsDots(mDirStack->mFindData.cFileName)); }
        inline bool is_dir() const { return ((mDirStack->mFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)); }

        bool push_dir()
        {
            runes_t dirname;
            dirname.m_utf16.m_str = (utf16::prune)mDirStack->mFindData.cFileName;
            dirname.m_utf16.m_end = dirname.m_utf16.m_str;
            while (*dirname.m_utf16.m_end != '\0')
            {
                dirname.m_utf16.m_end++;
            }

            mFilePath.down(mSysRoot->register_dirname(dirname));

            // We have found a directory, enter
            if (!enter_dir())
            {
                mFilePath.up();
                return false; // Could not enter this directory
            }

            return true;
        }

        bool enumerate_dir(enumerate_delegate_t& enumerator) 
        {
            mDirInfo = mFilePath.dirpath();
            return (enumerator(mLevel, mDirInfo)); 
        }

        fileattrs_t build_fileattrs(WIN32_FIND_DATAW* nFileData)
        {
            fileattrs_t attrs;
            attrs.setHidden(nFileData->dwFileAttributes & FILE_ATTRIBUTE_HIDDEN);
            return attrs;
        }

        filetimes_t build_filetimes(WIN32_FIND_DATAW* nFileData)
        {
            u64 const  creationtime   = nmem::makeu64(nFileData->ftCreationTime.dwLowDateTime, nFileData->ftCreationTime.dwHighDateTime);
            datetime_t creationdt     = datetime_t::sFromFileTime(creationtime);
            u64 const  lastwritetime  = nmem::makeu64(nFileData->ftLastWriteTime.dwLowDateTime, nFileData->ftLastWriteTime.dwHighDateTime);
            datetime_t lastwritedt    = datetime_t::sFromFileTime(lastwritetime);
            u64 const  lastaccesstime = nmem::makeu64(nFileData->ftLastAccessTime.dwLowDateTime, nFileData->ftLastAccessTime.dwHighDateTime);
            datetime_t lastaccessdt   = datetime_t::sFromFileTime(lastaccesstime);

            return filetimes_t(creationdt, lastaccessdt, lastwritedt);
        }

        bool enumerate_file(enumerate_delegate_t& enumerator)
        {
            // Prepare FileInfo
            runes_t filename;
            filename.m_utf16.m_str = (utf16::prune)mDirStack->mFindData.cFileName;
            filename.m_utf16.m_end = filename.m_utf16.m_str;
            filename.m_utf16.m_eos = (utf16::prune)mDirStack->mFindData.cFileName[sizeof(mDirStack->mFindData.cFileName) - 1];
            while (filename.m_utf16.m_end < filename.m_utf16.m_eos && *filename.m_utf16.m_end != '\0')
            {
                filename.m_utf16.m_end++;
            }
            pathname_t* fname;
            pathname_t* fext;
            mSysRoot->register_filename(filename, fname, fext);

            fileattrs_t fileattrs = build_fileattrs(&mDirStack->mFindData);
            filetimes_t filetimes = build_filetimes(&mDirStack->mFindData);

            mFilePath.setFilename(fname);
            mFilePath.setExtension(fext);
            mFileAttrs = fileattrs;
            mFileTimes = filetimes;

            return (enumerator(mLevel, mFilePath, mFileAttrs, mFileTimes));
        }

        bool pop_dir()
        {
            if (::GetLastError() == ERROR_NO_MORE_FILES)
            {
                // No more files here
            }

            node* n   = mDirStack;
            mDirStack = n->mPrev;

            //@TODO: Some error occurred, close the handle and return FALSE
            ::FindClose(n->mFindHandle);

            mNodeHeap->destruct(n);

            // Move up to the parent directory
            mFilePath.up();

            return mDirStack != nullptr;
        }

        bool enter_dir()
        {
            node* nextnode = mNodeHeap->construct<dirwalker::node>();

            mFilePath.dirpath().to_string(mPathStr);
            mPathStr += '*';

            nextnode->mFindHandle = ::FindFirstFileW(LPCWSTR(mPathStr.m_utf16.m_str), &nextnode->mFindData);

            if (nextnode->mFindHandle != INVALID_HANDLE_VALUE)
            {
                // Link new node into linked-list
                nextnode->mPrev = mDirStack;
                mDirStack       = nextnode;
                return true;
            }
            else
            {
                mNodeHeap->destruct(nextnode);
                return false;
            }
        }
    };

    struct enumerate_delegate_copy : public enumerate_delegate_t
    {
        dirpath_t const& mSrcDir;
        dirpath_t const& mDstDir;
        filedevice_t*    mDstDevice;
        bool             mOverwrite;

        enumerate_delegate_copy(dirpath_t const& srcdir, dirpath_t const& dstdir, filedevice_t* dstdevice, bool overwrite) : mSrcDir(srcdir), mDstDir(dstdir), mDstDevice(dstdevice), mOverwrite(overwrite) {}

        virtual bool operator()(s32 depth, const filepath_t& fp, const fileattrs_t& fa, const filetimes_t& ft)
        {
            if (mDstDevice != nullptr)
            {
                filepath_t dstfilepath = fp;
                dstfilepath.makeRelativeTo(mSrcDir);
                dstfilepath.makeAbsoluteTo(mDstDir);
                mDstDevice->copyFile(fp, dstfilepath, true);
                return true;
            }
            return false;
        }
        virtual bool operator()(s32 depth, const dirpath_t& dp)
        {
            if (mDstDevice != nullptr)
            {
                dirpath_t subpath;
                dirpath_t::getSubDir(mSrcDir, dp, subpath);
                dirpath_t dstdirpath = mDstDir + subpath;
                mDstDevice->createDir(dstdirpath);
                return true;
            }
            return false;
        }
    };

    bool filedevice_pc_t::copyDir(const dirpath_t& szDirPath, const dirpath_t& szToDirPath, bool boOverwrite)
    {
        enumerate_delegate_copy copy_enum(szDirPath, szToDirPath, this, boOverwrite);
        enumerate(szDirPath, copy_enum);
        return true;
    }

    struct enumerate_delegate_delete_dir : public enumerate_delegate_t
    {
        filesys_t* m_root;
        alloc_t* m_alloc;

        virtual bool operator()(s32 depth, const filepath_t& fp, const fileattrs_t& fa, const filetimes_t& ft)
        {
            s32 const len = fp.to_strlen() + 1;
            utf16::prune str = (utf16::prune)m_alloc->allocate(sizeof(utf16::rune) * len);
            runes_t runes16(str, str + len);

            DWORD dwFileAttributes = ::GetFileAttributesW((LPCWSTR)runes16.m_utf16.m_str);
            if (dwFileAttributes & FILE_ATTRIBUTE_READONLY) // change read-only file mode
                ::SetFileAttributesW((LPCWSTR)runes16.m_utf16.m_str, dwFileAttributes & ~FILE_ATTRIBUTE_READONLY);
            ::DeleteFileW((LPCWSTR)runes16.m_utf16.m_str);

            m_alloc->deallocate(str);
            return true;
        }

        virtual bool operator()(s32 depth, const dirpath_t& dp)
        {
            return true;
        }
    };

    bool filedevice_pc_t::deleteDir(const dirpath_t& szDirPath)
    {
        enumerate_delegate_delete_dir enumerator;
        enumerator.m_root = szDirPath.m_device->m_root;
        enumerator.m_alloc = enumerator.m_root->m_allocator;
        return enumerate(szDirPath, enumerator);
    }

    bool filedevice_pc_t::setDirTime(const dirpath_t& szDirPath, const filetimes_t& ftimes)
    {
        HANDLE handle;
        openDir(szDirPath, handle);
        if (handle != INVALID_HANDLE_VALUE)
        {
            datetime_t creationTime;
            ftimes.getCreationTime(creationTime);
            datetime_t lastAccessTime;
            ftimes.getLastAccessTime(lastAccessTime);
            datetime_t lastWriteTime;
            ftimes.getLastWriteTime(lastWriteTime);

            FILETIME _creationTime;
            u64      uCreationTime       = creationTime.toFileTime();
            _creationTime.dwHighDateTime = nmem::hiu32(uCreationTime);
            _creationTime.dwLowDateTime  = nmem::lou32(uCreationTime);

            FILETIME _lastAccessTime;
            u64      uLastAccessTime       = lastAccessTime.toFileTime();
            _lastAccessTime.dwHighDateTime = nmem::hiu32(uLastAccessTime);
            _lastAccessTime.dwLowDateTime  = nmem::lou32(uLastAccessTime);

            FILETIME _lastWriteTime;
            u64      uLastWriteTime       = lastWriteTime.toFileTime();
            _lastWriteTime.dwHighDateTime = nmem::hiu32(uLastWriteTime);
            _lastWriteTime.dwLowDateTime  = nmem::lou32(uLastWriteTime);

            ::SetFileTime(handle, &_creationTime, &_lastAccessTime, &_lastWriteTime);
            sCloseDir(handle);
            return true;
        }
        return false;
    }

    bool filedevice_pc_t::getDirTime(const dirpath_t& szDirPath, filetimes_t& ftimes)
    {
        HANDLE handle;
        openDir(szDirPath, handle);
        if (handle != INVALID_HANDLE_VALUE)
        {
            FILETIME _creationTime;
            FILETIME _lastAccessTime;
            FILETIME _lastWriteTime;
            ::GetFileTime(handle, &_creationTime, &_lastAccessTime, &_lastWriteTime);

            datetime_t outCreationTime;
            datetime_t outLastAccessTime;
            datetime_t outLastWriteTime;
            outCreationTime   = datetime_t::sFromFileTime((u64)nmem::makeu64(_creationTime.dwLowDateTime, _creationTime.dwHighDateTime));
            outLastAccessTime = datetime_t::sFromFileTime((u64)nmem::makeu64(_lastAccessTime.dwLowDateTime, _lastAccessTime.dwHighDateTime));
            outLastWriteTime  = datetime_t::sFromFileTime((u64)nmem::makeu64(_lastWriteTime.dwLowDateTime, _lastWriteTime.dwHighDateTime));
            ftimes.setCreationTime(outCreationTime);
            ftimes.setLastAccessTime(outLastAccessTime);
            ftimes.setLastWriteTime(outLastWriteTime);
            sCloseDir(handle);
            return true;
        }
        return false;
    }

    bool filedevice_pc_t::setDirAttr(const dirpath_t& szDirPath, const fileattrs_t& attr)
    {
        DWORD dwFileAttributes = 0;
        if (attr.isArchive())
            dwFileAttributes = dwFileAttributes | FILE_ATTRIBUTE_ARCHIVE;
        if (attr.isReadOnly())
            dwFileAttributes = dwFileAttributes | FILE_ATTRIBUTE_READONLY;
        if (attr.isHidden())
            dwFileAttributes = dwFileAttributes | FILE_ATTRIBUTE_HIDDEN;
        if (attr.isSystem())
            dwFileAttributes = dwFileAttributes | FILE_ATTRIBUTE_SYSTEM;

        alloc_t* allocator = szDirPath.m_device->m_root->m_allocator;

        s32 const pathstrlen = szDirPath.to_strlen();
        utf16::prune pathstr = (utf16::prune)allocator->allocate(pathstrlen * sizeof(utf16::rune));
        runes_t dirpath16(pathstr, pathstr + pathstrlen);
        szDirPath.to_string(dirpath16);

        bool result = ::SetFileAttributesW((LPCWSTR)dirpath16.m_utf16.m_str, dwFileAttributes) == TRUE;

        return result;
    }

    bool filedevice_pc_t::getDirAttr(const dirpath_t& szDirPath, fileattrs_t& attr)
    {
        alloc_t* allocator = szDirPath.m_device->m_root->m_allocator;

        s32 const pathstrlen = szDirPath.to_strlen();
        utf16::prune pathstr = (utf16::prune)allocator->allocate(pathstrlen * sizeof(utf16::rune));
        runes_t dirpath16(pathstr, pathstr + pathstrlen);
        szDirPath.to_string(dirpath16);

        bool  result           = false;
        DWORD dwFileAttributes = ::GetFileAttributesW((LPCWSTR)dirpath16.m_utf16.m_str);
        if (dwFileAttributes != INVALID_FILE_ATTRIBUTES)
        {
            result = true;
            attr.setArchive(dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE);
            attr.setReadOnly(dwFileAttributes & FILE_ATTRIBUTE_READONLY);
            attr.setHidden(dwFileAttributes & FILE_ATTRIBUTE_HIDDEN);
            attr.setSystem(dwFileAttributes & FILE_ATTRIBUTE_SYSTEM);
        }

        return true;
    }

    bool filedevice_pc_t::enumerate(const dirpath_t& szDirPath, enumerate_delegate_t& enumerator)
    {
        alloc_t* allocator = szDirPath.m_device->m_root->m_allocator;
        dirwalker walker(allocator, szDirPath);

        if (!walker.enter_dir())
            return false;

        walker.enumerate_dir(enumerator);

        bool bSearch = true;
        while (bSearch)
        {
            if (walker.next())
            {
                if (walker.is_dots())
                {
                    // NOP
                }
                else if (walker.is_dir())
                {
                    if (walker.push_dir())
                    {
                        bSearch = walker.enumerate_dir(enumerator);
                    }
                }
                else
                {
                    bSearch = walker.enumerate_file(enumerator);
                }
            }
            else
            {
                bSearch = walker.pop_dir();
            }
        }
        return true;
    }

    bool filedevice_pc_t::seek(void* nFileHandle, ESeekMode mode, u64 pos, u64& newPos)
    {
        s32 hardwareMode = 0;
        switch (mode)
        {
            case __SEEK_ORIGIN: hardwareMode = FILE_BEGIN; break;
            case __SEEK_CURRENT: hardwareMode = FILE_CURRENT; break;
            case __SEEK_END: hardwareMode = FILE_END; break;
            default: ASSERT(0); break;
        }

        // seek!
        LARGE_INTEGER position;
        LARGE_INTEGER newFilePointer;

        newPos            = pos;
        position.LowPart  = (u32)pos;
        position.HighPart = 0;
        DWORD result      = ::SetFilePointerEx((HANDLE)nFileHandle, position, &newFilePointer, hardwareMode);
        if (!result)
        {
            if (result == INVALID_SET_FILE_POINTER)
            {
                // Failed to seek.
            }
            return false;
        }
        newPos = newFilePointer.LowPart;
        return true;
    }
    bool filedevice_pc_t::seekOrigin(void* nFileHandle, u64 pos, u64& newPos) { return seek(nFileHandle, __SEEK_ORIGIN, pos, newPos); }
    bool filedevice_pc_t::seekCurrent(void* nFileHandle, u64 pos, u64& newPos) { return seek(nFileHandle, __SEEK_CURRENT, pos, newPos); }
    bool filedevice_pc_t::seekEnd(void* nFileHandle, u64 pos, u64& newPos) { return seek(nFileHandle, __SEEK_END, pos, newPos); }

}; // namespace ncore

#endif // TARGET_PC
