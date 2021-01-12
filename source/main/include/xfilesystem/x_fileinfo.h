#ifndef __X_FILESYSTEM_FILE_INFO_H__
#define __X_FILESYSTEM_FILE_INFO_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

#include "xbase/x_debug.h"

#include "xfilesystem/x_filepath.h"
#include "xfilesystem/x_attributes.h"

namespace xcore
{
    // Forward declares
    class stream_t;
    class filestream_t;
    class filetimes_t;
    class dirinfo_t;
    class filesys_t;

    class fileinfo_t
    {
        friend class dirpath_t;
        friend class filepath_t;
        friend class dirinfo_t;
        friend class fileinfo_t;
        friend class filesys_t;

        bool        mFileExists;
        filetimes_t mFileTimes;
        fileattrs_t mFileAttributes;
        filesys_t* mParent;
        filepath_t mPath;

    public:
        fileinfo_t();
        fileinfo_t(const fileinfo_t& fileinfo);
        fileinfo_t(const filepath_t& filename);

        u64  getLength() const;
        void setLength(u64 length);

        bool isValid() const;
        bool isRooted() const;

        bool getAttrs(fileattrs_t& fattrs) const;
        bool getTimes(filetimes_t& ftimes) const;
        bool setAttrs(fileattrs_t fattrs);
        bool setTimes(filetimes_t ftimes);

        bool create(stream_t*&) const;
        bool exists() const;
        bool remove();
        void refresh();
        bool create();

        bool open(stream_t*& outFileStream);
        bool openRead(stream_t*& outFileStream);
        bool openWrite(stream_t*& outFileStream);
        u64  readAllBytes(xbyte* buffer, u64 count);
        u64  writeAllBytes(const xbyte* buffer, u64 count);

        bool getParent(dirpath_t& parentpath) const;
        bool getRoot(dirpath_t& rootpath) const;
        void getDirpath(dirpath_t& dirpath) const;
        void getFilename(filepath_t& filename) const;
        void getFilenameWithoutExtension(filepath_t& filename) const;
        void getExtension(filepath_t& extension) const;

        dirpath_t  getParent() const;
        dirpath_t  getRoot() const;
        dirpath_t  getDirpath() const;
        filepath_t getFilename() const;
        filepath_t getFilenameWithoutExtension() const;
        filepath_t getExtension() const;

        void              getFilepath(filepath_t&) const;
        filepath_t const& getFilepath() const;

        void up();
        void down(dirpath_t const& dir);

        bool copy_to(const filepath_t& toFilename, bool overwrite);
        bool move_to(const filepath_t& toFilename, bool overwrite);

        fileinfo_t& operator=(const fileinfo_t&);
        fileinfo_t& operator=(const filepath_t&);
        bool        operator==(const fileinfo_t&) const;
        bool        operator!=(const fileinfo_t&) const;

        static bool sExists(const filepath_t& filepath);
        static bool sCreate(const filepath_t& filepath, stream_t*& outFileStream);
        static bool sDelete(const filepath_t& filepath);

        static bool sGetFileAttributes(const filepath_t& filepath, fileattrs_t& outAttr);

        static u64  sGetLength(const filepath_t& filename);
        static void sSetLength(const filepath_t& filename, u64 length);

        static bool sIsArchive(const filepath_t& filename);
        static bool sIsReadOnly(const filepath_t& filename);
        static bool sIsHidden(const filepath_t& filename);
        static bool sIsSystem(const filepath_t& filename);

        static bool sOpen(const filepath_t& filename, stream_t*& outFileStream);
        static bool sOpenRead(const filepath_t& filename, stream_t*& outFileStream);
        static bool sOpenWrite(const filepath_t& filename, stream_t*& outFileStream);

        static u64 sReadAllBytes(const filepath_t& filename, xbyte* buffer, u64 count);
        static u64 sWriteAllBytes(const filepath_t& filename, const xbyte* buffer, u64 count);

        static bool sSetTime(const filepath_t& filename, const filetimes_t& ftimes);
        static bool sGetTime(const filepath_t& filename, filetimes_t& ftimes);
        static bool sSetAttrs(const filepath_t& filename, const fileattrs_t& fattrs);
        static bool sGetAttrs(const filepath_t& filename, fileattrs_t& fattrs);

        static bool sCopy(const filepath_t& sourceFilename, const filepath_t& destFilename, bool overwrite = true);
        static bool sMove(const filepath_t& sourceFilename, const filepath_t& destFilename, bool overwrite = true);
    };
}; // namespace xcore

#endif
