#ifndef __X_FILESYSTEM_XPATH_H__
#define __X_FILESYSTEM_XPATH_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

#include "xbase/x_debug.h"
#include "xbase/x_runes.h"

namespace xcore
{
    class xfilesystem;
    class xfiledevice;
	class xfilesys;

    //==============================================================================
    // xdirpath:
    //		- Relative:		FolderA\FolderB\ 
	//		- Absolute:		Device:\FolderA\FolderB\ 
    //
    // Root                     = Device:\ 
    // Parent                   = Device:\FolderA\ 
    // Dir                      = \FolderA\FolderB\
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

    class xpath
    {
	public:
		static utf32::crunes s_device_separator;

        utf32::alloc* m_alloc;
        utf32::runes  m_path;

        xpath();
        xpath(utf32::alloc* allocator);
        xpath(utf32::alloc* allocator, const utf32::crunes& path);
        xpath(const xpath& path);
        xpath(const xpath& lhspath, const xpath& rhspath);
		~xpath();

        xpath resolve(xfilesys* fs, xfiledevice*& outdevice) const;

		static xdirinfo dirinfo(ascii::pcrune path, utf32::alloc* allocator);
		static xdirpath dirpath(ascii::pcrune path, utf32::alloc* allocator);
		static xfileinfo fileinfo(ascii::pcrune path, utf32::alloc* allocator);
		static xfilepath filepath(ascii::pcrune path, utf32::alloc* allocator);

		static s32 compare(xdirinfo const& d, ascii::pcrune path);
		static s32 compare(xdirpath const& d, ascii::pcrune path);

        void set_filepath(utf32::runes& runes, utf32::alloc* allocator);
        void set_dirpath(utf32::runes& runes, utf32::alloc* allocator);
        void combine(const xpath& dirpath, const xpath& filepath);

        void copy_dirpath(utf32::runes& runes);

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

        bool up();
        bool down(utf32::runes const& runes);

        void getFilename(xpath& out_filename) const;
        void getFilenameWithoutExtension(xpath& out_filename_no_ext) const;
        void getExtension(xpath& out_ext) const;

		xpath& operator=(const utf32::runes&);
        xpath& operator=(const xpath&);

		xpath& operator+=(const utf32::runes&);

        bool operator==(const xpath&) const;
        bool operator!=(const xpath&) const;

        void append_utf16(utf16::crunes const&);

        // These should be used carefully, since they are modifying the
        // utf32::runes array from holding utf32::rune characters to
        // utf16::rune characters. You should also use these 2 functions
        // as a begin/end pair. Do not leave this structure hanging in
        // utf16 format.
        void to_utf16(utf16::runes& runes) const;
        void to_utf32(utf16::runes& runes) const;
    };

};

#endif // __X_FILESYSTEM_XPATH_H__