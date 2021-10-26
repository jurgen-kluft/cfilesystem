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
        s32 m_ref;
        s16 m_len;
        s16 m_cap;
        pathname_t* m_path[1];

        path_t() : m_ref(0), m_len(0), m_cap(1) { }

        path_t* attach();
        bool detach(filesysroot_t* root);

        pathname_t* get_name() const;
        path_t* prepend(pathname_t* folder, alloc_t* allocator);
        path_t* append(pathname_t* folder, alloc_t* allocator);

        s32 compare(path_t* other) const;

        static path_t* construct(alloc_t* allocator, s32 len);
        static void destruct(alloc_t* allocator, path_t* path);
        static void copy_array(pathname_t** from, u32 from_start, u32 from_len, pathname_t** dst, u32 dst_start);
    };

    struct pathname_t
    {
        u64      m_hash;
        pathname_t* m_next;
        s32      m_refs;
        s32      m_len;
        utf32::rune m_name[2];

        pathname_t();
        pathname_t(s32 strlen);

        bool isEmpty() const;

        static s32 compare(const pathname_t* name, const pathname_t* other);
        s32 compare(const pathname_t* other) const;
        s32 compare(crunes_t const& name) const;

        pathname_t* incref();
        pathname_t* release(alloc_t* allocator);
        static bool release(pathname_t* name);
        static pathname_t* construct(alloc_t* allocator, u64 hname, crunes_t const& name);

        XCORE_CLASS_PLACEMENT_NEW_DELETE
    };

    struct pathname_table_t
    {
        void     initialize(alloc_t* allocator, s32 cap=65536);

        pathname_t* find(u64 hash) const;
        pathname_t* next(u64 hash, pathname_t* prev) const;
        void     insert(pathname_t* name);
        bool     remove(pathname_t* name);
        u32      hash_to_index(u64 hash) const;

        s32       m_len;
        s32       m_cap;
        pathname_t** m_table;
    };

    struct pathdevice_t
    {
        filesysroot_t*   m_root; // If alias, pathdevice_t ("work") | nullptr
        filedevice_t*    m_fd;   // nullptr                      | xfiledevice("e")
        pathname_t*      m_name; // "codedir"                    | "E"
        pathname_t*      m_path; // "codedir:\\xfilesystem\\"    | "E:\\dev.go\\src\\github.com\\jurgen-kluft\\"

        void       init(filesysroot_t* owner);

        pathdevice_t* attach() { return this; }
        pathdevice_t* release(alloc_t* allocator) { return this; }

        static pathdevice_t* construct(alloc_t* allocator, filesysroot_t* owner);
        static void       destruct(alloc_t* allocator, pathdevice_t*& device);
    };

    struct filesysroot_t
    {
        filesysroot_t(alloc_t* allocator) : m_allocator(allocator) {}

        u32                 m_max_open_files;
        u32                 m_max_path_objects;
        char                m_default_slash;
        filesys_t*          m_owner;
        alloc_t*            m_allocator;
        runes_alloc_t*      m_stralloc;
        s32                 m_num_devices;
        s32                 m_max_devices;
        pathdevice_t        m_tdevice[64];
        pathname_table_t    m_filename_table;
        pathname_table_t    m_extension_table;

        static pathdevice_t* sNilDevice;
        static pathname_t*   sNilName;
        static path_t*       sNilPath;

        void initialize(alloc_t* allocator);

        void release_name(pathname_t* name);
        void release_filename(pathname_t* name);
        void release_extension(pathname_t* name);
        void release_path(path_t* path);
        void release_device(pathdevice_t* dev);

        pathname_t* register_name(crunes_t const& namestr);
        pathname_t* get_empty_name() const;

        bool register_directory(crunes_t const& directory, pathname_t*& out_devicename, path_t*& out_path);
        bool register_directory(path_t** paths_to_concatenate, s32 paths_len, path_t*& out_path);
        bool register_filename(crunes_t const& filename, pathname_t*& out_filename, pathname_t*& out_extension);
        bool register_fullfilepath(crunes_t const& fullfilepath, pathname_t*& out_devicename, path_t*& out_path, pathname_t*& out_filename, pathname_t*& out_extension);

        pathname_t* register_dirname(crunes_t const& fulldirname);
        pathname_t* register_extension(crunes_t const& extension);
        pathdevice_t* register_device(crunes_t const& device);
        pathdevice_t* register_device(pathname_t* device);

        path_t* get_parent_path(path_t* path);
        void get_expand_path(path_t* path, pathname_t* folder, path_t*& out_path);
        void get_expand_path(pathname_t* folder, path_t* path, path_t*& out_path);
        void get_expand_path(path_t* left, s32 lstart, s32 llen, path_t* right, s32 rstart, s32 rlen, path_t*& out_path);
        void get_split_path(path_t* path, s32 pivot, path_t** left, path_t** right);
    };

}; // namespace xcore

#endif // __X_FILESYSTEM_XPATH_H__