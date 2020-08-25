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
    class xfiledevice_pc : public xfiledevice
    {
    public:
        xalloc*  mAllocator;
        xdirpath mDrivePath;
        xbool    mCanWrite;

		XCORE_CLASS_PLACEMENT_NEW_DELETE

        xfiledevice_pc(xalloc* alloc, const xdirpath& pDrivePath, xbool boCanWrite)
            : mAllocator(alloc)
			, mDrivePath(pDrivePath)
            , mCanWrite(boCanWrite)
        {
        }
        virtual ~xfiledevice_pc() {}

        virtual bool canSeek() const { return true; }
        virtual bool canWrite() const { return mCanWrite; }

        virtual bool getDeviceInfo(u64& totalSpace, u64& freeSpace) const ;

        virtual bool openFile(const xfilepath& szFilename, EFileMode mode, EFileAccess access, EFileOp op, void*& nFileHandle);
        virtual bool createFile(const xfilepath& szFilename, bool boRead, bool boWrite, void*& nFileHandle);
        virtual bool readFile(void* nFileHandle, u64 pos, void* buffer, u64 count, u64& outNumBytesRead);
        virtual bool writeFile(void* nFileHandle, u64 pos, const void* buffer, u64 count, u64& outNumBytesWritten);
        virtual bool closeFile(void* nFileHandle);

        virtual bool createStream(xfilepath const& szFilename, bool boRead, bool boWrite, xstream*& strm);
        virtual bool closeStream(xstream* strm);

        virtual bool setLengthOfFile(void* nFileHandle, u64 inLength);
        virtual bool getLengthOfFile(void* nFileHandle, u64& outLength);

        virtual bool setFileTime(const xfilepath& szFilename, const xfiletimes& ftimes);
        virtual bool getFileTime(const xfilepath& szFilename, xfiletimes& ftimes);
        virtual bool setFileAttr(const xfilepath& szFilename, const xfileattrs& attr);
        virtual bool getFileAttr(const xfilepath& szFilename, xfileattrs& attr);

        virtual bool setFileTime(void* pHandle, xfiletimes const& times);
        virtual bool getFileTime(void* pHandle, xfiletimes& outTimes);

        virtual bool hasFile(const xfilepath& szFilename);
        virtual bool moveFile(const xfilepath& szFilename, const xfilepath& szToFilename, bool boOverwrite);
        virtual bool copyFile(const xfilepath& szFilename, const xfilepath& szToFilename, bool boOverwrite);
        virtual bool deleteFile(const xfilepath& szFilename);

        virtual bool hasDir(const xdirpath& szDirPath);
        virtual bool createDir(const xdirpath& szDirPath);
        virtual bool moveDir(const xdirpath& szDirPath, const xdirpath& szToDirPath, bool boOverwrite);
        virtual bool copyDir(const xdirpath& szDirPath, const xdirpath& szToDirPath, bool boOverwrite);
        virtual bool deleteDir(const xdirpath& szDirPath);

        virtual bool setDirTime(const xdirpath& szDirPath, const xfiletimes& ftimes);
        virtual bool getDirTime(const xdirpath& szDirPath, xfiletimes& ftimes);
        virtual bool setDirAttr(const xdirpath& szDirPath, const xfileattrs& attr);
        virtual bool getDirAttr(const xdirpath& szDirPath, xfileattrs& attr);

        virtual bool enumerate(const xdirpath& szDirPath, enumerate_delegate& enumerator);

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

        static HANDLE sOpenDir(xdirpath const& szDirPath);
    };

    xfiledevice* x_CreateFileDevicePC(xalloc* alloc, const xdirpath& pDrivePath, xbool boCanWrite)
    {
        xfiledevice_pc* file_device = alloc->construct<xfiledevice_pc>(alloc, pDrivePath, boCanWrite);
        return file_device;
    }

    void x_DestroyFileDevicePC(xfiledevice* device)
    {
        xfiledevice_pc* file_device = (xfiledevice_pc*)device;
		file_device->mAllocator->destruct(file_device);
    }

    xfiledevice* x_CreateFileDevice(xalloc* allocator, utf32::crunes& pDrivePath, xbool boCanWrite)
	{
		xdirpath drivePath = xfilesystem::dirpath(pDrivePath);
		return x_CreateFileDevicePC(allocator, drivePath, boCanWrite);
	}
    
	void         x_DestroyFileDevice(xfiledevice* fd)
	{
		x_DestroyFileDevicePC(fd);
	}

    bool xfiledevice_pc::getDeviceInfo(u64& totalSpace, u64& freeSpace) const 
    {
        ULARGE_INTEGER totalbytes, freebytes;

        utf16::crunes drivepath16;
        xpath::view_utf16(mDrivePath, drivepath16);

        bool result = false;
        if (GetDiskFreeSpaceExW((LPCWSTR)drivepath16.m_str, NULL, &totalbytes, &freebytes) != 0)
        {
            freeSpace  = freebytes.QuadPart;
            totalSpace = totalbytes.QuadPart;
            result     = true;
        }
        xpath::release_utf16(mDrivePath, drivepath16);
        return result;
    }

    bool xfiledevice_pc::hasFile(const xfilepath& szFilename)
    {
        u32 shareType   = FILE_SHARE_READ;
        u32 fileMode    = GENERIC_READ;
        u32 disposition = OPEN_EXISTING;
        u32 attrFlags   = FILE_ATTRIBUTE_NORMAL;

        utf16::crunes filename16;
        xpath::view_utf16(szFilename, filename16);

        bool   result      = false;
        HANDLE nFileHandle = ::CreateFileW(LPCWSTR(filename16.m_str), fileMode, shareType, NULL, disposition, attrFlags, NULL);
        if (nFileHandle != INVALID_HANDLE_VALUE)
        {
            ::CloseHandle((HANDLE)nFileHandle);
            result = true;
        }

        xpath::release_utf16(szFilename, filename16);
        return result;
    }

    bool xfiledevice_pc::openFile(const xfilepath& szFilename, EFileMode mode, EFileAccess access, EFileOp op, void*& nFileHandle)
    {
        u32 shareType   = FILE_SHARE_READ;
        u32 fileMode    = (access == FileAccess_Read) ? GENERIC_READ : GENERIC_WRITE | GENERIC_READ;
        u32 disposition = OPEN_EXISTING;
        u32 attrFlags   = FILE_ATTRIBUTE_NORMAL;

        utf16::crunes filename16;
        xpath::view_utf16(szFilename, filename16);

        HANDLE handle = ::CreateFileW(LPCWSTR(filename16.m_str), fileMode, shareType, NULL, disposition, attrFlags, NULL);
        nFileHandle   = handle;

        xpath::release_utf16(szFilename, filename16);

        return nFileHandle != INVALID_HANDLE_VALUE;
    }

    bool xfiledevice_pc::createFile(const xfilepath& szFilename, bool boRead, bool boWrite, void*& nFileHandle)
    {
        u32 shareType   = FILE_SHARE_READ;
        u32 fileMode    = !boWrite ? GENERIC_READ : GENERIC_WRITE | GENERIC_READ;
        u32 disposition = CREATE_ALWAYS;
        u32 attrFlags   = FILE_ATTRIBUTE_NORMAL;

        utf16::crunes filename16;
        xpath::view_utf16(szFilename, filename16);

        HANDLE handle = ::CreateFileW((LPCWSTR)filename16.m_str, fileMode, shareType, NULL, disposition, attrFlags, NULL);
        nFileHandle   = handle;

        xpath::release_utf16(szFilename, filename16);

        return nFileHandle != INVALID_HANDLE_VALUE;
    }

    bool xfiledevice_pc::readFile(void* nFileHandle, u64 pos, void* buffer, u64 count, u64& outNumBytesRead)
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
    bool xfiledevice_pc::writeFile(void* nFileHandle, u64 pos, const void* buffer, u64 count, u64& outNumBytesWritten)
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

    bool xfiledevice_pc::moveFile(const xfilepath& szFilename, const xfilepath& szToFilename, bool boOverwrite)
    {
        if (!canWrite())
            return false;

        utf16::crunes filename16;
        xpath::view_utf16(szFilename, filename16);
        utf16::crunes tofilename16;
        xpath::view_utf16(szToFilename, tofilename16);

        BOOL result = ::MoveFileW((LPCWSTR)filename16.m_str, (LPCWSTR)tofilename16.m_str) != 0;

        xpath::release_utf16(szFilename, filename16);
        xpath::release_utf16(szToFilename, tofilename16);

        return result;
    }

    bool xfiledevice_pc::copyFile(const xfilepath& szFilename, const xfilepath& szToFilename, bool boOverwrite)
    {
        if (!canWrite())
            return false;

        const bool failIfExists = boOverwrite == false;

        utf16::crunes filename16;
        xpath::view_utf16(szFilename, filename16);
        utf16::crunes tofilename16;
        xpath::view_utf16(szToFilename, tofilename16);

        BOOL result = ::CopyFileW((LPCWSTR)filename16.m_str, (LPCWSTR)tofilename16.m_str, failIfExists) != 0;

        xpath::release_utf16(szFilename, filename16);
        xpath::release_utf16(szToFilename, tofilename16);

		return result == TRUE;
    }

    bool xfiledevice_pc::closeFile(void* nFileHandle)
    {
        if (!::CloseHandle((HANDLE)nFileHandle))
            return false;
        return true;
    }

    bool xfiledevice_pc::createStream(xfilepath const& szFilename, bool boRead, bool boWrite, xstream*& strm)
	{
		return false;
	}

    bool xfiledevice_pc::closeStream(xstream* strm)
	{
		return false;
	}

    bool xfiledevice_pc::deleteFile(const xfilepath& szFilename)
    {
        utf16::crunes filename16;
        xpath::view_utf16(szFilename, filename16);

        bool result = false;
        if (::DeleteFileW(LPCWSTR(filename16.m_str)))
        {
            result = true;
        }

        xpath::release_utf16(szFilename, filename16);
        return result;
    }

    bool xfiledevice_pc::setLengthOfFile(void* nFileHandle, u64 inLength)
    {
        xsize_t distanceLow  = (xsize_t)inLength;
        xsize_t distanceHigh = (xsize_t)(inLength >> 32);
        ::SetFilePointer((HANDLE)nFileHandle, (LONG)distanceLow, (PLONG)&distanceHigh, FILE_BEGIN);
        ::SetEndOfFile((HANDLE)nFileHandle);
        return true;
    }

    bool xfiledevice_pc::getLengthOfFile(void* nFileHandle, u64& outLength)
    {
        DWORD lowSize, highSize;
        lowSize   = ::GetFileSize((HANDLE)nFileHandle, &highSize);
        outLength = highSize;
        outLength = outLength << 16;
        outLength = outLength << 16;
        outLength = outLength | lowSize;
        return true;
    }

    bool xfiledevice_pc::setFileTime(const xfilepath& szFilename, const xfiletimes& ftimes)
    {
        void* nFileHandle;
        if (openFile(szFilename, FileMode_Open, FileAccess_Read, FileOp_Sync, nFileHandle))
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

    bool xfiledevice_pc::getFileTime(const xfilepath& szFilename, xfiletimes& ftimes)
    {
        void* nFileHandle;
        if (openFile(szFilename, FileMode_Open, FileAccess_Read, FileOp_Sync, nFileHandle))
        {
            FILETIME _creationTime;
            FILETIME _lastAccessTime;
            FILETIME _lastWriteTime;
            ::GetFileTime((HANDLE)nFileHandle, &_creationTime, &_lastAccessTime, &_lastWriteTime);
            xdatetime CreationTime   = xdatetime::sFromFileTime((u64)xmem_utils::makeu64(_creationTime.dwLowDateTime, _creationTime.dwHighDateTime));
            xdatetime LastAccessTime = xdatetime::sFromFileTime((u64)xmem_utils::makeu64(_lastAccessTime.dwLowDateTime, _lastAccessTime.dwHighDateTime));
            xdatetime LastWriteTime  = xdatetime::sFromFileTime((u64)xmem_utils::makeu64(_lastWriteTime.dwLowDateTime, _lastWriteTime.dwHighDateTime));
            ftimes.setCreationTime(CreationTime);
            ftimes.setLastAccessTime(LastAccessTime);
            ftimes.setLastWriteTime(LastWriteTime);
            closeFile(nFileHandle);
            return true;
        }
        return false;
    }

    bool xfiledevice_pc::setFileAttr(const xfilepath& szFilename, const xfileattrs& attr)
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

        utf16::crunes filename16;
        xpath::view_utf16(szFilename, filename16);

        bool result = ::SetFileAttributesW(LPCWSTR(filename16.m_str), dwFileAttributes) == TRUE;

        xpath::release_utf16(szFilename, filename16);

        return result;
    }

    bool xfiledevice_pc::getFileAttr(const xfilepath& szFilename, xfileattrs& attr)
    {
        DWORD dwFileAttributes = 0;

        utf16::crunes filename16;
        xpath::view_utf16(szFilename, filename16);

        bool result      = false;
        dwFileAttributes = ::GetFileAttributesW(LPCWSTR(filename16.m_str));
        if (dwFileAttributes != INVALID_FILE_ATTRIBUTES)
        {
            result = true;
            attr.setArchive(dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE);
            attr.setReadOnly(dwFileAttributes & FILE_ATTRIBUTE_READONLY);
            attr.setHidden(dwFileAttributes & FILE_ATTRIBUTE_HIDDEN);
            attr.setSystem(dwFileAttributes & FILE_ATTRIBUTE_SYSTEM);
        }

        xpath::release_utf16(szFilename, filename16);

        return result;
    }


    bool xfiledevice_pc::setFileTime(void* nFileHandle, const xfiletimes& ftimes)
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
        return true;
    }

    bool xfiledevice_pc::getFileTime(void* nFileHandle, xfiletimes& ftimes)
    {
        FILETIME _creationTime;
        FILETIME _lastAccessTime;
        FILETIME _lastWriteTime;
        ::GetFileTime((HANDLE)nFileHandle, &_creationTime, &_lastAccessTime, &_lastWriteTime);
        xdatetime CreationTime   = xdatetime::sFromFileTime((u64)xmem_utils::makeu64(_creationTime.dwLowDateTime, _creationTime.dwHighDateTime));
        xdatetime LastAccessTime = xdatetime::sFromFileTime((u64)xmem_utils::makeu64(_lastAccessTime.dwLowDateTime, _lastAccessTime.dwHighDateTime));
        xdatetime LastWriteTime  = xdatetime::sFromFileTime((u64)xmem_utils::makeu64(_lastWriteTime.dwLowDateTime, _lastWriteTime.dwHighDateTime));
        ftimes.setCreationTime(CreationTime);
        ftimes.setLastAccessTime(LastAccessTime);
        ftimes.setLastWriteTime(LastWriteTime);
        return true;
    }

    HANDLE xfiledevice_pc::sOpenDir(xdirpath const& szDirPath)
    {
        u32 shareType   = FILE_SHARE_READ;
        u32 fileMode    = GENERIC_READ | GENERIC_WRITE;
        u32 disposition = OPEN_EXISTING;
        u32 attrFlags   = FILE_FLAG_BACKUP_SEMANTICS;

        utf16::crunes path16;
        xpath::view_utf16(szDirPath, path16);
        HANDLE handle = ::CreateFileW((LPCWSTR)path16.m_str, fileMode, shareType, NULL, disposition, attrFlags, NULL);
        xpath::release_utf16(szDirPath, path16);
        return handle;
    }

    static void sCloseDir(HANDLE handle) { ::CloseHandle(handle); }

    bool xfiledevice_pc::hasDir(const xdirpath& szDirPath)
    {
        HANDLE handle = sOpenDir(szDirPath);
        if (handle == INVALID_HANDLE_VALUE)
            return false;
        sCloseDir(handle);
        return true;
    }

    bool xfiledevice_pc::createDir(const xdirpath& szDirPath)
    {
        utf16::crunes filename16;
        xpath::view_utf16(szDirPath, filename16);
        BOOL result = ::CreateDirectoryW(LPCWSTR(filename16.m_str), NULL) != 0;
        xpath::release_utf16(szDirPath, filename16);
        return result;
    }

    bool xfiledevice_pc::moveDir(const xdirpath& szDirPath, const xdirpath& szToDirPath, bool boOverwrite)
    {
        u32          dwFlags = MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED;
        utf16::crunes dirpath16;
        xpath::view_utf16(szDirPath, dirpath16);
        utf16::crunes todirpath16;
        xpath::view_utf16(szToDirPath, todirpath16);
        BOOL result = ::MoveFileExW((LPCWSTR)dirpath16.m_str, (LPCWSTR)todirpath16.m_str, dwFlags) != 0;
        xpath::release_utf16(szDirPath, dirpath16);
        xpath::release_utf16(szToDirPath, todirpath16);
        return result;
    }

    static void changeDirPath(const xdirpath& src, const xdirpath& dst, xdirinfo const* src_dirinfo, xdirpath& result)
    {
        xdirpath srcdirpath(src);
        xdirpath curdirpath = src_dirinfo->getDirpath();
        curdirpath.makeRelativeTo(srcdirpath);
        result = dst + curdirpath;
    }

    static void changeFilePath(const xdirpath& src, const xdirpath& dst, xfileinfo const* src_fileinfo, xfilepath& result)
    {
        xdirpath  srcdirpath(src);
        xfilepath curfilepath;
        src_fileinfo->getFilepath(curfilepath);
        curfilepath.makeRelativeTo(srcdirpath);
        result = dst + curfilepath;
    }

    static bool sIsDots(LPCWSTR str) { return (str[0] == '.' && str[1] == '\0') || (str[0] == '.' && str[1] == '.' && str[2] == '\0'); }

    struct xdirwalker
    {
        class xnode
        {
		public:
            HANDLE           mFindHandle;
            WIN32_FIND_DATAW mFindData;
            xnode*           mPrev;

			XCORE_CLASS_PLACEMENT_NEW_DELETE
        };

        xalloc*  mNodeHeap;
        xnode* mDirStack;
        s32    mLevel;

        xpath mDirPath;
        xpath mFilePath;

        xdirinfo  mDirInfo;
        xfileinfo mFileInfo;

        utf32::runez<4> mWildcard;

        xdirwalker(xdirpath const& dirpath)
            : mNodeHeap(nullptr)
            , mDirPath()
        {
            mDirPath  = xfilesys::get_xpath(dirpath);
            mNodeHeap = xfilesys::get_filesystem(dirpath)->m_allocator;

            *mWildcard.m_end++ = '*';
            *mWildcard.m_end   = '\0';
        }

        bool enter_dir()
        {
            xnode* nextnode = mNodeHeap->construct<xdirwalker::xnode>();

            utf32::concatenate(mDirPath.m_path, mWildcard, mDirPath.m_alloc, 16);
            utf16::crunes dirpath16;
            xpath::view_utf16(mDirPath, dirpath16);
            nextnode->mFindHandle = ::FindFirstFileW(LPCWSTR(dirpath16.m_str), &nextnode->mFindData);
            xpath::release_utf16(mDirPath, dirpath16);
            mDirPath.m_path.m_end -= 1;
            *mDirPath.m_path.m_end = '\0';

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

        inline bool is_dots() { return (sIsDots(mDirStack->mFindData.cFileName)); }

        bool is_dir() { return ((mDirStack->mFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)); }
        bool next() { return ::FindNextFileW(mDirStack->mFindHandle, &mDirStack->mFindData); }

        bool push_dir()
        {
            utf16::crunes dirname;
            dirname.m_str = (utf16::prune)mDirStack->mFindData.cFileName;
            dirname.m_end = dirname.m_str;
            while (*dirname.m_end != '\0')
            {
                dirname.m_end++;
            }
            xpath::append_utf16(mDirPath, dirname);

            utf32::runez<4> slash;
            *slash.m_end++ = '\\';
            *slash.m_end   = '\0';

            utf32::concatenate(mDirPath.m_path, slash, mDirPath.m_alloc, 16);

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
            xpath& dirinfopath = xfilesys::get_xpath(mDirInfo);
            dirinfopath.copy_dirpath(mDirPath.m_path);
            return (enumerator(mLevel, nullptr, &mDirInfo));
        }

        bool enumerate_file(enumerate_delegate& enumerator)
        {
            // Prepare FileInfo
            // FilePath = DirPath + mNode->mFindData.cFileName
            mFilePath.m_path.clear();

            utf16::runes filename;
            filename.m_str = (utf16::prune)mDirStack->mFindData.cFileName;
            filename.m_end = filename.m_str;
            filename.m_eos = (utf16::prune)mDirStack->mFindData.cFileName[sizeof(mDirStack->mFindData.cFileName) - 1];
            while (filename.m_end < filename.m_eos && *filename.m_end != '\0')
            {
                filename.m_end++;
            }
            mFilePath.copy_dirpath(mDirPath.m_path);
            xpath::append_utf16(mFilePath, filename);

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
            mNodeHeap->destruct(node);

            return mDirStack != nullptr;
        }
    };

    struct enumerate_delegate_copy : public enumerate_delegate
    {
        xdirpath const& mSrcDir;
        xdirpath const& mDstDir;
		xfiledevice*	mDstDevice;
        bool            mOverwrite;

        enumerate_delegate_copy(xdirpath const& srcdir, xdirpath const& dstdir, xfiledevice* dstdevice, bool overwrite)
            : mSrcDir(srcdir)
			, mDstDir(dstdir)
			, mDstDevice(dstdevice)
            , mOverwrite(overwrite)
        {
        }

        virtual bool operator()(s32 depth, const xfileinfo* finf, const xdirinfo* dinf)
        {
            if (mDstDevice != nullptr)
            {
				if (dinf != nullptr)
				{
					xdirpath subpath;
					dinf->getDirpath().makeRelativeTo(mSrcDir, subpath);
					xdirpath dstdirpath = mDstDir + subpath;
					mDstDevice->createDir(dstdirpath);
					return true;
				}
				else if (finf != nullptr)
				{
					xfilepath dstfilepath = finf->getFilepath();
					dstfilepath.makeRelativeTo(mSrcDir);
					dstfilepath.makeAbsoluteTo(mDstDir);
					mDstDevice->copyFile(finf->getFilepath(), dstfilepath, true);
					return true;
				}
			}
            return false;
        }
    };

    bool xfiledevice_pc::copyDir(const xdirpath& szDirPath, const xdirpath& szToDirPath, bool boOverwrite)
    {
        enumerate_delegate_copy copy_enum(szDirPath, szToDirPath, this, boOverwrite);
        enumerate(szDirPath, copy_enum);
        return true;
    }

    struct enumerate_delegate_delete_dir : public enumerate_delegate
    {
        utf16::alloc* mStrAlloc;

        virtual bool operator()(s32 depth, const xfileinfo* finf, const xdirinfo* dinf)
        {
            if (finf != nullptr)
            {
                const xfilepath& fp = finf->getFilepath();
				utf16::crunes runes16;
				xpath::view_utf16(fp, runes16);
;
                DWORD dwFileAttributes = ::GetFileAttributesW((LPCWSTR)runes16.m_str);
                if (dwFileAttributes & FILE_ATTRIBUTE_READONLY) // change read-only file mode
                    ::SetFileAttributesW((LPCWSTR)runes16.m_str, dwFileAttributes & ~FILE_ATTRIBUTE_READONLY);
                ::DeleteFileW((LPCWSTR)runes16.m_str);

				 xpath::release_utf16(fp, runes16);
            }
            return true;
        }
    };

    bool xfiledevice_pc::deleteDir(const xdirpath& szDirPath)
    {
        enumerate_delegate_delete_dir enumerator;
        return enumerate(szDirPath, enumerator);
    }

    bool xfiledevice_pc::setDirTime(const xdirpath& szDirPath, const xfiletimes& ftimes)
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

            ::SetFileTime(handle, &_creationTime, &_lastAccessTime, &_lastWriteTime);
            sCloseDir(handle);
            return true;
        }
        return false;
    }

    bool xfiledevice_pc::getDirTime(const xdirpath& szDirPath, xfiletimes& ftimes)
    {
        HANDLE handle = sOpenDir(szDirPath);
        if (handle != INVALID_HANDLE_VALUE)
        {
            FILETIME _creationTime;
            FILETIME _lastAccessTime;
            FILETIME _lastWriteTime;
            ::GetFileTime(handle, &_creationTime, &_lastAccessTime, &_lastWriteTime);

            xdatetime outCreationTime;
            xdatetime outLastAccessTime;
            xdatetime outLastWriteTime;
            outCreationTime   = xdatetime::sFromFileTime((u64)xmem_utils::makeu64(_creationTime.dwLowDateTime, _creationTime.dwHighDateTime));
            outLastAccessTime = xdatetime::sFromFileTime((u64)xmem_utils::makeu64(_lastAccessTime.dwLowDateTime, _lastAccessTime.dwHighDateTime));
            outLastWriteTime  = xdatetime::sFromFileTime((u64)xmem_utils::makeu64(_lastWriteTime.dwLowDateTime, _lastWriteTime.dwHighDateTime));
            ftimes.setCreationTime(outCreationTime);
            ftimes.setLastAccessTime(outLastAccessTime);
            ftimes.setLastWriteTime(outLastWriteTime);
            sCloseDir(handle);
            return true;
        }
        return false;
    }

    bool xfiledevice_pc::setDirAttr(const xdirpath& szDirPath, const xfileattrs& attr)
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

        utf16::crunes dirpath16;
        xpath::view_utf16(szDirPath, dirpath16);
        bool result = ::SetFileAttributesW((LPCWSTR)dirpath16.m_str, dwFileAttributes) == TRUE;
        xpath::release_utf16(szDirPath, dirpath16);
        return result;
    }

    bool xfiledevice_pc::getDirAttr(const xdirpath& szDirPath, xfileattrs& attr)
    {
        utf16::crunes dirpath16;
        xpath::view_utf16(szDirPath, dirpath16);

        bool  result           = false;
        DWORD dwFileAttributes = ::GetFileAttributesW((LPCWSTR)dirpath16.m_str);
        if (dwFileAttributes != INVALID_FILE_ATTRIBUTES)
        {
            result = true;
            attr.setArchive(dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE);
            attr.setReadOnly(dwFileAttributes & FILE_ATTRIBUTE_READONLY);
            attr.setHidden(dwFileAttributes & FILE_ATTRIBUTE_HIDDEN);
            attr.setSystem(dwFileAttributes & FILE_ATTRIBUTE_SYSTEM);
        }
        xpath::release_utf16(szDirPath, dirpath16);
        return true;
    }

    bool xfiledevice_pc::enumerate(const xdirpath& szDirPath, enumerate_delegate& enumerator)
    {
        xdirwalker walker(szDirPath);

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

    bool xfiledevice_pc::seek(void* nFileHandle, ESeekMode mode, u64 pos, u64& newPos)
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
    bool xfiledevice_pc::seekOrigin(void* nFileHandle, u64 pos, u64& newPos) { return seek(nFileHandle, __SEEK_ORIGIN, pos, newPos); }
    bool xfiledevice_pc::seekCurrent(void* nFileHandle, u64 pos, u64& newPos) { return seek(nFileHandle, __SEEK_CURRENT, pos, newPos); }
    bool xfiledevice_pc::seekEnd(void* nFileHandle, u64 pos, u64& newPos) { return seek(nFileHandle, __SEEK_END, pos, newPos); }


}; // namespace xcore

#endif // TARGET_PC
