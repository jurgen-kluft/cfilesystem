#ifndef __X_FILESYSTEM_DIRPATH_H__
#define __X_FILESYSTEM_DIRPATH_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

#include "xbase/x_debug.h"
#include "xbase/x_runes.h"

#include "xfilesystem/x_enumerator.h"
#include "xfilesystem/private/x_path.h"
#include "xfilesystem/private/x_filesystem.h"

namespace xcore
{
    class filepath_t;
    class filesys_t;

    //==============================================================================
    // dirpath_t:
    //		- Relative:		"FolderA\FolderB\"
    //		- Absolute:		"Device:\FolderA\FolderB\"
    //==============================================================================
    class dirpath_t
    {
    protected:
        pathdevice_t* m_device;
        path_t* m_path;

        friend class filepath_t;
        friend class filesys_t;
        friend class fileinfo_t;
        friend class dirinfo_t;

    public:
        dirpath_t();
        dirpath_t(dirpath_t const& other);
        dirpath_t(pathdevice_t* device);
        ~dirpath_t();

        void clear();
        bool isEmpty() const;
        bool isRoot() const;
        bool isRooted() const;

        void makeRelativeTo(const dirpath_t& dirpath);
        void makeAbsoluteTo(const dirpath_t& dirpath);

        pathname_t* devname() const;
        pathname_t* getname() const;

        dirpath_t root() const;

        void split(s32 pivot, dirpath_t left, dirpath_t right) const;
        void truncate(dirpath_t& dirpath, pathname_t*& folder) const;
        void truncate(pathname_t*& folder, dirpath_t& dirpath) const;
        void combine(pathname_t* folder, dirpath_t const& dirpath);
        void combine(dirpath_t const& dirpath, pathname_t* folder);

        void to_string(runes_t& str) const;
    };

}; // namespace xcore

#endif // __X_FILESYSTEM_DIRPATH_H__