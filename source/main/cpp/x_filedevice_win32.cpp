#include "xbase/x_target.h"
#if defined TARGET_PC || defined TARGET_MAC

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
#include "xfilesystem/x_attributes.h"
#include "xfilesystem/x_fileinfo.h"
#include "xfilesystem/x_dirinfo.h"

namespace xcore
{
    class FileDevice_PC_System : public xfiledevice
    {
        xalloc*  mAllocator;
        xdirpath mDrivePath;
        xbool    mCanWrite;

    public:
        FileDevice_PC_System(const xdirpath& pDrivePath, xbool boCanWrite) : mDrivePath(pDrivePath), mCanWrite(boCanWrite) {}
        virtual ~FileDevice_PC_System() {}

        virtual bool canSeek() const { return true; }
        virtual bool canWrite() const { return mCanWrite; }

        virtual bool getDeviceInfo(u64& totalSpace, u64& freeSpace) const;

        virtual bool openFile(const xfilepath& szFilename, bool boRead, bool boWrite, void*& nFileHandle) const;
        virtual bool createFile(const xfilepath& szFilename, bool boRead, bool boWrite, void*& nFileHandle) const;

        virtual bool readFile(void* nFileHandle, u64 pos, void* buffer, u64 count, u64& outNumBytesRead) const;
        virtual bool writeFile(void* nFileHandle, u64 pos, const void* buffer, u64 count, u64& outNumBytesWritten) const;
        virtual bool closeFile(void* nFileHandle) const;

        virtual bool setLengthOfFile(void* nFileHandle, u64 inLength) const;
        virtual bool getLengthOfFile(void* nFileHandle, u64& outLength) const;

        virtual bool setFileTime(const xfilepath& szFilename, const xfiletimes& ftimes) const;
        virtual bool getFileTime(const xfilepath& szFilename, xfiletimes& ftimes) const;
        virtual bool setFileAttr(const xfilepath& szFilename, const xfileattrs& attr) const;
        virtual bool getFileAttr(const xfilepath& szFilename, xfileattrs& attr) const;

        virtual bool hasFile(const xfilepath& szFilename) const;
        virtual bool moveFile(const xfilepath& szFilename, const xfilepath& szToFilename) const;
        virtual bool copyFile(const xfilepath& szFilename, const xfilepath& szToFilename, bool boOverwrite) const;
        virtual bool deleteFile(const xfilepath& szFilename) const;

        virtual bool hasDir(const xdirpath& szDirPath) const;
        virtual bool createDir(const xdirpath& szDirPath) const;
        virtual bool moveDir(const xdirpath& szDirPath, const xdirpath& szToDirPath) const;
        virtual bool copyDir(const xdirpath& szDirPath, const xdirpath& szToDirPath, bool boOverwrite) const;
        virtual bool deleteDir(const xdirpath& szDirPath) const;

        virtual bool setDirTime(const xdirpath& szDirPath, const xfiletimes& ftimes) const;
        virtual bool getDirTime(const xdirpath& szDirPath, xfiletimes& ftimes) const;
        virtual bool setDirAttr(const xdirpath& szDirPath, const xfileattrs& attr) const;
        virtual bool getDirAttr(const xdirpath& szDirPath, xfileattrs& attr) const;

        virtual bool enumerate(const xdirpath& szDirPath, enumerate_delegate& enumerator, s32 depth = 0) const;

        enum ESeekMode
        {
            __SEEK_ORIGIN  = 1,
            __SEEK_CURRENT = 2,
            __SEEK_END     = 3,
        };
        bool seek(void* nFileHandle, ESeekMode mode, u64 pos, u64& newPos) const;
        bool seekOrigin(void* nFileHandle, u64 pos, u64& newPos) const;
        bool seekCurrent(void* nFileHandle, u64 pos, u64& newPos) const;
        bool seekEnd(void* nFileHandle, u64 pos, u64& newPos) const;
    };

    xfiledevice* x_CreateFileDevicePC(const xdirpath& pDrivePath, xbool boCanWrite)
    {
        FileDevice_PC_System* file_device = xnew<FileDevice_PC_System>(pDrivePath, boCanWrite);
        return file_device;
    }

    void x_DestroyFileDevicePC(xfiledevice* device)
    {
        FileDevice_PC_System* file_device = (FileDevice_PC_System*)device;
        delete file_device;
    }

    bool FileDevice_PC_System::getDeviceInfo(u64& totalSpace, u64& freeSpace) const
    {
        ULARGE_INTEGER totalbytes, freebytes;
        if (GetDiskFreeSpaceExW((LPCWSTR)mDrivePath.mRunes.m_str, NULL, &totalbytes, &freebytes) == 0)
            return false;

        freeSpace  = freebytes.QuadPart;
        totalSpace = totalbytes.QuadPart;
        return true;
    }

    bool FileDevice_PC_System::hasFile(const xfilepath& szFilename) const
    {
        u32 shareType   = FILE_SHARE_READ;
        u32 fileMode    = GENERIC_READ;
        u32 disposition = OPEN_EXISTING;
        u32 attrFlags   = FILE_ATTRIBUTE_NORMAL;

        HANDLE nFileHandle =
            ::CreateFileW(LPCWSTR(szFilename.mRunes.m_str), fileMode, shareType, NULL, disposition, attrFlags, NULL);
        if (nFileHandle != INVALID_HANDLE_VALUE)
        {
            ::CloseHandle((HANDLE)nFileHandle);
            return true;
        }
        return false;
    }

    bool FileDevice_PC_System::openFile(const xfilepath& szFilename, bool boRead, bool boWrite, void*& nFileHandle) const
    {
        u32 shareType   = FILE_SHARE_READ;
        u32 fileMode    = !boWrite ? GENERIC_READ : GENERIC_WRITE | GENERIC_READ;
        u32 disposition = OPEN_EXISTING;
        u32 attrFlags   = FILE_ATTRIBUTE_NORMAL;

        HANDLE handle =
            ::CreateFileW(LPCWSTR(szFilename.mRunes.m_str), fileMode, shareType, NULL, disposition, attrFlags, NULL);
        nFileHandle = handle;
        return nFileHandle != INVALID_HANDLE_VALUE;
    }

    bool FileDevice_PC_System::createFile(const xfilepath& szFilename, bool boRead, bool boWrite, void*& nFileHandle) const
    {
        u32 shareType   = FILE_SHARE_READ;
        u32 fileMode    = !boWrite ? GENERIC_READ : GENERIC_WRITE | GENERIC_READ;
        u32 disposition = CREATE_ALWAYS;
        u32 attrFlags   = FILE_ATTRIBUTE_NORMAL;

        HANDLE handle =
            ::CreateFileW((LPCWSTR)szFilename.mRunes.m_str, fileMode, shareType, NULL, disposition, attrFlags, NULL);
        nFileHandle = handle;
        return nFileHandle != INVALID_HANDLE_VALUE;
    }

    bool FileDevice_PC_System::readFile(void* nFileHandle, u64 pos, void* buffer, u64 count, u64& outNumBytesRead) const
    {
        u64 newPos;
        if (seek(nFileHandle, __SEEK_ORIGIN, pos, newPos))
        {
            DWORD numBytesRead;
            xbool boSuccess = ::ReadFile((HANDLE)nFileHandle, buffer, (DWORD)count, &numBytesRead, NULL);

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
    bool FileDevice_PC_System::writeFile(void* nFileHandle, u64 pos, const void* buffer, u64 count,
                                         u64& outNumBytesWritten) const
    {
        u64 newPos;
        if (seek(nFileHandle, __SEEK_ORIGIN, pos, newPos))
        {
            DWORD numBytesWritten;
            xbool boSuccess = ::WriteFile((HANDLE)nFileHandle, buffer, (DWORD)count, &numBytesWritten, NULL);

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

    bool FileDevice_PC_System::moveFile(const xfilepath& szFilename, const xfilepath& szToFilename) const
    {
        if (!canWrite())
            return false;

        LPCWSTR wfilename   = (LPCWSTR)szFilename.mRunes.m_str;
        LPCWSTR wtofilename = (LPCWSTR)szToFilename.mRunes.m_str;
        return ::MoveFileW(wfilename, wtofilename) != 0;
    }

    bool FileDevice_PC_System::copyFile(const xfilepath& szFilename, const xfilepath& szToFilename, bool boOverwrite) const
    {
        if (!canWrite())
            return false;

        const bool failIfExists = boOverwrite == false;
        LPCWSTR    wfilename    = (LPCWSTR)szFilename.mRunes.m_str;
        LPCWSTR    wtofilename  = (LPCWSTR)szToFilename.mRunes.m_str;
        return ::CopyFileW(wfilename, wtofilename, failIfExists) != 0;
    }

    bool FileDevice_PC_System::closeFile(void* nFileHandle) const
    {
        if (!::CloseHandle((HANDLE)nFileHandle))
            return false;
        return true;
    }

    bool FileDevice_PC_System::deleteFile(const xfilepath& szFilename) const
    {
        if (!::DeleteFileW(LPCWSTR(szFilename.mRunes.m_str)))
            return false;
        return true;
    }

    bool FileDevice_PC_System::setLengthOfFile(void* nFileHandle, u64 inLength) const
    {
        xsize_t distanceLow  = (xsize_t)inLength;
        xsize_t distanceHigh = (xsize_t)(inLength >> 32);
        ::SetFilePointer((HANDLE)nFileHandle, (LONG)distanceLow, (PLONG)&distanceHigh, FILE_BEGIN);
        ::SetEndOfFile((HANDLE)nFileHandle);
        return true;
    }

    bool FileDevice_PC_System::getLengthOfFile(void* nFileHandle, u64& outLength) const
    {
        DWORD lowSize, highSize;
        lowSize   = ::GetFileSize((HANDLE)nFileHandle, &highSize);
        outLength = highSize;
        outLength = outLength << 16;
        outLength = outLength << 16;
        outLength = outLength | lowSize;
        return true;
    }

    bool FileDevice_PC_System::setFileTime(const xfilepath& szFilename, const xfiletimes& ftimes) const
    {
        void* nFileHandle;
        if (openFile(szFilename, true, true, nFileHandle))
        {
            xdatetime creationTime;
            ftimes.getCreationTime(creationTime);
            xdatetime lastAccessTime;
            ftimes.getLastAccessTime(lastAccessTime);
            xdatetime lastWriteTime;
            ftimes.getLastWriteTime(lastWriteTime);

            FILETIME _creationTime;
            u64      uCreationTime       = creationTime.toFileTime();
            _creationTime.dwHighDateTime = xmem_utils::hiu32(uCreationTime);
            _creationTime.dwLowDateTime  = xmem_utils::lou32(uCreationTime);

            FILETIME _lastAccessTime;
            u64      uLastAccessTime       = lastAccessTime.toFileTime();
            _lastAccessTime.dwHighDateTime = xmem_utils::hiu32(uLastAccessTime);
            _lastAccessTime.dwLowDateTime  = xmem_utils::lou32(uLastAccessTime);

            FILETIME _lastWriteTime;
            u64      uLastWriteTime       = lastWriteTime.toFileTime();
            _lastWriteTime.dwHighDateTime = xmem_utils::hiu32(uLastWriteTime);
            _lastWriteTime.dwLowDateTime  = xmem_utils::lou32(uLastWriteTime);

            ::SetFileTime((HANDLE)nFileHandle, &_creationTime, &_lastAccessTime, &_lastWriteTime);
            closeFile(nFileHandle);
            return true;
        }
        return false;
    }

    bool FileDevice_PC_System::getFileTime(const xfilepath& szFilename, xfiletimes& ftimes) const
    {
        void* nFileHandle;
        if (openFile(szFilename, true, false, nFileHandle))
        {
            FILETIME _creationTime;
            FILETIME _lastAccessTime;
            FILETIME _lastWriteTime;
            ::GetFileTime((HANDLE)nFileHandle, &_creationTime, &_lastAccessTime, &_lastWriteTime);
            xdatetime CreationTime =
                xdatetime::sFromFileTime((u64)xmem_utils::makeu64(_creationTime.dwLowDateTime, _creationTime.dwHighDateTime));
            xdatetime LastAccessTime = xdatetime::sFromFileTime(
                (u64)xmem_utils::makeu64(_lastAccessTime.dwLowDateTime, _lastAccessTime.dwHighDateTime));
            xdatetime LastWriteTime =
                xdatetime::sFromFileTime((u64)xmem_utils::makeu64(_lastWriteTime.dwLowDateTime, _lastWriteTime.dwHighDateTime));
            ftimes.setCreationTime(CreationTime);
            ftimes.setLastAccessTime(LastAccessTime);
            ftimes.setLastWriteTime(LastWriteTime);
            closeFile(nFileHandle);
            return true;
        }
        return false;
    }

    bool FileDevice_PC_System::setFileAttr(const xfilepath& szFilename, const xfileattrs& attr) const
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

        return ::SetFileAttributesW(LPCWSTR(szFilename.mRunes.m_str), dwFileAttributes) == TRUE;
    }

    bool FileDevice_PC_System::getFileAttr(const xfilepath& szFilename, xfileattrs& attr) const
    {
        DWORD dwFileAttributes = 0;
        dwFileAttributes       = ::GetFileAttributesW(LPCWSTR(szFilename.mRunes.m_str));
        if (dwFileAttributes == INVALID_FILE_ATTRIBUTES)
            return false;

        attr.setArchive(dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE);
        attr.setReadOnly(dwFileAttributes & FILE_ATTRIBUTE_READONLY);
        attr.setHidden(dwFileAttributes & FILE_ATTRIBUTE_HIDDEN);
        attr.setSystem(dwFileAttributes & FILE_ATTRIBUTE_SYSTEM);

        return true;
    }

    static HANDLE sOpenDir(LPCWSTR szDirPath)
    {
        u32 shareType   = FILE_SHARE_READ;
        u32 fileMode    = GENERIC_READ | GENERIC_WRITE;
        u32 disposition = OPEN_EXISTING;
        u32 attrFlags   = FILE_FLAG_BACKUP_SEMANTICS;

        HANDLE handle = ::CreateFileW(szDirPath, fileMode, shareType, NULL, disposition, attrFlags, NULL);
        return handle;
    }

    static void sCloseDir(HANDLE handle) { ::CloseHandle(handle); }

    bool FileDevice_PC_System::hasDir(const xdirpath& szDirPath) const
    {
        HANDLE handle = sOpenDir(LPCWSTR(szDirPath.mRunes.m_str));
        if (handle == INVALID_HANDLE_VALUE)
            return false;
        sCloseDir(handle);
        return true;
    }

    bool FileDevice_PC_System::createDir(const xdirpath& szDirPath) const
    {
        return ::CreateDirectoryW(LPCWSTR(szDirPath.mRunes.m_str), NULL) != 0;
    }

    bool FileDevice_PC_System::moveDir(const xdirpath& szDirPath, const xdirpath& szToDirPath) const
    {
        u32 dwFlags = MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED;
        return ::MoveFileExW(LPCWSTR(szDirPath.mRunes.m_str), (LPCWSTR)szToDirPath.mRunes.m_str, dwFlags) != 0;
    }

    static void changeDirPath(const xdirpath& src, const xdirpath& dst, const xdirinfo& src_dirinfo, xdirpath& result)
    {
        s32      srcdepth = src.getLevels();
        xdirpath dirpath;
        src_dirinfo.getDirpath(dirpath);
        xdirpath parent, child;
        dirpath.split(srcdepth, parent, child);
        result = dst + child;
    }

    static void changeFilePath(const xdirpath& src, const xdirpath& dst, const xfileinfo& src_fileinfo, xfilepath& result)
    {
        s32      srcdepth = src.getLevels();
        xdirpath dirpath;
        src_fileinfo.g(dirpath);
        xdirpath parent, child;
        dirpath.split(srcdepth, parent, child);
        result = dst + child;
    }

    static bool sIsDots(LPCWSTR str)
    {
        return (str[0] == '.' && str[1] == '\0') || (str[0] == '.' && str[1] == '.' && str[2] == '\0');
    }

    struct xdirwalker
    {
        struct xnode
        {
            HANDLE           mFindHandle;
            WIN32_FIND_DATAW mFindData;
            xnode*           mPrev;
        };

        xheap  mNodeHeap;
        xnode* mDirStack;
        s32    mLevel;

        xdirpath  mDirPath;
        xfilepath mFilePath;

        xdirinfo  mDirInfo;
        xfileinfo mFileInfo;

        utf16::runez<4> mWildcard;

        xdirwalker(xalloc* allocator, xdirpath const& dirpath) : mNodeHeap(allocator), mDirPath(dirpath)
        {
            *mWildcard.m_end++ = '*';
            *mWildcard.m_end   = '\0';
        }

        bool enter_dir()
        {
            xnode* nextnode = mNodeHeap.construct<xdirwalker::xnode>();

            utf16::concatenate(mDirPath.mRunes, mWildcard, mDirPath.mAlloc, 16);

            nextnode->mFindHandle = ::FindFirstFileW(LPCWSTR(mDirPath.mRunes.m_str), &nextnode->mFindData);

            mDirPath.mRunes.m_end -= 1;
            *mDirPath.mRunes.m_end = '\0';

            if (nextnode->mFindHandle != INVALID_HANDLE_VALUE)
            {
                // Link new node into linked-list
                nextnode->mPrev = mDirStack;
                mDirStack       = nextnode;
                return true;
            }
            else
            {
                mNodeHeap.destruct(nextnode);
                return false;
            }
        }

        inline bool is_dots() const { return (sIsDots(mDirStack->mFindData.cFileName)); }

        bool is_dir() const { return ((mDirStack->mFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)); }
        bool next() { return ::FindNextFileW(mDirStack->mFindHandle, &mDirStack->mFindData); }

        bool push_dir()
        {
            utf16::runez<4> slash;
            *slash.m_end++ = '\\';
            *slash.m_end   = '\0';

            utf16::runes dirname;
            dirname.m_str = (utf16::prune)mDirStack->mFindData.cFileName;
            dirname.m_end = dirname.m_str;
            dirname.m_eos = (utf16::prune)mDirStack->mFindData.cFileName[sizeof(mDirStack->mFindData.cFileName) - 1];
            while (dirname.m_end < dirname.m_eos && *dirname.m_end != '\0')
            {
                dirname.m_end++;
            }
            utf16::concatenate(mDirPath.mRunes, dirname, mDirPath.mAlloc, 16);
            utf16::concatenate(mDirPath.mRunes, slash, mDirPath.mAlloc, 16);

            // We have found a directory, enter
            if (!enter_dir())
            {
                // Could not enter this directory
                mDirPath.up();
                return false;
            }
            return true;
        }

        bool enumerate_dir(enumerate_delegate& enumerator)
        {
            mDirInfo.mDirPath.mRunes = mDirPath.mRunes;
            return (enumerator(mLevel, nullptr, &mDirInfo));
        }

        bool enumerate_file(enumerate_delegate& enumerator)
        {
            // Prepare FileInfo
            // FilePath = DirPath + mNode->mFindData.cFileName
            mFilePath.mRunes.clear();

            utf16::runes filename;
            filename.m_str = (utf16::prune)mDirStack->mFindData.cFileName;
            filename.m_end = filename.m_str;
            filename.m_eos = (utf16::prune)mDirStack->mFindData.cFileName[sizeof(mDirStack->mFindData.cFileName) - 1];
            while (filename.m_end < filename.m_eos && *filename.m_end != '\0')
            {
                filename.m_end++;
            }

            utf16::concatenate(mFilePath.mRunes, mDirPath.mRunes, filename, mFilePath.mAlloc, 16);
            mFileInfo.mFilePath.mRunes = mFilePath.mRunes;

            return (enumerator(mLevel, &mFileInfo, nullptr));
        }

        bool pop_dir()
        {
            if (::GetLastError() == ERROR_NO_MORE_FILES)
            {
                // No more files here
            }

            // Some error oc.0curred, close the handle and return FALSE
            xnode* node = mDirStack;
            mDirStack   = node->mPrev;

            ::FindClose(node->mFindHandle);
            mNodeHeap.destruct(node);

            return mDirStack != nullptr;
        }
    };

    static bool enumerateCopyDir(xalloc* allocator, const xdirpath& szDirPath, enumerate_delegate& enumerator, s32 depth)
    {
        return true;
    }

    struct enumerate_delegate_copy : public enumerate_delegate
    {
        enumerate_delegate_copy() {}
        virtual ~enumerate_delegate_copy() {}

        virtual bool operator()(s32 depth, const xfileinfo* finf, const xdirinfo* dinf)
        {
            if (dinf != nullptr)
            {
                xdirpath copyDirPath_To;
                changeDirPath(szDirPath, szToDirPath, dinf, copyDirPath_To);
                // nDirinfo_From --------------------->   copyDirPath_To       ( copy dir)
                if (!::CreateDirectoryW(copyDirPath_To.c_str_device(), NULL))
                {
                    return false;
                }
            }
            else if (finf != nullptr)
            {
                xfilepath copyFilePath_To;
                changeFilePath(szDirPath, szToDirPath, finf, copyFilePath_To);
                // nFileinfo_From --------------------->  copyFilePath_To       (copy file)
                if (!::CopyFileW(finf->getFullName().c_str_device(), copyFilePath_To.c_str_device(), false))
                {
                    return false;
                }
            }
            return true;
        }
    };

    bool FileDevice_PC_System::copyDir(const xdirpath& szDirPath, const xdirpath& szToDirPath, bool boOverwrite) const
    {
        enumerate_delegate_copy copy_enum;
        enumerateCopyDir(mAllocator, szDirPath, copy_enum, 0);

        return true;
    }

    struct enumerate_delegate_delete_dir : public enumerate_delegate
    {
        virtual bool operator()(s32 depth, const xfileinfo* finf, const xdirinfo* dinf)
        {
            if (finf != nullptr)
            {
                DWORD dwFileAttributes = ::GetFileAttributesW((LPCTSTR)inf.getFullName().c_str_device());
                if (dwFileAttributes & FILE_ATTRIBUTE_READONLY) // change read-only file mode
                    ::SetFileAttributesW((LPCTSTR)inf.getFullName().c_str_device(),
                                         dwFileAttributes & ~FILE_ATTRIBUTE_READONLY);
                ::DeleteFileW((LPCTSTR)inf.getFullName().c_str_device());
            }
            return true;
        }
    };

    bool FileDevice_PC_System::deleteDir(const xdirpath& szDirPath) const
    {
        enumerate_delegate_delete_dir enumerator;
        return enumerate(szDirPath, &enumerator, 0);
    }

    bool FileDevice_PC_System::setDirTime(const xdirpath& szDirPath, const xfiletimes& ftimes) const
    {
        HANDLE handle = sOpenDir(szDirPath);
        if (handle != INVALID_HANDLE_VALUE)
        {
            xdatetime creationTime;
            ftimes.getCreationTime(creationTime);
            xdatetime lastAccessTime;
            ftimes.getLastAccessTime(lastAccessTime);
            xdatetime lastWriteTime;
            ftimes.getLastWriteTime(lastWriteTime);

            FILETIME _creationTime;
            u64      uCreationTime       = createDateTime.toFileTime();
            _creationTime.dwHighDateTime = xmem_utils::hiu32(uCreationTime);
            _creationTime.dwLowDateTime  = xmem_utils::lou32(uCreationTime);

            FILETIME _lastAccessTime;
            u64      uLastAccessTime       = lastAccessTime.toFileTime();
            _lastAccessTime.dwHighDateTime = xmem_utils::hiu32(uLastAccessTime);
            _lastAccessTime.dwLowDateTime  = xmem_utils::lou32(uLastAccessTime);

            FILETIME _lastWriteTime;
            u64      uLastWriteTime       = lastWriteTime.toFileTime();
            _lastWriteTime.dwHighDateTime = xmem_utils::hiu32(uLastWriteTime);
            _lastWriteTime.dwLowDateTime  = xmem_utils::lou32(uLastWriteTime);

            ::SetFileTime(handle, &_creationTime, &_lastAccessTime, &_lastWriteTime);
            sCloseDir(handle);
            return true;
        }
        return false;
    }

    bool FileDevice_PC_System::getDirTime(const xdirpath& szDirPath, xfiletimes& ftimes) const
    {
        HANDLE handle = sOpenDir(szDirPath);
        if (handle != INVALID_HANDLE_VALUE)
        {
            FILETIME _creationTime;
            FILETIME _lastAccessTime;
            FILETIME _lastWriteTime;
            ::GetFileTime(handle, &_creationTime, &_lastAccessTime, &_lastWriteTime);

            xdatetime  outCreationTime;
            xdatetime& outLastAccessTime;
            xdatetime& outLastWriteTime;
            outCreationTime =
                xdatetime::sFromFileTime((u64)xmem_utils::makeu64(_creationTime.dwLowDateTime, _creationTime.dwHighDateTime));
            outLastAccessTime = xdatetime::sFromFileTime(
                (u64)xmem_utils::makeu64(_lastAccessTime.dwLowDateTime, _lastAccessTime.dwHighDateTime));
            outLastWriteTime =
                xdatetime::sFromFileTime((u64)xmem_utils::makeu64(_lastWriteTime.dwLowDateTime, _lastWriteTime.dwHighDateTime));
            ftimes.setCreationTime(outCreationTime);
            ftimes.setLastAccessTime(outLastAccessTime);
            ftimes.setLastWriteTime(outLastWriteTime);
            sCloseDir(handle);
            return true;
        }
        return false;
    }

    bool FileDevice_PC_System::setDirAttr(const xdirpath& szDirPath, const xfileattrs& attr) const
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

        return ::SetFileAttributes(szDirPath, dwFileAttributes) == TRUE;
    }

    bool FileDevice_PC_System::getDirAttr(const xdirpath& szDirPath, xattributes& attr) const
    {
        DWORD dwFileAttributes = 0;
        dwFileAttributes       = ::GetFileAttributes(szDirPath);
        if (dwFileAttributes == INVALID_FILE_ATTRIBUTES)
            return false;

        attr.setArchive(dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE);
        attr.setReadOnly(dwFileAttributes & FILE_ATTRIBUTE_READONLY);
        attr.setHidden(dwFileAttributes & FILE_ATTRIBUTE_HIDDEN);
        attr.setSystem(dwFileAttributes & FILE_ATTRIBUTE_SYSTEM);

        return true;
    }

    bool FileDevice_PC_System::enumerate(const xdirpath& szDirPath, enumerate_delegate& enumerator, s32 depth) const
    {
        xdirwalker walker(mAllocator, szDirPath);

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

    bool FileDevice_PC_System::seek(void* nFileHandle, ESeekMode mode, u64 pos, u64& newPos) const
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
    bool FileDevice_PC_System::seekOrigin(void* nFileHandle, u64 pos, u64& newPos) const
    {
        return seek(nFileHandle, __SEEK_ORIGIN, pos, newPos);
    }

    bool FileDevice_PC_System::seekCurrent(void* nFileHandle, u64 pos, u64& newPos) const
    {
        return seek(nFileHandle, __SEEK_CURRENT, pos, newPos);
    }

    bool FileDevice_PC_System::seekEnd(void* nFileHandle, u64 pos, u64& newPos) const
    {
        return seek(nFileHandle, __SEEK_END, pos, newPos);
    }
}; // namespace xcore

#endif // TARGET_PC
