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
    class filetimes_t;
    class fileinfo_t;
    class dirpath_t;

    class dirinfo_t
    {
    protected:
        friend class filesys_t;

        filesystem_t::context_t* m_context;
        dirpath_t  m_path;

    public:
        dirinfo_t();
        dirinfo_t(const dirinfo_t& dirinfo);
        ~dirinfo_t();

        explicit dirinfo_t(const dirpath_t& dir);

        bool isRoot() const;
        bool isRooted() const;

        bool exists() const;
        bool create();
        bool remove();
        void refresh();

        void copy(const dirpath_t& toDirectory, bool overwrite = xFALSE);
        void move(const dirpath_t& toDirectory);

        void enumerate(enumerate_delegate_t& enumerator);

        dirpath_t const& getDirpath() const;
        bool            getRoot(dirinfo_t& outRootDirInfo) const;
        bool            getParent(dirinfo_t& outParentDirInfo) const;
        bool            getTimes(filetimes_t& times) const;
        bool            setTimes(filetimes_t times);
        bool            getAttrs(fileattrs_t& attrs) const;
        bool            setAttrs(fileattrs_t attrs);

        dirinfo_t& operator=(const dirinfo_t&);
        dirinfo_t& operator=(const dirpath_t&);
        bool      operator==(const dirpath_t&) const;
        bool      operator!=(const dirpath_t&) const;
        bool      operator==(const dirinfo_t&) const;
        bool      operator!=(const dirinfo_t&) const;

        ///< Static functions
        static bool sIsValid(const dirpath_t& directory);
        static bool sExists(const dirpath_t& directory);
        static bool sCreate(const dirpath_t& directory);
        static bool sDelete(const dirpath_t& directory);
        static void sEnumerate(const dirpath_t& directory, enumerate_delegate_t& dir_enumerator);
        static bool sSetTime(const dirpath_t& directory, const filetimes_t& ftimes);
        static bool sGetTime(const dirpath_t& directory, filetimes_t& ftimes);
        static bool sSetAttrs(const dirpath_t& directory, const fileattrs_t& fattrs);
        static bool sGetAttrs(const dirpath_t& directory, fileattrs_t& fattrs);
        static bool sCopy(const dirpath_t& sourceDirectory, const dirpath_t& destDirectory, bool overwrite = xTRUE);
        static bool sMove(const dirpath_t& sourceDirectory, const dirpath_t& destDirectory, bool overwrite = xTRUE);
    };
};

#endif