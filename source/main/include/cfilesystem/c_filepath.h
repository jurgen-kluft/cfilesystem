#ifndef __C_FILESYSTEM_FILEPATH_H__
#define __C_FILESYSTEM_FILEPATH_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "ccore/c_debug.h"
#include "cbase/c_runes.h"
#include "cfilesystem/c_dirpath.h"
#include "cfilesystem/c_filesystem.h"

namespace ncore
{
    class dirpath_t;
    class filesystem_t;

    class filepath_t
    {
        dirpath_t  m_dirpath;
        pathstr_t* m_filename;
        pathstr_t* m_extension;

        friend class fileinfo_t;
        friend class filesys_t;
        friend class filedevice_t;
        friend class filedevice_pc_t;

    public:
        filepath_t();
        filepath_t(const filepath_t&);
        filepath_t(pathstr_t* filename, pathstr_t* extension);
        filepath_t(pathdevice_t* device, pathnode_t* dirpath, pathstr_t* filename, pathstr_t* extension);
        filepath_t(dirpath_t const& dirpath, pathstr_t* filename, pathstr_t* extension);
        ~filepath_t();

        void clear();
        bool isRooted() const;
        bool isEmpty() const;

        void makeRelativeTo(const dirpath_t& dirpath);
        void makeAbsoluteTo(const dirpath_t& dirpath);

        void setDevice(crunes_t const& devicename);
        void setDirpath(dirpath_t const& dirpath);
        void setFilename(pathstr_t* filename);
        void setFilename(crunes_t const& filename);
        void setExtension(pathstr_t* extension);
        void setExtension(crunes_t const& extension);

        dirpath_t  root() const;
        dirpath_t  dirpath() const;
        filepath_t filename() const;
        filepath_t relative() const;

        pathnode_t* dirstr() const;
        pathstr_t* filenamestr() const;
        pathstr_t* extensionstr() const;

        void split(s32 pivot, dirpath_t& left, filepath_t& right) const;
        void truncate(filepath_t& filepath, pathnode_t*& folder) const;
        void truncate(pathnode_t*& folder, filepath_t& filepath) const;
        void combine(pathnode_t* folder, filepath_t const& filepath);
        void combine(filepath_t const& filepath, pathnode_t* folder);

        void down(crunes_t const& folder);
        void up();

        s32  compare(const filepath_t& right) const;
        void to_string(runes_t& str) const;
        s32  to_strlen() const;

        filepath_t& operator=(const filepath_t& fp);
    };

    extern bool operator==(const filepath_t& left, const filepath_t& right);
    extern bool operator!=(const filepath_t& left, const filepath_t& right);

    extern filepath_t operator+(const dirpath_t& dirpath, const filepath_t& filepath);

}; // namespace ncore

#endif