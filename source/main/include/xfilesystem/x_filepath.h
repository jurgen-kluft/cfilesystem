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
    class dirpath_t;
    class filesystem_t;

    //==============================================================================
    // filepath_t:
    //		- Relative:		Folder\Filename.Extension
    //		- Absolute:		Device:\Folder\Folder\Filename.Extension
    //==============================================================================
    class filepath_t
    {
    protected:
        friend class filesys_t;
        friend class fileinfo_t;
		friend class path_t;

        filepath_t(filesys_t* parent, path_t& path);
        filesys_t* mParent;
        path_t     mPath;

    public:
        filepath_t();
        filepath_t(const filepath_t& filepath);
        explicit filepath_t(const dirpath_t& dir, const filepath_t& filename);
        ~filepath_t();

        void       clear();
        bool       isEmpty() const;
        bool       isRooted() const;
        void       makeRelative();
        void       makeRelativeTo(const dirpath_t& dirpath);
		void       makeAbsoluteTo(const dirpath_t& dirpath);
        bool       getRoot(dirpath_t&) const;
        bool       getDirname(dirpath_t&) const;
        void       getFilename(filepath_t&) const;
        void       getFilenameWithoutExtension(filepath_t&) const;
        void       getExtension(filepath_t&) const;
        void       up();
        void       down(dirpath_t const&);

        filepath_t& operator=(const filepath_t&);
        bool       operator==(const filepath_t&) const;
        bool       operator!=(const filepath_t&) const;
    };

    inline filepath_t operator+(const dirpath_t& dir, const filepath_t& filename) { return filepath_t(dir, filename); }

}; // namespace xcore

#endif