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
        dirpath_t(pathdevice_t* device, path_t* path);
        ~dirpath_t();

        void clear();
        bool isEmpty() const;
        bool isRoot() const;
        bool isRooted() const;

        void makeRelativeTo(const dirpath_t& dirpath);
        void makeAbsoluteTo(const dirpath_t& dirpath);

        pathname_t* devname() const; // "E:\documents\old\inventory\", -> "E"
        pathname_t* rootname() const;// "E:\documents\old\inventory\", -> "documents"
        pathname_t* basename() const;// "E:\documents\old\inventory\", -> "inventory"

        dirpath_t device() const;
        dirpath_t root() const;
        dirpath_t parent() const;

        void split(s32 pivot, dirpath_t& left, dirpath_t& right) const;
        void truncate(dirpath_t& dirpath, pathname_t*& folder) const;
        void truncate(pathname_t*& folder, dirpath_t& dirpath) const;
        void combine(pathname_t* folder, dirpath_t const& dirpath);
        void combine(dirpath_t const& dirpath, pathname_t* folder);

        void down(pathname_t* folder);
        void up();

        s32 compare(const dirpath_t& other) const;

        void to_string(runes_t& str) const;

        static void getSubDir(const dirpath_t& parentpath, const dirpath_t& path, dirpath_t& out_subpath);

    };

    extern dirpath_t operator+(const dirpath_t& dirpath, const dirpath_t& append_dirpath);


}; // namespace xcore

#endif // __X_FILESYSTEM_DIRPATH_H__