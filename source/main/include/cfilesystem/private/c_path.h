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

    class filepath_t;
    class dirpath_t;
    struct filesysroot_t;
    struct pathdevice_t;

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

        enum EType
        {
            kNil    = 0x00,
            kFolder = 0x01,
            kFile   = 0x02,
            kString = 0x03,
            kMask   = 0x0F,
        };

        // Dynamic length
        struct str_t
        {
            u32                 m_hash;
            u32                 m_len;
            inline utf8::pcrune str() const { return reinterpret_cast<utf8::pcrune>(this + 1); }

            static s32 compare(str_t* left, str_t* right)
            {
                if (left->m_hash < right->m_hash)
                    return -1;
                if (left->m_hash > right->m_hash)
                    return 1;
                return utf8::compare(left->str(), left->m_len, right->str(), right->m_len);
            }
        };

        struct node_t
        {
            u32         m_children[2]; // left, right (rbtree)
            inline bool is_red() const { return (m_children[0] & (0x80000000)) != 0; }
            inline bool is_black() const { return (m_children[0] & (0x80000000)) == 0; }
            inline void set_red() { m_children[0] |= (0x80000000); }
            inline void set_black() { m_children[0] &= ~(0x80000000); }
            inline u32  get_left() const { return m_children[0] & ~(0x80000000); }
            inline u32  get_right() const { return m_children[1]; }
            inline u32  get_child(s8 child) const { return m_children[child] & ~(0x80000000); }
            inline void set_left(u32 index) { m_children[0] = (m_children[0]) | (index & ~(0x80000000)); }
            inline void set_right(u32 index) { m_children[1] = index; }
            inline void set_left(s8 child, u32 index) { m_children[child] = (m_children[child]) | (index & ~(0x80000000)); }
        };

        // Techically this is str_t but now part of a rbtree
        struct name_t : node_t
        {
            str_t* m_str; // Pointing somewhere in 'm_text_data'

            void reset()
            {
                m_children[0] = 0;
                m_children[1] = 0;
                m_str         = nullptr;
            }
        };

        struct folder_t : node_t
        {
            folder_t* m_parent; // folder parent
            str_t*    m_name;   // folder name

            void reset()
            {
                m_children[0] = 0;
                m_children[1] = 0;
                m_parent      = nullptr;
                m_name        = nullptr;
            }
        };

        void          init(alloc_t* allocator, u32 cap = 1024 * 1024);
        void          release(alloc_t* allocator);
        folder_t*     attach(folder_t* node) {}
        name_t*       attach(name_t* node) { return node; }
        void          detach(folder_t* node) {}
        void          detach(name_t* node) {}
        pathdevice_t* attach(pathdevice_t* device) { return device; }
        void          detach(pathdevice_t* device) {}

        name_t*   findOrInsert(crunes_t const& str);
        bool      remove(name_t* item);
        folder_t* findOrInsert(folder_t* parent, name_t* str);
        bool      remove(folder_t* item);
        void      to_string(folder_t* str, runes_t& out_str) const;
        void      to_string(name_t* str, runes_t& out_str) const;
        s32       to_strlen(folder_t* str) const;
        s32       compare(name_t* left, name_t* right) const { return str_t::compare(left->m_str, right->m_str); }
        s32       compare(folder_t* left, folder_t* right) const { return str_t::compare(left->m_name, right->m_name); }

        inline folder_t* get_nil_node() const { return m_nil_node; }
        name_t*          get_nil_name() const { return m_nil_str; }

        void* m_text_data;      // Virtual memory array (text_t[])
        u64   m_text_data_size; // Current size of the text data
        u64   m_text_data_cap;  // Current capacity of the text data

        folder_t* m_node_array;      // Virtual memory array
        folder_t* m_node_free_head;  // Head of the free list
        u64       m_node_free_index; // Index of the free list

        folder_t* m_nil_node; // sentinel node
        name_t*   m_nil_str;  // sentinel string

        name_t*   m_strings_root; // bst-tree root, all strings
        folder_t* m_nodes_root;   // bst-tree root, paths
    };

    typedef paths_t::folder_t pathnode_t;
    typedef paths_t::name_t   pathstr_t;

    struct pathdevice_t
    {
        inline pathdevice_t() : m_root(nullptr), m_alias(nullptr), m_deviceName(nullptr), m_devicePath(nullptr), m_redirector(nullptr), m_fileDevice(nullptr) {}

        void          init(filesys_t* owner);
        pathdevice_t* construct(alloc_t* allocator, filesys_t* owner);
        void          destruct(alloc_t* allocator, pathdevice_t*& device);
        pathdevice_t* attach();
        bool          detach();
        s32           compare(pathdevice_t* device) const;
        void          to_string(runes_t& str) const;
        s32           to_strlen() const;

        filesys_t*    m_root;
        pathstr_t*    m_alias;      // an alias redirection (e.g. "data")
        pathstr_t*    m_deviceName; // "[appdir:\]data\bin.pc\", "[data:\]files\" to "[appdir:\]data\bin.pc\files\"
        pathnode_t*   m_devicePath; // "appdir:\[data\bin.pc\]", "data:\[files\]" to "appdir:\[data\bin.pc\files\]"
        pathdevice_t* m_redirector; // If device path can point to another pathdevice_t
        filedevice_t* m_fileDevice; // or the final device (e.g. "e:\")
    };
}; // namespace ncore

#endif // __C_FILESYSTEM_PATH_H__