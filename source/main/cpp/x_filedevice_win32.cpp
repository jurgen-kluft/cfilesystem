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
#include "xfilesystem/x_fileinfo.h"
#include "xfilesystem/x_dirinfo.h"
#include "xfilesystem/x_filepath.h"
#include "xfilesystem/x_dirpath.h"
#include "xfilesystem/x_filesystem.h"

namespace xcore
{
    class filedevice_pc_t : public filedevice_t
    {
    public:
        alloc_t*       mAllocator;
        runes_t        mPathStr;
        runes_t        mPathStr2;
        dirpath_t      mDrivePath;
        filesysroot_t* mSysRoot;
        bool           mCanWrite;

        XCORE_CLASS_PLACEMENT_NEW_DELETE

        filedevice_pc_t(alloc_t* alloc, runes_t& path16Str, runes_t& path16Str2, const dirpath_t& pDrivePath, bool boCanWrite) : mAllocator(alloc), mPathStr(path16Str), mPathStr2(path16Str2), mDrivePath(pDrivePath), mCanWrite(boCanWrite) {}
        virtual ~filedevice_pc_t() {}

        virtual void destruct(alloc_t* allocator)
        {
            allocator->deallocate(mPathStr.m_runes.m_utf16.m_bos);
            allocator->deallocate(mPathStr2.m_runes.m_utf16.m_bos);
            allocator->destruct(this);
        }

        virtual bool canSeek() const { return true; }
        virtual bool canWrite() const { return mCanWrite; }

        virtual bool getDeviceInfo(u64& totalSpace, u64& freeSpace) const;

        virtual bool openFile(const filepath_t& szFilename, EFileMode mode, EFileAccess access, EFileOp op, void*& nFileHandle);
        virtual bool createFile(const filepath_t& szFilename, bool boRead, bool boWrite, void*& nFileHandle);
        virtual bool readFile(void* nFileHandle, u64 pos, void* buffer, u64 count, u64& outNumBytesRead);
        virtual bool writeFile(void* nFileHandle, u64 pos, const void* buffer, u64 count, u64& outNumBytesWritten);
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

        static HANDLE openDir(dirpath_t const& szDirPath);
    };

    filedevice_t* x_CreateFileDevicePC(alloc_t* alloc, const dirpath_t& pDrivePath, bool boCanWrite)
    {
        utf16::prune     pPath16Str = (utf16::prune)alloc->allocate(8200 * sizeof(utf16::rune));
        runes_t          pathStr(pPath16Str, pPath16Str + 8192);
        utf16::prune     pPath16Str2 = (utf16::prune)alloc->allocate(8200 * sizeof(utf16::rune));
        runes_t          pathStr2(pPath16Str2, pPath16Str2 + 8192);
        filedevice_pc_t* file_device = alloc->construct<filedevice_pc_t>(alloc, pathStr, pathStr2, pDrivePath, boCanWrite);
        return file_device;
    }

    void x_DestroyFileDevicePC(alloc_t* alloc, filedevice_t* device) { device->destruct(alloc); }

    filedevice_t* x_CreateFileDevice(alloc_t* allocator, crunes_t const& pDrivePath, bool boCanWrite)
    {
        dirpath_t drivePath = filesystem_t::dirpath(pDrivePath);
        return x_CreateFileDevicePC(allocator, drivePath, boCanWrite);
    }

    void x_DestroyFileDevice(alloc_t* allocator, filedevice_t* fd) { x_DestroyFileDevicePC(allocator, fd); }

    bool filedevice_pc_t::getDeviceInfo(u64& totalSpace, u64& freeSpace) const
    {
        ULARGE_INTEGER totalbytes, freebytes;

        runes_t drivepath(mPathStr);
        mDrivePath.to_string(drivepath);

        bool result = false;
        if (GetDiskFreeSpaceExW((LPCWSTR)drivepath.str16(), NULL, &totalbytes, &freebytes) != 0)
        {
            freeSpace  = freebytes.QuadPart;
            totalSpace = totalbytes.QuadPart;
            result     = true;
        }

        return result;
    }

    bool filedevice_pc_t::hasFile(const filepath_t& szFilename)
    {
        u32 shareType   = FILE_SHARE_READ;
        u32 fileMode    = GENERIC_READ;
        u32 disposition = OPEN_EXISTING;
        u32 attrFlags   = FILE_ATTRIBUTE_NORMAL;

        runes_t filename16(mPathStr);
        szFilename.to_string(filename16);

        bool   result      = false;
        HANDLE nFileHandle = ::CreateFileW(LPCWSTR(filename16.str16()), fileMode, shareType, NULL, disposition, attrFlags, NULL);
        if (nFileHandle != INVALID_HANDLE_VALUE)
        {
            ::CloseHandle((HANDLE)nFileHandle);
            result = true;
        }

        // path_t::release_utf16(szFilename, filename16);
        return result;
    }

    bool filedevice_pc_t::openFile(const filepath_t& szFilename, EFileMode mode, EFileAccess access, EFileOp op, void*& nFileHandle)
    {
        u32 shareType   = FILE_SHARE_READ;
        u32 fileMode    = (access == FileAccess_Read) ? GENERIC_READ : GENERIC_WRITE | GENERIC_READ;
        u32 disposition = OPEN_EXISTING;
        u32 attrFlags   = FILE_ATTRIBUTE_NORMAL;

        runes_t filename16(mPathStr);
        szFilename.to_string(filename16);

        HANDLE handle = ::CreateFileW(LPCWSTR(filename16.str16()), fileMode, shareType, NULL, disposition, attrFlags, NULL);
        nFileHandle   = handle;

        // path_t::release_utf16(szFilename, filename16);

        return nFileHandle != INVALID_HANDLE_VALUE;
    }

    bool filedevice_pc_t::createFile(const filepath_t& szFilename, bool boRead, bool boWrite, void*& nFileHandle)
    {
        u32 shareType   = FILE_SHARE_READ;
        u32 fileMode    = !boWrite ? GENERIC_READ : GENERIC_WRITE | GENERIC_READ;
        u32 disposition = CREATE_ALWAYS;
        u32 attrFlags   = FILE_ATTRIBUTE_NORMAL;

        runes_t filename16(mPathStr);
        szFilename.to_string(filename16);

        HANDLE handle = ::CreateFileW((LPCWSTR)filename16.str16(), fileMode, shareType, NULL, disposition, attrFlags, NULL);
        nFileHandle   = handle;

        // path_t::release_utf16(szFilename, filename16);

        return nFileHandle != INVALID_HANDLE_VALUE;
    }

    bool filedevice_pc_t::readFile(void* nFileHandle, u64 pos, void* buffer, u64 count, u64& outNumBytesRead)
    {
        u64 newPos;
        if (seek(nFileHandle, __SEEK_ORIGIN, pos, newPos))
        {
            DWORD numBytesRead;
            bool  boSuccess = ::ReadFile((HANDLE)nFileHandle, buffer, (DWORD)count, &numBytesRead, NULL);

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
            bool  boSuccess = ::WriteFile((HANDLE)nFileHandle, buffer, (DWORD)count, &numBytesWritten, NULL);

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

        runes_t filename16(mPathStr);
        szFilename.to_string(filename16);
        runes_t tofilename16(mPathStr);
        szFilename.to_string(tofilename16);

        BOOL result = ::MoveFileW((LPCWSTR)filename16.str16(), (LPCWSTR)tofilename16.m_runes.m_utf16.m_str) != 0;

        return result;
    }

    bool filedevice_pc_t::copyFile(const filepath_t& szFilename, const filepath_t& szToFilename, bool boOverwrite)
    {
        if (!canWrite())
            return false;

        const bool failIfExists = boOverwrite == false;

        runes_t filename16(mPathStr);
        szFilename.to_string(filename16);
        runes_t tofilename16(mPathStr2);
        szToFilename.to_string(tofilename16);

        BOOL result = ::CopyFileW((LPCWSTR)filename16.str16(), (LPCWSTR)tofilename16.m_runes.m_utf16.m_str, failIfExists) != 0;

        // path_t::release_utf16(szFilename, filename16);
        // path_t::release_utf16(szToFilename, tofilename16);

        return result == TRUE;
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
        runes_t filename16(mPathStr);
        szFilename.to_string(filename16);

        bool result = false;
        if (::DeleteFileW(LPCWSTR(filename16.str16())))
        {
            result = true;
        }

        // path_t::release_utf16(szFilename, filename16);
        return result;
    }

    bool filedevice_pc_t::setLengthOfFile(void* nFileHandle, u64 inLength)
    {
        xsize_t distanceLow  = (xsize_t)inLength;
        xsize_t distanceHigh = (xsize_t)(inLength >> 32);
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
            _creationTime.dwHighDateTime = xmem::hiu32(uCreationTime);
            _creationTime.dwLowDateTime  = xmem::lou32(uCreationTime);

            FILETIME _lastAccessTime;
            u64      uLastAccessTime       = lastAccessTime.toFileTime();
            _lastAccessTime.dwHighDateTime = xmem::hiu32(uLastAccessTime);
            _lastAccessTime.dwLowDateTime  = xmem::lou32(uLastAccessTime);

            FILETIME _lastWriteTime;
            u64      uLastWriteTime       = lastWriteTime.toFileTime();
            _lastWriteTime.dwHighDateTime = xmem::hiu32(uLastWriteTime);
            _lastWriteTime.dwLowDateTime  = xmem::lou32(uLastWriteTime);

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
            datetime_t CreationTime   = datetime_t::sFromFileTime((u64)xmem::makeu64(_creationTime.dwLowDateTime, _creationTime.dwHighDateTime));
            datetime_t LastAccessTime = datetime_t::sFromFileTime((u64)xmem::makeu64(_lastAccessTime.dwLowDateTime, _lastAccessTime.dwHighDateTime));
            datetime_t LastWriteTime  = datetime_t::sFromFileTime((u64)xmem::makeu64(_lastWriteTime.dwLowDateTime, _lastWriteTime.dwHighDateTime));
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

        runes_t filename16(mPathStr);
        szFilename.to_string(filename16);

        bool result = ::SetFileAttributesW(LPCWSTR(filename16.str16()), dwFileAttributes) == TRUE;

        // path_t::release_utf16(szFilename, filename16);

        return result;
    }

    bool filedevice_pc_t::getFileAttr(const filepath_t& szFilename, fileattrs_t& attr)
    {
        DWORD dwFileAttributes = 0;

        runes_t filename16(mPathStr);
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

        // path_t::release_utf16(szFilename, filename16);

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
        _creationTime.dwHighDateTime = xmem::hiu32(uCreationTime);
        _creationTime.dwLowDateTime  = xmem::lou32(uCreationTime);

        FILETIME _lastAccessTime;
        u64      uLastAccessTime       = lastAccessTime.toFileTime();
        _lastAccessTime.dwHighDateTime = xmem::hiu32(uLastAccessTime);
        _lastAccessTime.dwLowDateTime  = xmem::lou32(uLastAccessTime);

        FILETIME _lastWriteTime;
        u64      uLastWriteTime       = lastWriteTime.toFileTime();
        _lastWriteTime.dwHighDateTime = xmem::hiu32(uLastWriteTime);
        _lastWriteTime.dwLowDateTime  = xmem::lou32(uLastWriteTime);

        ::SetFileTime((HANDLE)nFileHandle, &_creationTime, &_lastAccessTime, &_lastWriteTime);
        return true;
    }

    bool filedevice_pc_t::getFileTime(void* nFileHandle, filetimes_t& ftimes)
    {
        FILETIME _creationTime;
        FILETIME _lastAccessTime;
        FILETIME _lastWriteTime;
        ::GetFileTime((HANDLE)nFileHandle, &_creationTime, &_lastAccessTime, &_lastWriteTime);
        datetime_t CreationTime   = datetime_t::sFromFileTime((u64)xmem::makeu64(_creationTime.dwLowDateTime, _creationTime.dwHighDateTime));
        datetime_t LastAccessTime = datetime_t::sFromFileTime((u64)xmem::makeu64(_lastAccessTime.dwLowDateTime, _lastAccessTime.dwHighDateTime));
        datetime_t LastWriteTime  = datetime_t::sFromFileTime((u64)xmem::makeu64(_lastWriteTime.dwLowDateTime, _lastWriteTime.dwHighDateTime));
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

        runes_t path16(mPathStr);
        szDirPath.to_string(path16);

        nDirHandle = ::CreateFileW((LPCWSTR)path16.str16(), fileMode, shareType, NULL, disposition, attrFlags, NULL);

        return true;
    }

    static void sCloseDir(HANDLE handle) { ::CloseHandle(handle); }

    bool filedevice_pc_t::hasDir(const dirpath_t& szDirPath)
    {
        HANDLE handle = openDir(szDirPath);
        if (handle == INVALID_HANDLE_VALUE)
            return false;
        sCloseDir(handle);
        return true;
    }

    bool filedevice_pc_t::createDir(const dirpath_t& szDirPath)
    {
        runes_t path16(mPathStr);
        szDirPath.to_string(path16);

        BOOL result = ::CreateDirectoryW(LPCWSTR(path16.str16()), NULL) != 0;
        return result;
    }

    bool filedevice_pc_t::moveDir(const dirpath_t& szDirPath, const dirpath_t& szToDirPath, bool boOverwrite)
    {
        u32     dwFlags = MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED;
        runes_t path16(mPathStr);
        szDirPath.to_string(path16);
        runes_t topath16(mPathStr);
        szToDirPath.to_string(topath16);
        BOOL result = ::MoveFileExW((LPCWSTR)path16.str16(), (LPCWSTR)topath16.m_runes.m_utf16.m_str, dwFlags) != 0;
        return result;
    }

    static void changeDirPath(const dirpath_t& src, const dirpath_t& dst, dirinfo_t const* src_dirinfo, dirpath_t& result)
    {
        dirpath_t srcdirpath(src);
        dirpath_t curdirpath = src_dirinfo->getDirpath();
        curdirpath.makeRelativeTo(srcdirpath);
        result = dst + curdirpath;
    }

    static void changeFilePath(const dirpath_t& src, const dirpath_t& dst, fileinfo_t const* src_fileinfo, filepath_t& result)
    {
        dirpath_t  srcdirpath(src);
        filepath_t curfilepath;
        src_fileinfo->getFilepath(curfilepath);
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

        filesysroot_t* mSysRoot;
        runes_t        mPathStr;

        dirinfo_t  mDirInfo;
        fileinfo_t mFileInfo;
        filepath_t mFilePath;

        dirwalker(dirpath_t const& dirpath) : mNodeHeap(nullptr)
        {
            mNodeHeap = mSysRoot->m_allocator;
            mFilePath.setDirpath(dirpath);
        }

        bool next() { return ::FindNextFileW(mDirStack->mFindHandle, &mDirStack->mFindData); }
        inline bool is_dots() const { return (sIsDots(mDirStack->mFindData.cFileName)); }
        inline bool is_dir() const { return ((mDirStack->mFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)); }

        bool push_dir()
        {
            runes_t dirname;
            dirname.m_runes.m_utf16.m_str = (utf16::prune)mDirStack->mFindData.cFileName;
            dirname.m_runes.m_utf16.m_end = dirname.m_runes.m_utf16.m_str;
            while (*dirname.m_runes.m_utf16.m_end != '\0')
            {
                dirname.m_runes.m_utf16.m_end++;
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
            return (enumerator(mLevel, nullptr, &mDirInfo)); 
        }

        fileattrs_t build_fileattrs(WIN32_FIND_DATAW* nFileData)
        {
            fileattrs_t attrs;
            attrs.setHidden(nFileData->dwFileAttributes & FILE_ATTRIBUTE_HIDDEN);
            return attrs;
        }

        filetimes_t build_filetimes(WIN32_FIND_DATAW* nFileData)
        {
            u64 const  creationtime   = xmem::makeu64(nFileData->ftCreationTime.dwLowDateTime, nFileData->ftCreationTime.dwHighDateTime);
            datetime_t creationdt     = datetime_t::sFromFileTime(creationtime);
            u64 const  lastwritetime  = xmem::makeu64(nFileData->ftLastWriteTime.dwLowDateTime, nFileData->ftLastWriteTime.dwHighDateTime);
            datetime_t lastwritedt    = datetime_t::sFromFileTime(lastwritetime);
            u64 const  lastaccesstime = xmem::makeu64(nFileData->ftLastAccessTime.dwLowDateTime, nFileData->ftLastAccessTime.dwHighDateTime);
            datetime_t lastaccessdt   = datetime_t::sFromFileTime(lastaccesstime);

            return filetimes_t(creationdt, lastaccessdt, lastwritedt);
        }

        bool enumerate_file(enumerate_delegate_t& enumerator)
        {
            // Prepare FileInfo
            runes_t filename;
            filename.m_runes.m_utf16.m_str = (utf16::prune)mDirStack->mFindData.cFileName;
            filename.m_runes.m_utf16.m_end = filename.m_runes.m_utf16.m_str;
            filename.m_runes.m_utf16.m_eos = (utf16::prune)mDirStack->mFindData.cFileName[sizeof(mDirStack->mFindData.cFileName) - 1];
            while (filename.m_runes.m_utf16.m_end < filename.m_runes.m_utf16.m_eos && *filename.m_runes.m_utf16.m_end != '\0')
            {
                filename.m_runes.m_utf16.m_end++;
            }
            pathname_t* fname;
            pathname_t* fext;
            mSysRoot->register_filename(filename, fname, fext);

            fileattrs_t fileattrs = build_fileattrs(&mDirStack->mFindData);
            filetimes_t filetimes = build_filetimes(&mDirStack->mFindData);

            mFilePath.setFilename(fname);
            mFilePath.setExtension(fext);
            mFileInfo = fileinfo_t(mFilePath, fileattrs, filetimes);

            return (enumerator(mLevel, &mFileInfo, nullptr));
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

            nextnode->mFindHandle = ::FindFirstFileW(LPCWSTR(mPathStr.m_runes.m_utf16.m_str), &nextnode->mFindData);

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

        virtual bool operator()(s32 depth, const fileinfo_t* finf, const dirinfo_t* dinf)
        {
            if (mDstDevice != nullptr)
            {
                if (dinf != nullptr)
                {
                    dirpath_t subpath;
                    dirpath_t::getSubDir(mSrcDir, dinf->getDirpath(), subpath);
                    dirpath_t dstdirpath = mDstDir + subpath;
                    mDstDevice->createDir(dstdirpath);
                    return true;
                }
                else if (finf != nullptr)
                {
                    filepath_t dstfilepath = finf->getFilepath();
                    dstfilepath.makeRelativeTo(mSrcDir);
                    dstfilepath.makeAbsoluteTo(mDstDir);
                    mDstDevice->copyFile(finf->getFilepath(), dstfilepath, true);
                    return true;
                }
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
        virtual bool operator()(s32 depth, const fileinfo_t* finf, const dirinfo_t* dinf)
        {
            if (finf != nullptr)
            {
                const filepath_t& fp = finf->getFilepath();

                runes_t runes16;
                //@TODO: memory for the generated utf16 string?

                DWORD dwFileAttributes = ::GetFileAttributesW((LPCWSTR)runes16.m_runes.m_utf16.m_str);
                if (dwFileAttributes & FILE_ATTRIBUTE_READONLY) // change read-only file mode
                    ::SetFileAttributesW((LPCWSTR)runes16.m_runes.m_utf16.m_str, dwFileAttributes & ~FILE_ATTRIBUTE_READONLY);
                ::DeleteFileW((LPCWSTR)runes16.m_runes.m_utf16.m_str);
            }
            return true;
        }
    };

    bool filedevice_pc_t::deleteDir(const dirpath_t& szDirPath)
    {
        enumerate_delegate_delete_dir enumerator;
        return enumerate(szDirPath, enumerator);
    }

    bool filedevice_pc_t::setDirTime(const dirpath_t& szDirPath, const filetimes_t& ftimes)
    {
        HANDLE handle = openDir(szDirPath);
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
            _creationTime.dwHighDateTime = xmem::hiu32(uCreationTime);
            _creationTime.dwLowDateTime  = xmem::lou32(uCreationTime);

            FILETIME _lastAccessTime;
            u64      uLastAccessTime       = lastAccessTime.toFileTime();
            _lastAccessTime.dwHighDateTime = xmem::hiu32(uLastAccessTime);
            _lastAccessTime.dwLowDateTime  = xmem::lou32(uLastAccessTime);

            FILETIME _lastWriteTime;
            u64      uLastWriteTime       = lastWriteTime.toFileTime();
            _lastWriteTime.dwHighDateTime = xmem::hiu32(uLastWriteTime);
            _lastWriteTime.dwLowDateTime  = xmem::lou32(uLastWriteTime);

            ::SetFileTime(handle, &_creationTime, &_lastAccessTime, &_lastWriteTime);
            sCloseDir(handle);
            return true;
        }
        return false;
    }

    bool filedevice_pc_t::getDirTime(const dirpath_t& szDirPath, filetimes_t& ftimes)
    {
        HANDLE handle = openDir(szDirPath);
        if (handle != INVALID_HANDLE_VALUE)
        {
            FILETIME _creationTime;
            FILETIME _lastAccessTime;
            FILETIME _lastWriteTime;
            ::GetFileTime(handle, &_creationTime, &_lastAccessTime, &_lastWriteTime);

            datetime_t outCreationTime;
            datetime_t outLastAccessTime;
            datetime_t outLastWriteTime;
            outCreationTime   = datetime_t::sFromFileTime((u64)xmem::makeu64(_creationTime.dwLowDateTime, _creationTime.dwHighDateTime));
            outLastAccessTime = datetime_t::sFromFileTime((u64)xmem::makeu64(_lastAccessTime.dwLowDateTime, _lastAccessTime.dwHighDateTime));
            outLastWriteTime  = datetime_t::sFromFileTime((u64)xmem::makeu64(_lastWriteTime.dwLowDateTime, _lastWriteTime.dwHighDateTime));
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

        runes_t dirpath16(mPathStr);
        szDirPath.to_string(dirpath16);
        bool result = ::SetFileAttributesW((LPCWSTR)dirpath16.m_runes.m_utf16.m_str, dwFileAttributes) == TRUE;

        return result;
    }

    bool filedevice_pc_t::getDirAttr(const dirpath_t& szDirPath, fileattrs_t& attr)
    {
        runes_t dirpath16(mPathStr);
        szDirPath.to_string(dirpath16);

        bool  result           = false;
        DWORD dwFileAttributes = ::GetFileAttributesW((LPCWSTR)dirpath16.m_runes.m_utf16.m_str);
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
        dirwalker walker(szDirPath);

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

}; // namespace xcore

#endif // TARGET_PC
