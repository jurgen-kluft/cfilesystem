#ifndef __X_FILESYSTEM_DIR_INFO_H__
#define __X_FILESYSTEM_DIR_INFO_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

#include "xbase/x_debug.h"

#include "xfilesystem/x_dirpath.h"
#include "xfilesystem/x_attributes.h"
#include "xfilesystem/x_enumerator.h"

namespace xcore
{
    class xfiletimes;
    class xfileinfo;
    class xdirpath;

    class xdirinfo
    {
    protected:
        friend class xfilesys;

        xfilesys* mParent;
        xdirpath  mPath;

    public:
        xdirinfo();
        xdirinfo(const xdirinfo& dirinfo);
        explicit xdirinfo(const xdirpath& dir);

        bool isRoot() const;
        bool isRooted() const;

        bool exists() const;
        bool create();
        bool remove();
        void refresh();

        void copy(const xdirpath& toDirectory, xbool overwrite = xFALSE);
        void move(const xdirpath& toDirectory);

        void enumerate(enumerate_delegate& enumerator);

        xdirpath const& getDirpath() const;
        bool            getRoot(xdirinfo& outRootDirInfo) const;
        bool            getParent(xdirinfo& outParentDirInfo) const;
        bool            getTimes(xfiletimes& times) const;
        bool            setTimes(xfiletimes times);
        bool            getAttrs(xfileattrs& attrs) const;
        bool            setAttrs(xfileattrs attrs);

        xdirinfo& operator=(const xdirinfo&);
        xdirinfo& operator=(const xdirpath&);
        bool      operator==(const xdirpath&) const;
        bool      operator!=(const xdirpath&) const;
        bool      operator==(const xdirinfo&) const;
        bool      operator!=(const xdirinfo&) const;

        ///< Static functions
        static bool sIsValid(const xdirpath& directory);
        static bool sExists(const xdirpath& directory);
        static bool sCreate(const xdirpath& directory);
        static bool sDelete(const xdirpath& directory);
        static void sEnumerate(const xdirpath& directory, enumerate_delegate& dir_enumerator);
        static bool sSetTime(const xdirpath& directory, const xfiletimes& ftimes);
        static bool sGetTime(const xdirpath& directory, xfiletimes& ftimes);
        static bool sSetAttrs(const xdirpath& directory, const xfileattrs& fattrs);
        static bool sGetAttrs(const xdirpath& directory, xfileattrs& fattrs);
        static bool sCopy(const xdirpath& sourceDirectory, const xdirpath& destDirectory, xbool overwrite = xTRUE);
        static bool sMove(const xdirpath& sourceDirectory, const xdirpath& destDirectory, xbool overwrite = xTRUE);
    };
};

#endif