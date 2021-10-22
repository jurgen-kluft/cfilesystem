#ifndef __X_FILESYSTEM_FILEPATH_H__
#define __X_FILESYSTEM_FILEPATH_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

#include "xbase/x_debug.h"
#include "xbase/x_runes.h"
#include "xfilesystem/private/x_path.h"
#include "xfilesystem/x_dirpath.h"
#include "xfilesystem/x_filesystem.h"

namespace xcore
{
    class pathname_t;
    class dirpath_t;
    class filesystem_t;

    class filepath_t
    {
        dirpath_t   m_dirpath;
        pathname_t* m_filename;
        pathname_t* m_extension;

        friend class fileinfo_t;

    public:
        filepath_t();
        filepath_t(pathname_t* filename, pathname_t* extension);
        filepath_t(dirpath_t dirpath, pathname_t* filename, pathname_t* extension);
        ~filepath_t();

        void clear();
        bool isRooted() const;
        bool isEmpty() const;

        void makeRelativeTo(const dirpath_t& dirpath);
        void makeAbsoluteTo(const dirpath_t& dirpath);

        void setDirpath(dirpath_t const& dirpath);
        void setFilename(pathname_t* filename);
        void setFilename(crunes_t const& filename);
        void setExtension(pathname_t* extension);
        void setExtension(crunes_t const& extension);

        dirpath_t   root() const;
        dirpath_t   dirpath() const;
        pathname_t* dirname() const;
        pathname_t* filename() const;
        pathname_t* extension() const;

        void split(s32 pivot, dirpath_t& left, filepath_t& right) const;
        void truncate(filepath_t& filepath, pathname_t*& folder) const;
        void truncate(pathname_t*& folder, filepath_t& filepath) const;
        void combine(pathname_t* folder, filepath_t const& filepath);
        void combine(filepath_t const& filepath, pathname_t* folder);

        void down(pathname_t* folder);
        void up();

        void to_string(runes_t& str) const;
    };

    extern filepath_t operator+(const dirpath_t& dirpath, const filepath_t& filepath);

}; // namespace xcore

#endif