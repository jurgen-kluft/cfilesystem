#ifndef __X_FILESYSTEM_DIRPATH_H__
#define __X_FILESYSTEM_DIRPATH_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

#include "xbase/x_debug.h"

#include "xfilesystem/x_enumerator.h"
#include "xfilesystem/private/x_path.h"
#include "xfilesystem/private/x_filesystem.h"

namespace xcore
{
    class xfilepath;
    class xfilesys;

    //==============================================================================
    // xdirpath:
    //		- Relative:		FolderA\FolderB\ 
	//		- Absolute:		Device:\FolderA\FolderB\ 
	//==============================================================================
    class xdirpath
    {
    protected:
        friend class xfilepath;
        friend class xfileinfo;
        friend class xdirinfo;
        friend class xdirpathz;
        friend class xdirpathc;

        xfilesys* mParent;
        xpath     mPath;

        xdirpath(xfilesys* fs, xpath& path);

        xpath const path() const;

    public:
        xdirpath();
        xdirpath(const xdirpath& dir);
        xdirpath(const xdirpath& rootdir, const xdirpath& subdir);
        ~xdirpath();

        void clear();

        bool isEmpty() const;
        bool isRoot() const;
        bool isRooted() const;
        bool isSubDirOf(const xdirpath&) const;

        void relative(xdirpath& outRelative) const;
        void makeRelative();
        void makeRelativeTo(const xdirpath& parent);
        void makeRelativeTo(const xdirpath& parent, xdirpath& sub) const;

        s32  getLevels() const;
        bool getLevel(s32 level, xdirpath& name) const;
        s32  getLevelOf(const xdirpath& name) const;
        bool split(s32 level, xdirpath& parent, xdirpath& subDir) const;

        bool getName(xfilepath& outName) const;
        bool hasName(const xfilepath& inName) const;
        bool getParent(xdirpath& outParentDirPath) const;
        void setRoot(const xdirpath& device);
        bool getRoot(xdirpath& outDevice) const;

        xdirpath& operator=(const xdirpath&);
        xdirpath& operator=(const xfilepath&);
        xdirpath& operator+=(const xdirpath&);
        xfilepath operator+=(const xfilepath&);
        bool      operator==(const xdirpath& rhs) const;
        bool      operator!=(const xdirpath& rhs) const;
    };

    xdirpath operator+(const xdirpath&, const xdirpath&);

}; // namespace xcore

#endif // __X_FILESYSTEM_DIRPATH_H__