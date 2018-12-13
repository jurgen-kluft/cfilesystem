#ifndef __X_FILESYSTEM_DIR_INFO_H__
#define __X_FILESYSTEM_DIR_INFO_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase/x_debug.h"

#include "xfilesystem/x_dirpath.h"
#include "xfilesystem/x_attributes.h"
#include "xfilesystem/x_enumerator.h"

//==============================================================================
namespace xcore
{
    // Forward declares
    class xdatetime;
    class xfileinfo;
    class xdirpath;

    class xdirinfo
    {
    protected:
        friend class _xfilesystem_;
        friend class xdirpath;
        friend class xfileinfo;
        friend class xfilepath;

    public:
        _xfilesystem_* mParent;
        xdirpath     mDirPath;

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

        bool getDirpath(xdirpath& outDirpath) const;
        bool getRoot(xdirinfo& outRootDirInfo) const;
        bool getParent(xdirinfo& outParentDirInfo) const;

        bool getTimes(xfiletimes& times) const;
        bool setTimes(xfiletimes times);

        bool getAttrs(xfileattrs& attrs) const;
        bool setAttrs(xfileattrs attrs);

        xdirinfo& operator=(const xdirinfo&);
        xdirinfo& operator=(const xdirpath&);

        bool operator==(const xdirpath&) const;
        bool operator!=(const xdirpath&) const;

        bool operator==(const xdirinfo&) const;
        bool operator!=(const xdirinfo&) const;

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

        ///< Copies an existing directory to a new directory. Overwriting a directory of the same name is allowed.
        static bool sCopy(const xdirpath& sourceDirectory, const xdirpath& destDirectory, xbool overwrite = xFALSE);
        ///< Moves a specified directory to a new location, providing the option to specify a new directory name.
        static bool sMove(const xdirpath& sourceDirectory, const xdirpath& destDirectory);
    };
}; // namespace xcore

#endif