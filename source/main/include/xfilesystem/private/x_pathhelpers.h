#ifndef __X_FILESYSTEM_XPATH_H__
#define __X_FILESYSTEM_XPATH_H__
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
    class xfilesystem;
    class xfiledevice;

    //==============================================================================
    // xdirpath:
    //		- Relative:		FolderA\FolderB\ 
	//		- Absolute:		Device:\FolderA\FolderB\ 
	//==============================================================================

    //==============================================================================
    // xfilepath:
    //		- Relative:		FolderA\FolderB\Filename.ext
    //		- Absolute:		Device:\FolderA\FolderB\Filename.ext
    //
    // Root                     = Device:\ 
    // Parent                   = Device:\FolderA\ 
    // Dir                      = \FolderA\FolderB\Filename.ext
    // Filename                 = Filename.ext
    // FilenameWithoutExtension = Filename
    //==============================================================================

    struct xpath
    {
        utf16::alloc* m_alloc;
        utf16::runes  m_path;

        xpath(utf16::alloc* allocator);

        xpath resolve(xfilesystem* filesystem, xfiledevice*& outdevice) const;

        void clear();
        void erase();

        bool isEmpty() const;
        bool isRoot() const;
        bool isRooted() const;
        bool isSubDirOf(const xpath& dirpath) const;

        void set_filepath(utf16::runes& runes, utf16::alloc* allocator);
        void set_dirpath(utf16::runes& runes, utf16::alloc* allocator);
        void set_combine(xpath const& dirpath, xpath const& filepath);

        s32  getLevels() const;
        bool getLevel(s32 level, xpath& dirpath) const;
        s32  getLevelOf(const xpath& dirname) const;
        bool split(s32 level, xpath& parent_dirpath, xpath& relative_filepath) const;

        void makeRelative();
        void makeRelativeTo(const xpath& parent_dirpath);

        void setRootDir(const xpath& in_root_dirpath);
        bool getRootDir(xpath& out_root_dirpath) const;
        bool getParentDir(xpath& out_parent_dirpath) const;
        bool getDir(xpath& out_dirpath) const;

        void getFilename(xpath& out_filename) const;
        void getFilenameWithoutExtension(xpath& out_filename_no_ext) const;
        void getExtension(xpath& out_ext) const;

        xpath& operator=(const xpath&) const;

        bool operator==(const xpath&) const;
        bool operator!=(const xpath&) const;
    };

}; // namespace xcore

#endif // __X_FILESYSTEM_XPATH_H__