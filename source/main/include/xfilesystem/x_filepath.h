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
    //==============================================================================
    class xfilepath
    {
    protected:
        friend class xfilesys;
        friend class xfileinfo;

        xfilepath(xfilesys* parent, xpath& path);
        xfilesys* mParent;
        xpath     mPath;

    public:
        xfilepath();
        xfilepath(ascii::pcrune path);
        xfilepath(const xfilepath& filepath);
        explicit xfilepath(const xdirpath& dir, const xfilepath& filename);
        ~xfilepath();

        void       clear();
        bool       isEmpty() const;
        bool       isRooted() const;
        void       makeRelative();
        void       makeRelativeTo(const xdirpath& dirpath);
		void       makeAbsoluteTo(const xdirpath& dirpath);
        bool       getRoot(xdirpath&) const;
        bool       getDirname(xdirpath&) const;
        void       getFilename(xfilepath&) const;
        void       getFilenameWithoutExtension(xfilepath&) const;
        void       getExtension(xfilepath&) const;
        void       up();
        void       down(xdirpath const&);

        xfilepath& operator=(const xfilepath&);
        bool       operator==(const xfilepath&) const;
        bool       operator!=(const xfilepath&) const;

	private:
		void	   to_utf16(utf16::runes& str) const;
		void	   view_utf16(utf16::crunes& str) const;
		void	   release_utf16(utf16::crunes& str) const;
    };

    inline xfilepath operator+(const xdirpath& dir, const xfilepath& filename) { return xfilepath(dir, filename); }

}; // namespace xcore

#endif