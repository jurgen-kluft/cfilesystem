#ifndef __C_FILESYSTEM_FILESYSTEM_IMP_H__
#define __C_FILESYSTEM_FILESYSTEM_IMP_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

//==============================================================================
#include "cbase/c_allocator.h"
#include "cfilesystem/private/c_enumerations.h"
#include "cfilesystem/private/c_path.h"

namespace ncore
{
    struct runes_t;
    struct crunes_t;
    class alloc_t;

    class filesys_t;
    class filedevice_t;
    class stream_t;
    class istream_t;

    struct filehandle_t
    {
        void*         m_handle;
        filesys_t*    m_owner;
        s32           m_refcount;
        s32           m_salt;
        filedevice_t* m_filedevice;
        pathdevice_t* m_device;
        pathnode_t*   m_path;
        pathstr_t*    m_filename;
        pathstr_t*    m_extension;
        filehandle_t* m_prev;
        filehandle_t* m_next;
    };

    class filesys_t
    {
    public:
        void init(alloc_t* allocator);
        void exit(alloc_t* allocator);

        // -----------------------------------------------------------
        filehandle_t* m_filehandle_list_free;
        filehandle_t* m_filehandle_list_active;
        filehandle_t* m_filehandle_array;

        // -----------------------------------------------------------
        static void          destroy(stream_t& stream);
        static void          resolve(filepath_t const&, pathdevice_t*& device, pathnode_t*& dir, pathstr_t*& filename, pathstr_t*& extension);
        static void          resolve(dirpath_t const&, pathdevice_t*& device, pathnode_t*& dir);
        static pathdevice_t* get_pathdevice(dirpath_t const& dirpath);
        static pathdevice_t* get_pathdevice(filepath_t const& filepath);
        static pathnode_t*   get_path(dirpath_t const& dirpath);
        static pathnode_t*   get_path(filepath_t const& filepath);
        static pathstr_t*    get_filename(filepath_t const& filepath);
        static pathstr_t*    get_extension(filepath_t const& filepath);
        static filesys_t*    get_filesystem(dirpath_t const& dirpath);
        static filesys_t*    get_filesystem(filepath_t const& filepath);

        // -----------------------------------------------------------
        void open(const filepath_t& filename, EFileMode mode, EFileAccess access, EFileOp op, stream_t& out_stream);
        void close(stream_t& stream);
        bool exists(filepath_t const&);
        bool exists(dirpath_t const&);
        s64  size(filepath_t const&);
        void rename(filepath_t const&, filepath_t const&);
        void move(filepath_t const& src, filepath_t const& dst);
        void copy(filepath_t const& src, filepath_t const& dst);
        void rm(filepath_t const&);
        void rm(dirpath_t const&);

        // -----------------------------------------------------------
        //
        u32            m_max_open_files;
        u32            m_max_path_objects;
        char           m_default_slash;
        alloc_t*       m_allocator;
        runes_alloc_t* m_stralloc;
        s32            m_num_devices;
        s32            m_max_devices;
        pathdevice_t   m_tdevice[64];
        paths_t*       m_paths; // all data for pathnode_t, pathnode_t, pathnode_t, etc.

        static pathdevice_t* sNilDevice;
        static pathstr_t*    sNilName;
        static pathnode_t*   sNilNode;

        void filepath(const char* str, filepath_t&);
        void dirpath(const char* str, dirpath_t&);
        void filepath(const crunes_t& str, filepath_t&);
        void dirpath(const crunes_t& str, dirpath_t&);

        void register_fulldirpath(crunes_t const& fulldirpath, pathstr_t*& out_devicename, pathnode_t*& out_path);
        void register_fullfilepath(crunes_t const& fullfilepath, pathstr_t*& out_devicename, pathnode_t*& out_path, pathstr_t*& out_filename, pathstr_t*& out_extension);
        void register_dirpath(crunes_t const& dirpath, pathnode_t*& out_path);
        void register_filename(crunes_t const& namestr, pathstr_t*& filename, pathstr_t*& extension);
        void register_name(crunes_t const& namestr, pathstr_t*& name);

        pathdevice_t* register_device(crunes_t const& device);
        pathdevice_t* register_device(pathstr_t* device);

        void release_name(pathstr_t* name);
        void release_filename(pathstr_t* name);
        void release_extension(pathstr_t* name);
        void release_path(pathnode_t* path);
        void release_device(pathdevice_t* dev);

        pathnode_t* get_parent_path(pathnode_t* path);

        bool has_device(const crunes_t& device_name);
        bool register_device(const crunes_t& device_name, filedevice_t* device);
        bool register_alias(const crunes_t& alias_name, const crunes_t& device_name);

        filehandle_t* obtain_filehandle();
        void          release_filehandle(filehandle_t* fh);

        filehandle_t*  m_filehandles_array;
        filehandle_t** m_filehandles_free;
        s32            m_filehandles_count;

        pathstr_t*   find_name(crunes_t const& namestr) const;
        pathdevice_t* find_device(pathstr_t* devicename) const;

        DCORE_CLASS_PLACEMENT_NEW_DELETE
    };

}; // namespace ncore

#endif