#ifndef __X_FILESYSTEM_FILEPATH_H__
#define __X_FILESYSTEM_FILEPATH_H__
#include "cbase/c_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

#include "cbase/c_debug.h"
#include "cbase/c_runes.h"
#include "cfilesystem/c_dirpath.h"
#include "cfilesystem/c_filesystem.h"

namespace ncore
{
    class dirpath_t;
    class filesystem_t;

    struct pathname_t;

    class filepath_t
    {
        dirpath_t   m_dirpath;
        pathname_t* m_filename;
        pathname_t* m_extension;

        friend class fileinfo_t;
        friend class filesys_t;
        friend class filedevice_t;
        friend class filedevice_pc_t;

    public:
        filepath_t();
        filepath_t(const filepath_t&);
        filepath_t(pathname_t* filename, pathname_t* extension);
        filepath_t(pathdevice_t* device, path_t* path, pathname_t* filename, pathname_t* extension);
        filepath_t(dirpath_t const& dirpath, pathname_t* filename, pathname_t* extension);
        ~filepath_t();

        void clear();
        bool isRooted() const;
        bool isEmpty() const;

        void makeRelativeTo(const dirpath_t& dirpath);
        void makeAbsoluteTo(const dirpath_t& dirpath);

        void setDevice(crunes_t const& devicename);
        void setDirpath(dirpath_t const& dirpath);
        void setFilename(pathname_t* filename);
        void setFilename(crunes_t const& filename);
        void setExtension(pathname_t* extension);
        void setExtension(crunes_t const& extension);

        dirpath_t   root() const;
        dirpath_t   base() const;
        dirpath_t   dirpath() const;
        filepath_t  filename() const;
        filepath_t  relative() const;

        pathname_t* dirstr() const;
        pathname_t* filenamestr() const;
        pathname_t* extensionstr() const;

        void split(s32 pivot, dirpath_t& left, filepath_t& right) const;
        void truncate(filepath_t& filepath, pathname_t*& folder) const;
        void truncate(pathname_t*& folder, filepath_t& filepath) const;
        void combine(pathname_t* folder, filepath_t const& filepath);
        void combine(filepath_t const& filepath, pathname_t* folder);

        void down(pathname_t* folder);
        void up();

        s32 compare(const filepath_t& right) const;
        void to_string(runes_t& str) const;
        s32 to_strlen() const;

        filepath_t& operator=(const filepath_t& fp);
    };

    extern bool operator==(const filepath_t& left, const filepath_t& right);
    extern bool operator!=(const filepath_t& left, const filepath_t& right);

    extern filepath_t operator+(const dirpath_t& dirpath, const filepath_t& filepath);

}; // namespace ncore

#endif