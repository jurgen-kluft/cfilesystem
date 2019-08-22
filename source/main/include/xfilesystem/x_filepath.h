#ifndef __X_FILESYSTEM_FILEPATH_H__
#define __X_FILESYSTEM_FILEPATH_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

#include "xbase/x_debug.h"
#include "xbase/x_runes.h"
#include "xfilesystem/private/x_path.h"

namespace xcore
{
    class xdirpath;
    class xfilesystem;

    //==============================================================================
    // xfilepath:
    //		- Relative:		Folder\Filename.Extension
    //		- Absolute:		Device:\Folder\Folder\Filename.Extension
    //
    // What about making the following classes:
    // - xfilext; for file extensions, ".JPG"
    // - xfilename; for file names, "MyImage"
    // - xroot; for device part, "Device:\"
    //
    // "C:\" + "Windows\" + "DBGHELP" + ".DLL"
    // xroot + xdirpath + xfilename + xfilext
    //
    //==============================================================================
    class xfilepath
    {
    protected:
        friend class xfilesys;

        xfilepath(xfilesys* parent, xpath& path);
        xfilesys* mParent;
        xpath     mPath;

    public:
        xfilepath();
        xfilepath(const xfilepath& filepath);
        explicit xfilepath(const xdirpath& dir, const xfilepath& filename);
        ~xfilepath();

        void clear();

        bool isEmpty() const;
        bool isRooted() const;

        void makeRelative();
        void makeRelativeTo(const xdirpath& dirpath);

        bool getRoot(xdirpath&) const;
        bool getDirname(xdirpath&) const;
        void getFilename(xfilepath&) const;
        void getFilenameWithoutExtension(xfilepath&) const;
        void getExtension(xfilepath&) const;

        void up();
        void down(xdirpath const&);

        xpath const path() const;

        xfilepath& operator=(const xfilepath&);
        bool       operator==(const xfilepath&) const;
        bool       operator!=(const xfilepath&) const;
    };

    inline xfilepath operator+(const xdirpath& dir, const xfilepath& filename) { return xfilepath(dir, filename); }

}; // namespace xcore

#endif