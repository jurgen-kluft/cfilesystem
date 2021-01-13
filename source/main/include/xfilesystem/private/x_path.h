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
    class filesystem_t;
    class filedevice_t;
    class filesys_t;

    //==============================================================================
    // dirpath_t:
    //		- Relative:		"FolderA\FolderB\"
    //		- Absolute:		"Device:\FolderA\FolderB\"
    //
    // Root                     = "Device:\"
    // Parent                   = "Device:\FolderA\"
    // Dir                      = "\FolderA\FolderB\"
    //==============================================================================

    //==============================================================================
    // filepath_t:
    //		- Relative:		"FolderA\FolderB\Filename.ext"
    //		- Absolute:		"Device:\FolderA\FolderB\Filename.ext"
    //
    // Root                     = "Device:\"
    // Parent                   = "Device:\FolderA\"
    // Dir                      = "\FolderA\FolderB\Filename.ext"
    // Filename                 = "Filename.ext"
    // FilenameWithoutExtension = "Filename"
    //==============================================================================
    class filepath_t;
    class dirpath_t;

    class path_t
    {
    public:
        static crunes_t s_device_separator;

        runes_alloc_t* m_alloc;
        runes_t m_path;

        path_t();
        path_t(runes_alloc_t* allocator);
        path_t(runes_alloc_t* allocator, const crunes_t& path);
        path_t(const path_t& path);
        path_t(const path_t& lhspath, const path_t& rhspath);
        ~path_t();

        path_t resolve(filesys_t* fs, filedevice_t*& outdevice) const;

        void set_filepath(runes_t& runes, runes_alloc_t* allocator);
        void set_dirpath(runes_t& runes, runes_alloc_t* allocator);

        void combine(const path_t& dirpath, const path_t& filepath);
        void copy_dirpath(runes_t& runes);
        void clear();
        void erase();

        bool isEmpty() const;
        bool isRoot() const;
        bool isRooted() const;
        bool isSubDirOf(const path_t& dirpath) const;

        s32  getLevels() const;
        bool getLevel(s32 level, path_t& dirpath) const;
        s32  getLevelOf(const path_t& dirname) const;
        bool split(s32 level, path_t& parent_dirpath, path_t& relative_filepath) const;

        void makeRelative();
        void makeRelativeTo(const path_t& parent_dirpath);

        void setRootDir(const path_t& in_root_dirpath);
        bool getRootDir(path_t& out_root_dirpath) const;
        bool getDirname(path_t& out_dirpath) const;

        bool up();
        bool down(path_t const& runes);

        void getFilename(path_t& out_filename) const;
        void getFilenameWithoutExtension(path_t& out_filename_no_ext) const;
        void getExtension(path_t& out_ext) const;

        path_t& operator=(const runes_t&);
        path_t& operator=(const path_t&);
        path_t& operator+=(const runes_t&);

        bool operator==(const path_t&) const;
        bool operator!=(const path_t&) const;

        void toString(runes_t& dst) const;

        static void as_utf16(path_t const& p,  path_t& dst);
        static void as_utf16(filepath_t const& fp, path_t& dst);
        static void as_utf16(filepath_t const& fp, filepath_t& dst);
        static void as_utf16(dirpath_t const& dp, path_t& dst);
        static void as_utf16(dirpath_t const& dp, dirpath_t& dst);
    };

}; // namespace xcore

#endif // __X_FILESYSTEM_XPATH_H__