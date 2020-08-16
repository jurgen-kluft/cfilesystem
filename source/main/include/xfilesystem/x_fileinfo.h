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
    class xstream;
    class xfilestream;
    class xfiletimes;
    class xdirinfo;
    class xfilesys;

    class xfileinfo
    {
        friend class xdirpath;
        friend class xfilepath;
        friend class xdirinfo;
        friend class xfileinfo;
        friend class xfilesys;

        bool       mFileExists;
        xfiletimes mFileTimes;
        xfileattrs mFileAttributes;

        xfilesys* mParent;
        xfilepath mPath;

    public:
        xfileinfo();
        xfileinfo(const xfileinfo& fileinfo);
        xfileinfo(const xfilepath& filename);

        u64  getLength() const;
        void setLength(u64 length);

        bool isValid() const;
        bool isRooted() const;

        bool getAttrs(xfileattrs& fattrs) const;
        bool getTimes(xfiletimes& ftimes) const;
        bool setAttrs(xfileattrs fattrs);
        bool setTimes(xfiletimes ftimes);

        bool create(xstream*&) const;
        bool exists() const;
        bool remove();
        void refresh();
        bool create();

        bool open(xstream*& outFileStream);
        bool openRead(xstream*& outFileStream);
        bool openWrite(xstream*& outFileStream);
        u64  readAllBytes(xbyte* buffer, u64 count);
        u64  writeAllBytes(const xbyte* buffer, u64 count);

        bool getParent(xdirpath& parentpath) const;
        bool getRoot(xdirpath& rootpath) const;
        void getDirpath(xdirpath& dirpath) const;
        void getFilename(xfilepath& filename) const;
        void getFilenameWithoutExtension(xfilepath& filename) const;
        void getExtension(xfilepath& extension) const;

		xdirpath getParent() const;
        xdirpath getRoot() const;
        xdirpath getDirpath() const;
        xfilepath getFilename() const;
        xfilepath getFilenameWithoutExtension() const;
        xfilepath getExtension() const;

        void             getFilepath(xfilepath&) const;
        xfilepath const& getFilepath() const;

        void up();
        void down(xdirpath const& dir);

        bool copy_to(const xfilepath& toFilename, bool overwrite);
        bool move_to(const xfilepath& toFilename, bool overwrite);

        xfileinfo& operator=(const xfileinfo&);
        xfileinfo& operator=(const xfilepath&);
        bool       operator==(const xfileinfo&) const;
        bool       operator!=(const xfileinfo&) const;

        static bool sExists(const xfilepath& filepath);
        static bool sCreate(const xfilepath& filepath, xstream*& outFileStream);
        static bool sDelete(const xfilepath& filepath);

        static bool sGetFileAttributes(const xfilepath& filepath, xfileattrs& outAttr);

        static u64  sGetLength(const xfilepath& filename);
        static void sSetLength(const xfilepath& filename, u64 length);

        static bool sIsArchive(const xfilepath& filename);
        static bool sIsReadOnly(const xfilepath& filename);
        static bool sIsHidden(const xfilepath& filename);
        static bool sIsSystem(const xfilepath& filename);

        static bool sOpen(const xfilepath& filename, xstream*& outFileStream);
        static bool sOpenRead(const xfilepath& filename, xstream*& outFileStream);
        static bool sOpenWrite(const xfilepath& filename, xstream*& outFileStream);

        static u64 sReadAllBytes(const xfilepath& filename, xbyte* buffer, u64 count);
        static u64 sWriteAllBytes(const xfilepath& filename, const xbyte* buffer, u64 count);

        static bool sSetTime(const xfilepath& filename, const xfiletimes& ftimes);
        static bool sGetTime(const xfilepath& filename, xfiletimes& ftimes);
        static bool sSetAttrs(const xfilepath& filename, const xfileattrs& fattrs);
        static bool sGetAttrs(const xfilepath& filename, xfileattrs& fattrs);

        static bool sCopy(const xfilepath& sourceFilename, const xfilepath& destFilename, bool overwrite = true);
        static bool sMove(const xfilepath& sourceFilename, const xfilepath& destFilename, bool overwrite = true);
    };
}; // namespace xcore

#endif
