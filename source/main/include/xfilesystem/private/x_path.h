#ifndef __X_FILESYSTEM_XPATH_H__
#define __X_FILESYSTEM_XPATH_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

#include "xbase/x_debug.h"
#include "xbase/x_runes.h"
#include "xfilesystem/x_filesystem.h"

namespace xcore
{
    class alloc_t;

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

    class fullpath_parser_utf32
    {
    public:
        crunes_t m_device;
        crunes_t m_path;
        crunes_t m_filename;
        crunes_t m_extension;
        crunes_t m_first_folder;
        crunes_t m_last_folder;

        void parse(const crunes_t& fullpath);

        bool has_device() const { return !m_device.is_empty(); }
        bool has_path() const { return !m_path.is_empty(); }
        bool has_filename() const { return !m_filename.is_empty(); }
        bool has_extension() const { return !m_extension.is_empty(); }

        crunes_t iterate_folder() const { return m_first_folder; }
        bool     next_folder(crunes_t& folder) const;
        crunes_t last_folder() const { return m_last_folder; }
        bool     prev_folder(crunes_t& folder) const;
    };

    class filepath_t;
    class dirpath_t;
    struct filesysroot_t;
    struct pathname_t;

    struct path_t
    {
        s32         m_ref;
        s16         m_len;
        s16         m_cap;
        pathname_t* m_path[1];

        inline path_t() : m_ref(0), m_len(0), m_cap(1) {}

        pathname_t* get_name() const;
        path_t*     prepend(pathname_t* folder, alloc_t* allocator);
        path_t*     append(pathname_t* folder, alloc_t* allocator);
        s32         compare(path_t* other) const;

        path_t*        attach();
        bool           detach(filesys_t* root);
        static path_t* construct(alloc_t* allocator, s32 len);
        static void    destruct(alloc_t* allocator, path_t* path);
        static void    copy_array(pathname_t** from, u32 from_start, u32 from_len, pathname_t** dst, u32 dst_start);
    };

    struct pathname_t
    {
        u64         m_hash;
        pathname_t* m_next;
        s32         m_refs;
        s32         m_len;
        utf32::rune m_name[2];

        pathname_t();
        pathname_t(s32 strlen);

        bool isEmpty() const;

        static s32 compare(const pathname_t* name, const pathname_t* other);
        s32        compare(const pathname_t* other) const;
        s32        compare(crunes_t const& name) const;

        pathname_t*        incref();
        pathname_t*        release(alloc_t* allocator);
        static bool        release(pathname_t* name);
        static pathname_t* construct(alloc_t* allocator, u64 hname, crunes_t const& name);

        XCORE_CLASS_PLACEMENT_NEW_DELETE
    };

    struct pathname_table_t
    {
        inline pathname_table_t() : m_len(0), m_cap(0), m_table(nullptr) {}

        void        initialize(alloc_t* allocator, s32 cap = 65536);
        pathname_t* find(u64 hash) const;
        pathname_t* next(u64 hash, pathname_t* prev) const;
        void        insert(pathname_t* name);
        bool        remove(pathname_t* name);
        u32         hash_to_index(u64 hash) const;

        s32          m_len;
        s32          m_cap;
        pathname_t** m_table;
    };

    struct pathdevice_t
    {
        filesys_t*     m_root; // If alias, pathdevice_t ("work") | nullptr
        filedevice_t*  m_fd;   // nullptr                      | xfiledevice("e")
        pathname_t*    m_name; // "codedir"                    | "E"
        pathname_t*    m_path; // "codedir:\\xfilesystem\\"    | "E:\\dev.go\\src\\github.com\\jurgen-kluft\\"

        pathdevice_t() : m_root(nullptr), m_fd(nullptr), m_name(nullptr), m_path(nullptr) {}

        void          init(filesys_t* owner);
        pathdevice_t* attach() { return this; }
        pathdevice_t* release(alloc_t* allocator) { return this; }
        void          to_string(runes_t& str) const;

        static pathdevice_t* construct(alloc_t* allocator, filesys_t* owner);
        static void          destruct(alloc_t* allocator, pathdevice_t*& device);
    };

}; // namespace xcore

#endif // __X_FILESYSTEM_XPATH_H__