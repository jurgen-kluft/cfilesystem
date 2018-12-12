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
        typedef utf32::runes __runes;
        typedef utf32::alloc __alloc;

        __alloc* m_alloc;
        __runes  m_path;

        xpath();
        xpath(__alloc* allocator);
        xpath(const xpath& path);
        xpath(const xpath& lhspath, const xpath& rhspath);

        xpath resolve(xfilesystem* filesystem, xfiledevice*& outdevice) const;

        void set_filepath(xpath::__runes& runes, xpath::__alloc* allocator);
        void set_dirpath(xpath::__runes& runes, xpath::__alloc* allocator);
        void combine(const xpath& dirpath, const xpath& filepath);

        void clear();
        void erase();

        bool isEmpty() const;
        bool isRoot() const;
        bool isRooted() const;
        bool isSubDirOf(const xpath& dirpath) const;

        s32  getLevels() const;
        bool getLevel(s32 level, xpath& dirpath) const;
        s32  getLevelOf(const xpath& dirname) const;
        bool split(s32 level, xpath& parent_dirpath, xpath& relative_filepath) const;

        void makeRelative();
        void makeRelativeTo(const xpath& parent_dirpath);

        void setRootDir(const xpath& in_root_dirpath);
        bool getRootDir(xpath& out_root_dirpath) const;
        bool getDirname(xpath& out_dirpath) const;

        void getFilename(xpath& out_filename) const;
        void getFilenameWithoutExtension(xpath& out_filename_no_ext) const;
        void getExtension(xpath& out_ext) const;

        xpath& operator=(const xpath&);

        bool operator==(const xpath&) const;
        bool operator!=(const xpath&) const;
    };

}; // namespace xcore

#endif // __X_FILESYSTEM_XPATH_H__