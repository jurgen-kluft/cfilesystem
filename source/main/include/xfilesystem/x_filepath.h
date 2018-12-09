#ifndef __X_FILESYSTEM_FILEPATH_H__
#define __X_FILESYSTEM_FILEPATH_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase/x_debug.h"
#include "xbase/x_runes.h"

//==============================================================================
namespace xcore
{
    class xpath;
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
        friend class xdirpath;
        friend class _xfilesystem_;

        xfilepath(xfilesystem* parent, utf16::alloc* allocator, utf16::runes& str);

    public:
        xfilesystem*  mFileSystem;
        utf16::alloc* mAlloc;
        utf16::runes  mRunes;

    public:
        xfilepath();
        xfilepath(const xfilepath& filepath);
        explicit xfilepath(const xdirpath& dir, const xfilepath& filename);
        ~xfilepath();

        void clear();

        bool isEmpty() const;
        bool isRooted() const;

        void makeRelative();
        void makeRelative(const xdirpath&);
        void makeRelative(xdirpath&, xfilepath&) const;

        bool getRoot(xdirpath&) const;
        bool getDirname(xdirpath&) const;
        void getFilename(xfilepath&) const;
        void getFilenameWithoutExtension(xfilepath&) const;
        void getExtension(xfilepath&) const;

        xfilepath& operator=(const xfilepath&);
        bool       operator==(const xfilepath&) const;
        bool       operator!=(const xfilepath&) const;
    };

    inline xfilepath operator+(const xdirpath& dir, const xfilepath& filename) { return xfilepath(dir, filename); }

}; // namespace xcore

#endif