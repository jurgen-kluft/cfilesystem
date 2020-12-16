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
    //		- Relative:		"FolderA\FolderB\"
    //		- Absolute:		"Device:\FolderA\FolderB\"
    //
    // Root                     = "Device:\"
    // Parent                   = "Device:\FolderA\"
    // Dir                      = "\FolderA\FolderB\"
    //==============================================================================

    //==============================================================================
    // xfilepath:
    //		- Relative:		"FolderA\FolderB\Filename.ext"
    //		- Absolute:		"Device:\FolderA\FolderB\Filename.ext"
    //
    // Root                     = "Device:\"
    // Parent                   = "Device:\FolderA\"
    // Dir                      = "\FolderA\FolderB\Filename.ext"
    // Filename                 = "Filename.ext"
    // FilenameWithoutExtension = "Filename"
    //==============================================================================
    class xfilepath;
    class xdirpath;

    class xpath
    {
    public:
        static crunes_t s_device_separator;

        runes_alloc_t* m_alloc;
        runes_t  m_path;

        xpath();
        xpath(runes_alloc_t* allocator);
        xpath(runes_alloc_t* allocator, const crunes_t& path);
        xpath(runes_alloc_t* allocator, const crunes_t& path);
        xpath(const xpath& path);
        xpath(const xpath& lhspath, const xpath& rhspath);
        ~xpath();

        xpath resolve(xfilesys* fs, xfiledevice*& outdevice) const;

        void set_filepath(runes_t& runes, runes_alloc_t* allocator);
        void set_dirpath(runes_t& runes, runes_alloc_t* allocator);
        void set_filepath(runes_t& runes, runes_alloc_t* allocator);
        void set_dirpath(runes_t& runes, runes_alloc_t* allocator);

        void combine(const xpath& dirpath, const xpath& filepath);
        void copy_dirpath(runes_t& runes);
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
        bool down(xpath const& runes);

        void getFilename(xpath& out_filename) const;
        void getFilenameWithoutExtension(xpath& out_filename_no_ext) const;
        void getExtension(xpath& out_ext) const;

        xpath& operator=(const runes_t&);
        xpath& operator=(const xpath&);
        xpath& operator+=(const runes_t&);

        bool operator==(const xpath&) const;
        bool operator!=(const xpath&) const;

        // These should be used carefully, since they are modifying the
        // runes_t array from holding utf32::rune characters to
        // utf16::rune characters. You should also use these 2 functions
        // as a begin/end pair. Do not leave this structure hanging in
        // utf16 format.
        static void append_utf16(xpath const& p, crunes_t const&);
        static void view_utf16(xpath const& fp, crunes_t& runes);
        static void release_utf16(xpath const& fp, crunes_t& runes);

        static void append_utf16(xfilepath const& fp, crunes_t const&);
        static void view_utf16(xfilepath const& fp, crunes_t& runes);
        static void release_utf16(xfilepath const& fp, crunes_t& runes);

        static void append_utf16(xdirpath const& fp, crunes_t const&);
        static void view_utf16(xdirpath const& fp, crunes_t& runes);
        static void release_utf16(xdirpath const& fp, crunes_t& runes);
    };

}; // namespace xcore

#endif // __X_FILESYSTEM_XPATH_H__