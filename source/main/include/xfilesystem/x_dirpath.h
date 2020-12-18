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
        friend class path_t;
        friend class filepath_t;
        friend class fileinfo_t;
        friend class dirinfo_t;
        friend class xdirpathz;
        friend class xdirpathc;
        friend class filesys_t;
        friend class filedevice_pc_t;

        filesys_t* mParent;
        path_t     mPath;

        dirpath_t(filesys_t* fs, path_t& path);

    public:
        dirpath_t();
        dirpath_t(const dirpath_t& dir);
        dirpath_t(const dirpath_t& rootdir, const dirpath_t& subdir);
        ~dirpath_t();

        void clear();

        bool isEmpty() const;
        bool isRoot() const;
        bool isRooted() const;
        bool isSubDirOf(const dirpath_t&) const;

        void relative(dirpath_t& outRelative) const;
        void makeRelative();
        void makeRelativeTo(const dirpath_t& parent);
        void makeRelativeTo(const dirpath_t& parent, dirpath_t& sub) const;

        s32  getLevels() const;
        bool getLevel(s32 level, dirpath_t& name) const;
        s32  getLevelOf(const dirpath_t& name) const;
        bool split(s32 level, dirpath_t& parent, dirpath_t& subDir) const;

        bool getName(dirpath_t& outName) const;
        bool hasName(const dirpath_t& inName) const;
        bool getParent(dirpath_t& outParentDirPath) const;
        void setRoot(const dirpath_t& device);
        bool getRoot(dirpath_t& outDevice) const;

        dirpath_t& operator=(const dirpath_t&);
        dirpath_t& operator=(const filepath_t&);
        dirpath_t& operator+=(const dirpath_t&);
        filepath_t operator+=(const filepath_t&);
        bool      operator==(const dirpath_t& rhs) const;
        bool      operator!=(const dirpath_t& rhs) const;
    };

    dirpath_t operator+(const dirpath_t&, const dirpath_t&);

}; // namespace xcore

#endif // __X_FILESYSTEM_DIRPATH_H__