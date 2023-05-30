#ifndef __C_FILESYSTEM_PATH_H__
#define __C_FILESYSTEM_PATH_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "ccore/c_debug.h"
#include "cbase/c_runes.h"
#include "cfilesystem/c_filesystem.h"

namespace ncore
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

    // The whole path table should become a red-black tree and not a hash table.
    // Every 'folder' has siblings (files and folders), each 'folder' sibling again
    // has siblings (files and folders) and so on. The root folder is the only one
    // that has no siblings. The root folder is the only one that has a null parent.
    //
    struct pathitem_t
    {
        pathitem_t* m_siblings[2]; // rbtree, left/right
    };

    struct pathname_t
    {
        u32 m_str_offset; // offset into 'm_textdata'
        u16 m_str_len;    // length of the string
        u8  m_color;      // bst-tree, color (0=red, 1=black)
        u8  m_flags;      // flags; device/folder/file/extension
    };

    struct pathnames_t
    {
        inline pathnames_t() : m_len(0), m_cap(0), m_table(nullptr) {}

        void init(alloc_t* allocator, s32 cap = 65536);
        void release(alloc_t* allocator);

        enum EType
        {
            kDevice    = 0,
            kFolder    = 1,
            kFile      = 2,
            kExtension = 3,
        };

        pathname_t* find(utf32::pcrune item, EType type) const;
        pathname_t* insert(utf32::pcrune item, EType type);
        bool        remove(pathname_t* item);

        utf32::rune* m_text_data;       // Virtual memory array
        pathitem_t*  m_item_array;      // Virtual memory array
        pathitem_t*  m_item_free_head;  // Head of the free list
        u64          m_item_free_index; // Index of the free list
        pathname_t*  m_item_keys;       // Virtual memory array
        pathitem_t*  m_device_names;    // bst-tree root, device names
        pathitem_t*  m_file_names;      // bst-tree root, file names
        pathitem_t*  m_folder_names;    // bst-tree root, folder names
        pathitem_t*  m_extensions;      // bst-tree root, file extensions
    };

    struct pathnode_t
    {
        pathname_t* m_name;    // file/folder name
        pathnode_t* m_parent;  // parent folder (not part of rbtree)
        pathnode_t* m_files;   // rbtree, content root, files
        pathnode_t* m_folders; // rbtree, content root, sub folders
    };

    struct pathnodes_t
    {
        pathitem_t* m_treenode_free;   //
        pathitem_t* m_treenode_array;  // Should be a virtual memory array
        u8*         m_treenode_colors; //
        pathnode_t* m_treenode_keys;   //
    };

    struct pathdevice_t
    {
        inline pathdevice_t() : m_root(nullptr), m_alias(nullptr), m_deviceName(nullptr), m_devicePath(nullptr), m_redirector(nullptr), m_fileDevice(nullptr) {}

        void          init(filesys_t* owner);
        pathdevice_t* construct(alloc_t* allocator, filesys_t* owner);
        pathdevice_t* attach();
        bool          detach(filesys_t* root);
        void          destruct(alloc_t* allocator, pathdevice_t*& device);
        s32           compare(pathdevice_t* device) const;
        void          to_string(runes_t& str) const;
        s32           to_strlen() const;

        filesys_t*    m_root;       //
        pathname_t*   m_alias;      // an alias redirection (e.g. "data")
        pathname_t*   m_deviceName; // "[appdir:\]data\bin.pc\", "[data:\]files\" to "[appdir:\]data\bin.pc\files\"
        pathnode_t*   m_devicePath; // "appdir:\[data\bin.pc\]", "data:\[files\]" to "appdir:\[data\bin.pc\files\]"
        pathdevice_t* m_redirector; // If device path can point to another pathdevice_t
        filedevice_t* m_fileDevice; // or the final device (e.g. "e:\")
    };
};                                  // namespace ncore

#endif                              // __C_FILESYSTEM_PATH_H__