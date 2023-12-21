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

    // The whole path table should become a red-black tree and not a hash table.
    // Every 'folder' has siblings (files and folders), each 'folder' sibling again
    // has siblings (files and folders) and so on. The root folder is the only one
    // that has no siblings. The root folder is the only one that has a null parent.
    // This means using a red-black tree we can have trees within trees within trees, and
    // with a hash table it would be harder to handle 'sizing' of each sub table.
    //
    struct paths_t
    {
        paths_t();

        void init(alloc_t* allocator, u32 cap = 1024 * 1024);
        void release(alloc_t* allocator);

        enum EType
        {
            kDevice = 0,
            kFolder = 1,
            kFile   = 2,
            kString = 3,
            kMask   = 0xF,
            kColor  = 0x10,
        };

        struct name_t
        {
            u32           m_length;
            utf32::pcrune str() const { return reinterpret_cast<utf32::pcrune>(this + 1); }
        };

        struct node_t
        {
            u32 m_siblings[2]; // rbtree(left/right)
            u32 m_item;        // index to file_t, folder_t or offset into text data
            u8  m_flags;
        };

        struct file_t
        {
            node_t* m_parent; // parent folder (not part of rbtree)
        };

        struct folder_t
        {
            node_t* m_parent;  // parent folder (not part of rbtree)
            node_t* m_files;   // rbtree, content root, files
            node_t* m_folders; // rbtree, content root, sub folders
        };

        static inline bool is_typeof(node_t* n, EType type) { return (n->m_flags & kMask) == type; }
        static inline bool is_red(node_t* n) { return n->m_flags & kColor == kColor; }
        static inline bool is_black(node_t* n) { return n->m_flags & kColor == 0; }
        static inline void set_red(node_t* n) { n->m_flags |= kColor; }
        static inline void set_black(node_t* n) { n->m_flags &= ~kColor; }
        static bool        is_filetype(node_t* n) { return is_typeof(n, kFile); }
        file_t*            get_file(node_t* n); // file_t

        static bool is_foldertype(node_t* n) { return is_typeof(n, kFolder); }
        folder_t*   get_folder(node_t* n); // folder_t
        static bool is_stringtype(node_t* n) { return is_typeof(n, kString); }
        static void get_string(node_t* n, name_t*& str);

        node_t* find(utf32::pcrune item, EType type) const;
        node_t* insert(utf32::pcrune item, EType type);
        bool    remove(node_t* item);

        utf32::rune* m_text_data; // Virtual memory array ([length, rune[], length, rune[], ...])

        node_t*   m_node_array;        // Virtual memory array
        node_t*   m_node_free_head;    // Head of the free list
        u64       m_node_free_index;   // Index of the free list
        file_t*   m_file_array;        // Virtual memory array
        file_t*   m_file_free_head;    // Head of the free list
        u64       m_file_free_index;   // Index of the free list
        folder_t* m_folder_array;      // Virtual memory array
        folder_t* m_folder_free_head;  // Head of the free list
        u64       m_folder_free_index; // Index of the free list

        node_t* m_strings_root;    // bst-tree root, all strings
        node_t* m_devices_root;    // bst-tree root, device names
        node_t* m_files_root;      // bst-tree root, file names
        node_t* m_folders_root;    // bst-tree root, folder names
        node_t* m_extensions_root; // bst-tree root, file extensions
    };

    typedef paths_t::node_t pathnode_t;

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
}; // namespace ncore

#endif // __C_FILESYSTEM_PATH_H__