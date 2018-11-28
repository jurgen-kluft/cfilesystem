#include "xbase/x_target.h"
#ifdef TARGET_PC

//==============================================================================
// INCLUDES
//==============================================================================
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOKANJI
#include <windows.h>
#include <stdio.h>

#include "xbase/x_debug.h"
#include "xbase/x_limits.h"
#include "xbase/x_memory_std.h"
#include "xbase/x_string_std.h"
#include "xbase/x_va_list.h"
#include "xbase/x_integer.h"

#include "xtime/x_datetime.h"

#include "xfilesystem/private/x_filesystem_common.h"
#include "xfilesystem/private/x_filesystem_win32.h"
#include "xfilesystem/x_attributes.h"
#include "xfilesystem/x_filedevice.h"
#include "xfilesystem/x_fileinfo.h"
#include "xfilesystem/x_dirinfo.h"
#include "xfilesystem/private/x_filedata.h"
#include "xfilesystem/private/x_fileasync.h"
#include "xfilesystem/private/x_filesystem_cstack.h"

namespace xcore
{
    class FileDevice_PC_System : public xfiledevice
    {
        xstring mDrivePath;
        xbool   mCanWrite;

        wchar_t mLPWStr[8192];
        s32     mLPWStrPos;
        LPCWSTR ConvertToLPCWSTR(xstring const &str, bool reset = true);

      public:
        FileDevice_PC_System(const xstring &pDrivePath, xbool boCanWrite)
         : mDrivePath(pDrivePath), mCanWrite(boCanWrite)
        {
        }
        virtual ~FileDevice_PC_System() {}

        virtual bool canSeek() const { return true; }
        virtual bool canWrite() const { return mCanWrite; }

        virtual bool getDeviceInfo(u64 &totalSpace, u64 &freeSpace) const;

        virtual bool openFile(const xstring &szFilename, bool boRead, bool boWrite, void *&nFileHandle) const;
        virtual bool createFile(const xstring &szFilename, bool boRead, bool boWrite, void *&nFileHandle) const;
        virtual bool readFile(void *nFileHandle, u64 pos, void *buffer, u64 count, u64 &outNumBytesRead) const;
        virtual bool writeFile(
            void *nFileHandle, u64 pos, const void *buffer, u64 count, u64 &outNumBytesWritten) const;
        virtual bool closeFile(void *nFileHandle) const;

        virtual bool setLengthOfFile(u32 nFileHandle, u64 inLength) const;
        virtual bool getLengthOfFile(u32 nFileHandle, u64 &outLength) const;

        virtual bool setFileTime(const xstring &szFilename, const xdatetime &creationTime,
            const xdatetime &lastAccessTime, const xdatetime &lastWriteTime) const;
        virtual bool getFileTime(const xstring &szFilename, xdatetime &outCreationTime, xdatetime &outLastAccessTime,
            xdatetime &outLastWriteTime) const;
        virtual bool setFileAttr(const xstring &szFilename, const xattributes &attr) const;
        virtual bool getFileAttr(const xstring &szFilename, xattributes &attr) const;

        virtual bool hasFile(const xstring &szFilename) const;
        virtual bool moveFile(const xstring &szFilename, const xstring &szToFilename) const;
        virtual bool copyFile(const xstring &szFilename, const xstring &szToFilename, bool boOverwrite) const;
        virtual bool deleteFile(const xstring &szFilename) const;

        virtual bool hasDir(const xstring &szDirPath) const;
        virtual bool createDir(const xstring &szDirPath) const;
        virtual bool moveDir(const xstring &szDirPath, const xstring &szToDirPath) const;
        virtual bool copyDir(const xstring &szDirPath, const xstring &szToDirPath, bool boOverwrite) const;
        virtual bool deleteDir(const xstring &szDirPath) const;

        virtual bool setDirTime(const xstring &szDirPath, const xdatetime &creationTime,
            const xdatetime &lastAccessTime, const xdatetime &lastWriteTime) const;
        virtual bool getDirTime(const xstring &szDirPath, xdatetime &outCreationTime, xdatetime &outLastAccessTime,
            xdatetime &outLastWriteTime) const;
        virtual bool setDirAttr(const xstring &szDirPath, const xattributes &attr) const;
        virtual bool getDirAttr(const xstring &szDirPath, xattributes &attr) const;

        virtual bool enumerate(const xstring &szDirPath, enumerate_delegate *enumerator, s32 depth = 0) const;

        enum ESeekMode
        {
            __SEEK_ORIGIN  = 1,
            __SEEK_CURRENT = 2,
            __SEEK_END     = 3,
        };
        bool seek(u32 nFileHandle, ESeekMode mode, u64 pos, u64 &newPos) const;
        bool seekOrigin(u32 nFileHandle, u64 pos, u64 &newPos) const;
        bool seekCurrent(u32 nFileHandle, u64 pos, u64 &newPos) const;
        bool seekEnd(u32 nFileHandle, u64 pos, u64 &newPos) const;

        XFILESYSTEM_OBJECT_NEW_DELETE()
    };

    xfiledevice *x_CreateFileDevicePC(const xstring &pDrivePath, xbool boCanWrite)
    {
        FileDevice_PC_System *file_device = new FileDevice_PC_System(pDrivePath, boCanWrite);
        return file_device;
    }

    void x_DestroyFileDevicePC(xfiledevice *device)
    {
        FileDevice_PC_System *file_device = (FileDevice_PC_System *)device;
        delete file_device;
    }

    bool FileDevice_PC_System::getDeviceInfo(u64 &totalSpace, u64 &freeSpace) const
    {
        ULARGE_INTEGER totalbytes, freebytes;
        if (GetDiskFreeSpaceExW(ConvertToLPCWSTR(mDrivePath), NULL, &totalbytes, &freebytes) == 0)
            return false;

        freeSpace  = freebytes.QuadPart;
        totalSpace = totalbytes.QuadPart;
        return true;
    }

    bool FileDevice_PC_System::hasFile(const xstring &szFilename) const
    {
        u32 shareType   = FILE_SHARE_READ;
        u32 fileMode    = GENERIC_READ;
        u32 disposition = OPEN_EXISTING;
        u32 attrFlags   = FILE_ATTRIBUTE_NORMAL;

        HANDLE nFileHandle =
            ::CreateFile(ConvertToLPCWSTR(szFilename), fileMode, shareType, NULL, disposition, attrFlags, NULL);
        if (nFileHandle != INVALID_HANDLE_VALUE)
        {
            ::CloseHandle((HANDLE)nFileHandle);
            return true;
        }
        return false;
    }

    bool FileDevice_PC_System::openFile(const xstring &szFilename, bool boRead, bool boWrite, void *&nFileHandle) const
    {
        u32 shareType   = FILE_SHARE_READ;
        u32 fileMode    = !boWrite ? GENERIC_READ : GENERIC_WRITE | GENERIC_READ;
        u32 disposition = OPEN_EXISTING;
        u32 attrFlags   = FILE_ATTRIBUTE_NORMAL;

        HANDLE handle =
            ::CreateFile(ConvertToLPCWSTR(szFilename), fileMode, shareType, NULL, disposition, attrFlags, NULL);
        nFileHandle = (u32)handle;
        return nFileHandle != (u32)INVALID_HANDLE_VALUE;
    }

    bool FileDevice_PC_System::createFile(
        const xstring &szFilename, bool boRead, bool boWrite, void *&nFileHandle) const
    {
        u32 shareType   = FILE_SHARE_READ;
        u32 fileMode    = !boWrite ? GENERIC_READ : GENERIC_WRITE | GENERIC_READ;
        u32 disposition = CREATE_ALWAYS;
        u32 attrFlags   = FILE_ATTRIBUTE_NORMAL;

        HANDLE handle =
            ::CreateFileW(ConvertToLPCWSTR(szFilename), fileMode, shareType, NULL, disposition, attrFlags, NULL);
        nFileHandle = (u32)handle;
        return nFileHandle != (u32)(INVALID_HANDLE_VALUE);
    }

    bool FileDevice_PC_System::readFile(void *nFileHandle, u64 pos, void *buffer, u64 count, u64 &outNumBytesRead) const
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
                    case ERROR_HANDLE_EOF:    // We have reached the end of the FilePC during the call to ReadFile
                        return false;
                    case ERROR_IO_PENDING:
                        return false;
                    default:
                        return false;
                }
            }

            return true;
        }
        return false;
    }
    bool FileDevice_PC_System::writeFile(
        void *nFileHandle, u64 pos, const void *buffer, u64 count, u64 &outNumBytesWritten) const
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
                    case ERROR_HANDLE_EOF:    // We have reached the end of the FilePC during the call to WriteFile
                        return false;
                    case ERROR_IO_PENDING:
                        return false;
                    default:
                        return false;
                }
            }

            return true;
        }
        return false;
    }

    bool FileDevice_PC_System::moveFile(const xstring &szFilename, const xstring &szToFilename) const
    {
        if (!canWrite())
            return false;

        LPCWSTR wfilename   = ConvertToLPCWSTR(szFilename);
        LPCWSTR wtofilename = ConvertToLPCWSTR(szToFilename, false);
        return ::MoveFileW(wfilename, wtofilename) != 0;
    }

    bool FileDevice_PC_System::copyFile(const xstring &szFilename, const xstring &szToFilename, bool boOverwrite) const
    {
        if (!canWrite())
            return false;

        const bool failIfExists = boOverwrite == false;
        LPCWSTR    wfilename    = ConvertToLPCWSTR(szFilename);
        LPCWSTR    wtofilename  = ConvertToLPCWSTR(szToFilename, false);
        return ::CopyFileW(wfilename, wtofilename, failIfExists) != 0;
    }

    bool FileDevice_PC_System::closeFile(void *nFileHandle) const
    {
        if (!::CloseHandle((HANDLE)nFileHandle))
            return false;
        return true;
    }

    bool FileDevice_PC_System::deleteFile(const xstring &szFilename) const
    {
        if (!::DeleteFileW(ConvertToLPCWSTR(szFilename)))
            return false;
        return true;
    }

    bool FileDevice_PC_System::setLengthOfFile(void *nFileHandle, u64 inLength) const
    {
        xsize_t distanceLow  = (xsize_t)inLength;
        xsize_t distanceHigh = (xsize_t)(inLength >> 32);
        ::SetFilePointer((HANDLE)nFileHandle, (LONG)distanceLow, (PLONG)&distanceHigh, FILE_BEGIN);
        ::SetEndOfFile((HANDLE)nFileHandle);
        return true;
    }

    bool FileDevice_PC_System::getLengthOfFile(void *nFileHandle, u64 &outLength) const
    {
        DWORD lowSize, highSize;
        lowSize   = ::GetFileSize((HANDLE)nFileHandle, &highSize);
        outLength = highSize;
        outLength = outLength << 16;
        outLength = outLength << 16;
        outLength = outLength | lowSize;
        return true;
    }

    bool FileDevice_PC_System::setFileTime(const xstring &szFilename, const xdatetime &creationTime,
        const xdatetime &lastAccessTime, const xdatetime &lastWriteTime) const
    {
        void *nFileHandle;
        if (openFile(szFilename, true, true, nFileHandle))
        {
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

    bool FileDevice_PC_System::getFileTime(const xstring &szFilename, xdatetime &outCreationTime,
        xdatetime &outLastAccessTime, xdatetime &outLastWriteTime) const
    {
        u32 nFileHandle;
        if (openFile(szFilename, true, false, nFileHandle))
        {
            FILETIME _creationTime;
            FILETIME _lastAccessTime;
            FILETIME _lastWriteTime;
            ::GetFileTime((HANDLE)nFileHandle, &_creationTime, &_lastAccessTime, &_lastWriteTime);
            outCreationTime = xdatetime::sFromFileTime(
                (u64)xmem_utils::makeu64(_creationTime.dwLowDateTime, _creationTime.dwHighDateTime));
            outLastAccessTime = xdatetime::sFromFileTime(
                (u64)xmem_utils::makeu64(_lastAccessTime.dwLowDateTime, _lastAccessTime.dwHighDateTime));
            outLastWriteTime = xdatetime::sFromFileTime(
                (u64)xmem_utils::makeu64(_lastWriteTime.dwLowDateTime, _lastWriteTime.dwHighDateTime));
            closeFile(nFileHandle);
            return true;
        }
        return false;
    }

    bool FileDevice_PC_System::setFileAttr(const xstring &szFilename, const xattributes &attr) const
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

        return ::SetFileAttributesW(ConvertToLPCWSTR(szFilename), dwFileAttributes) == TRUE;
    }

    bool FileDevice_PC_System::getFileAttr(const xstring &szFilename, xattributes &attr) const
    {
        DWORD dwFileAttributes = 0;
        dwFileAttributes       = ::GetFileAttributesW(ConvertToLPCWSTR(szFilename));
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

    bool FileDevice_PC_System::hasDir(const xstring &szDirPath) const
    {
        HANDLE handle = sOpenDir(ConvertToLPCWSTR(szDirPath));
        if (handle == INVALID_HANDLE_VALUE)
            return false;
        sCloseDir(handle);
        return true;
    }

    bool FileDevice_PC_System::createDir(const xstring &szDirPath) const
    {
        return ::CreateDirectoryW(ConvertToLPCWSTR(szDirPath), NULL) != 0;
    }

    bool FileDevice_PC_System::moveDir(const xstring &szDirPath, const xstring &szToDirPath) const
    {
        u32 dwFlags = MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED;
        return ::MoveFileExW(ConvertToLPCWSTR(szDirPath), szToDirPath, dwFlags) != 0;
    }

    static void changeDirPath(
        const xstring &szDirPath, const xstring &szToDirPath, const xdirinfo *szDirinfo, xdirpath &outDirPath)
    {
        xdirpath nDirpath_from(szDirPath);
        xdirpath nDirpath_to(szToDirPath);
        s32      depth1 = nDirpath_from.getLevels();
        xdirpath parent, child;
        szDirinfo->getFullName().split(depth1, parent, child);
        nDirpath_to.getSubDir(child, outDirPath);
    }

    static void changeFilePath(
        const xstring &szDirPath, const xstring &szToDirPath, const xfileinfo *szFileinfo, xfilepath &outFilePath)
    {
        xdirpath nDir;
        szFileinfo->getFullName().getDirPath(nDir);
        xdirpath nDirpath_from(szDirPath);
        xdirpath nDirpath_to(szToDirPath);

        xfilepath fileName = szFileinfo->getFullName();
        fileName.onlyFilename();
        s32      depth = nDirpath_from.getLevels();
        xdirpath parent, child, copyDirPath_To;
        nDir.split(depth, parent, child);
        nDirpath_to.getSubDir(child.c_str(), copyDirPath_To);
        outFilePath = xfilepath(copyDirPath_To, fileName);
    }

    static bool sIsDots(LPCWSTR str) { return (x_strcmp(str, ".") == 0 || x_strcmp(str, "..") == 0); }

    struct enumerate_delegate_dirs_copy_dir : public enumerate_delegate
    {
        cstack<const xdirinfo *> dirStack;
        enumerate_delegate_dirs_copy_dir() { dirStack.init(getAllocator(), MAX_ENUM_SEARCH_DIRS); }
        virtual ~enumerate_delegate_dirs_copy_dir() { dirStack.clear(); }

        virtual bool operator()(s32 depth, const xfileinfo *finf, const xdirinfo *dinf)
        {
            if (dinf != nullptr)
            {
                dirStack.push(inf);
            }
            return true;
        }
    };

    struct enumerate_delegate_files_copy_dir : public enumerate_delegate
    {
        cstack<const xfileinfo *> fileStack;

        enumerate_delegate_files_copy_dir() { fileStack.init(getAllocator(), MAX_ENUM_SEARCH_FILES); }
        virtual ~enumerate_delegate_files_copy_dir() { fileStack.clear(); }

        virtual bool operator()(s32 depth, const xfileinfo *finf, const xdirinfo *dinf)
        {
            if (finf != nullptr)
            {
                fileStack.push(finf);
            }
            return true;
        }
    };

    static bool enumerateCopyDir(const xstring &szDirPath, enumerate_delegate *enumerator, s32 depth)
    {
        HANDLE hFind;    // file handle

        WIN32_FIND_DATAW FindFileData;

        char     DirPathBuffer[MAX_PATH + 2];
        char     FileNameBuffer[MAX_PATH + 2];
        xcstring DirPath(DirPathBuffer, sizeof(DirPathBuffer), szDirPath);
        DirPath += "\\*";

        xcstring FileName(FileNameBuffer, sizeof(FileNameBuffer), szDirPath);    // searching all files
        FileName += "\\";

        hFind = ::FindFirstFileW(ConvertToLPCWSTR(DirPath), &FindFileData);    // find the first file
        if (hFind == INVALID_HANDLE_VALUE)
            return false;

        DirPath = FileName;

        bool bSearch = true;
        while (bSearch)
        {    // until we find an entry
            if (::FindNextFileW(hFind, &FindFileData))
            {
                if (sIsDots(FindFileData.cFileName))
                    continue;

                FileName += FindFileData.cFileName;

                if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                {
                    // We have found a directory, recurse
                    if (!enumerateCopyDir(FileName.c_str(), enumerator, depth + 1))
                    {
                        ::FindClose(hFind);
                        return false;    // directory couldn't be enumerated
                    }

                    FileName = DirPath;
                }
                else
                {
                    xfileinfo *fi = new xfileinfo(FileName.c_str());
                    if (file_enumerator != NULL && fi != NULL)
                    {
                        bool terminate;
                        (*file_enumerator)(depth, fi, terminate);
                        bSearch = !terminate;
                    }
                    else
                    {
                        delete fi;
                        fi = NULL;
                    }
                    FileName = DirPath;
                }
            }
            else
            {
                if (::GetLastError() == ERROR_NO_MORE_FILES)
                {
                    // No more files here
                    bSearch = false;
                }
                else
                {
                    // Some error occurred, close the handle and return FALSE
                    ::FindClose(hFind);
                    return false;
                }
            }
        }
        ::FindClose(hFind);    // Closing file handle

        if (dir_enumerator != NULL)
        {
            xdirinfo *di = new xdirinfo(FileName.c_str());
            bool      terminate;
            if (di != NULL)
            {
                (*dir_enumerator)(depth, di, terminate);
            }
            else
            {
                delete di;
                di = NULL;
            }
            bSearch = !terminate;
        }

        return true;
    }

    bool FileDevice_PC_System::copyDir(const xstring &szDirPath, const xstring &szToDirPath, bool boOverwrite) const
    {
        if (boOverwrite)
        {
            if (::CreateDirectoryA(szToDirPath, NULL))
            {
                ::RemoveDirectory(LPCTSTR(szToDirPath));
            }
            else
            {
                deleteDir(szToDirPath);
            }
        }
        else
        {
            if (::CreateDirectoryA(szToDirPath, NULL))
            {
                ::RemoveDirectory(LPCTSTR(szToDirPath));
            }
            else
                return false;
        }
        enumerate_delegate_files_copy_dir files_copy_enum;
        enumerate_delegate_dirs_copy_dir  dirs_copy_enum;
        enumerateCopyDir(szDirPath, true, &files_copy_enum, &dirs_copy_enum, 0);

        const xdirinfo *dirInfo = NULL;
        while (dirs_copy_enum.dirStack.pop(dirInfo))
        {
            xdirpath copyDirPath_To;
            changeDirPath(szDirPath, szToDirPath, dirInfo, copyDirPath_To);
            // nDirinfo_From --------------------->   copyDirPath_To       ( copy dir)
            delete dirInfo;
            dirInfo = NULL;
            if (!::CreateDirectoryA(copyDirPath_To.c_str_device(), NULL))
                return false;
        }

        const xfileinfo *fileInfo = NULL;
        while (files_copy_enum.fileStack.pop(fileInfo))
        {
            xfilepath copyFilePath_To;
            changeFilePath(szDirPath, szToDirPath, fileInfo, copyFilePath_To);
            // nFileinfo_From --------------------->  copyFilePath_To       (copy file)
            if (!::CopyFileA(fileInfo->getFullName().c_str_device(), copyFilePath_To.c_str_device(), false))
            {
                delete fileInfo;
                fileInfo = NULL;
                return false;
            }
            delete fileInfo;
            fileInfo = NULL;
        }

        return true;
    }

    struct enumerate_delegate_files_delete_dir : public enumerate_delegate
    {
        virtual bool operator()(s32 depth, const xfileinfo *finf, const xdirinfo *dinf)
        {
            if (finf != nullptr)
            {
                DWORD dwFileAttributes = ::GetFileAttributes((LPCTSTR)inf.getFullName().c_str_device());
                if (dwFileAttributes & FILE_ATTRIBUTE_READONLY)    // change read-only file mode
                    ::SetFileAttributes(
                        (LPCTSTR)inf.getFullName().c_str_device(), dwFileAttributes & ~FILE_ATTRIBUTE_READONLY);
                ::DeleteFile((LPCTSTR)inf.getFullName().c_str_device());
            }
            return true;
        }
    };

    struct enumerate_delegate_dirs_delete_dir : public enumerate_delegate
    {
        virtual bool operator()(s32 depth, const xfileinfo *finf, const xdirinfo *dinf)
        {
            if (dinf != nullptr)
            {
                ::RemoveDirectory((LPCTSTR)dinf.getFullName().c_str_device());    // remove the empty directory
            }
            return true;
        }
    };

    bool FileDevice_PC_System::deleteDir(const xstring &szDirPath) const
    {
        enumerate_delegate_files_delete_dir files_enum;
        enumerate_delegate_dirs_delete_dir  dirs_enum;
        return enumerate(szDirPath, true, &files_enum, &dirs_enum, 0);
    }

    bool FileDevice_PC_System::setDirTime(const xstring &szDirPath, const xdatetime &creationTime,
        const xdatetime &lastAccessTime, const xdatetime &lastWriteTime) const
    {
        HANDLE handle = sOpenDir(szDirPath);
        if (handle != INVALID_HANDLE_VALUE)
        {
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

    bool FileDevice_PC_System::getDirTime(const xstring &szDirPath, xdatetime &outCreationTime,
        xdatetime &outLastAccessTime, xdatetime &outLastWriteTime) const
    {
        HANDLE handle = sOpenDir(szDirPath);
        if (handle != INVALID_HANDLE_VALUE)
        {
            FILETIME _creationTime;
            FILETIME _lastAccessTime;
            FILETIME _lastWriteTime;
            ::GetFileTime(handle, &_creationTime, &_lastAccessTime, &_lastWriteTime);
            outCreationTime = xdatetime::sFromFileTime(
                (u64)xmem_utils::makeu64(_creationTime.dwLowDateTime, _creationTime.dwHighDateTime));
            outLastAccessTime = xdatetime::sFromFileTime(
                (u64)xmem_utils::makeu64(_lastAccessTime.dwLowDateTime, _lastAccessTime.dwHighDateTime));
            outLastWriteTime = xdatetime::sFromFileTime(
                (u64)xmem_utils::makeu64(_lastWriteTime.dwLowDateTime, _lastWriteTime.dwHighDateTime));
            sCloseDir(handle);
            return true;
        }
        return false;
    }

    bool FileDevice_PC_System::setDirAttr(const xstring &szDirPath, const xattributes &attr) const
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

    bool FileDevice_PC_System::getDirAttr(const xstring &szDirPath, xattributes &attr) const
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

    /*
    We should have two very long utf16::slice that can be used to use for the dirinfo and fileinfo objects when
    calling the enumerate_delegate. Also when we recurse into a subdirectory we only have to append the current
    directory name. When we 'return' we can pop the appended directory etc...
    */
    bool FileDevice_PC_System::enumerate(const xstring &szDirPath, enumerate_delegate *enumerator, s32 depth) const
    {
        HANDLE hFind;    // file handle

        WIN32_FIND_DATA FindFileData;

        char     DirPathBuffer[MAX_PATH + 2];
        char     FileNameBuffer[MAX_PATH + 2];
        xcstring DirPath(DirPathBuffer, sizeof(DirPathBuffer), szDirPath);
        DirPath += "\\*";

        xcstring FileName(FileNameBuffer, sizeof(FileNameBuffer), szDirPath);    // searching all files
        // FileName += "\\";

        hFind = ::FindFirstFile(DirPath.c_str(), &FindFileData);    // find the first file
        if (hFind == INVALID_HANDLE_VALUE)
            return false;

        DirPath = FileName;

        bool bSearch = true;
        while (bSearch)
        {    // until we find an entry
            if (::FindNextFile(hFind, &FindFileData))
            {
                if (sIsDots(FindFileData.cFileName))
                    continue;

                FileName += FindFileData.cFileName;

                if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                {
                    // We have found a directory, recurse
                    if (!enumerate(
                            FileName.c_str(), boSearchSubDirectories, file_enumerator, dir_enumerator, depth + 1))
                    {
                        ::FindClose(hFind);
                        return false;    // directory couldn't be enumerated
                    }

                    FileName = DirPath;
                }
                else
                {
                    xfileinfo fi(FileName.c_str());
                    if (file_enumerator != NULL)
                    {
                        bool terminate;
                        (*file_enumerator)(depth, fi, terminate);
                        bSearch = !terminate;
                    }
                    FileName = DirPath;
                }
            }
            else
            {
                if (::GetLastError() == ERROR_NO_MORE_FILES)
                {
                    // No more files here
                    bSearch = false;
                }
                else
                {
                    // Some error occurred, close the handle and return FALSE
                    ::FindClose(hFind);
                    return false;
                }
            }
        }
        ::FindClose(hFind);    // Closing file handle

        if (dir_enumerator != NULL)
        {
            xdirinfo di(FileName.c_str());
            bool     terminate;
            (*dir_enumerator)(depth, di, terminate);
            bSearch = !terminate;
        }

        return true;
    }

    bool FileDevice_PC_System::seek(u32 nFileHandle, ESeekMode mode, u64 pos, u64 &newPos) const
    {
        s32 hardwareMode = 0;
        switch (mode)
        {
            case __SEEK_ORIGIN:
                hardwareMode = FILE_BEGIN;
                break;
            case __SEEK_CURRENT:
                hardwareMode = FILE_CURRENT;
                break;
            case __SEEK_END:
                hardwareMode = FILE_END;
                break;
            default:
                ASSERT(0);
                break;
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
    bool FileDevice_PC_System::seekOrigin(u32 nFileHandle, u64 pos, u64 &newPos) const
    {
        return seek(nFileHandle, __SEEK_ORIGIN, pos, newPos);
    }

    bool FileDevice_PC_System::seekCurrent(u32 nFileHandle, u64 pos, u64 &newPos) const
    {
        return seek(nFileHandle, __SEEK_CURRENT, pos, newPos);
    }

    bool FileDevice_PC_System::seekEnd(u32 nFileHandle, u64 pos, u64 &newPos) const
    {
        return seek(nFileHandle, __SEEK_END, pos, newPos);
    }
};

#endif    // TARGET_PC
