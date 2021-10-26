#ifndef __X_FILESYSTEM_FILESYSTEM_IMP_H__
#define __X_FILESYSTEM_FILESYSTEM_IMP_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

//==============================================================================
#include "xfilesystem/private/x_enumerations.h"

namespace xcore
{
    struct runes_t;
    struct crunes_t;
    class alloc_t;

    class filesys_t;
    class filedevice_t;
    class fileinfo_t;
    class dirinfo_t;
    class devicemanager_t;
    class stream_t;
    class istream_t;

    struct pathdevice_t;
    struct path_t;
    struct pathname_t;

    struct filehandle_t
    {
        void*         m_handle;
        filesys_t*    m_owner;
        s32           m_refcount;
        s32           m_salt;
        pathdevice_t* m_device;
        path_t*       m_path;
        pathname_t*   m_filename;
        pathname_t*   m_extension;
        filehandle_t* m_prev;
        filehandle_t* m_next;
    };

    class filesys_t
    {
    public:
        filesysroot_t*     m_context;
        devicemanager_t*   m_devman;

        filehandle_t* m_filehandle_list_free;
        filehandle_t* m_filehandle_list_active;
        filehandle_t* m_filehandle_array;

        XCORE_CLASS_PLACEMENT_NEW_DELETE

        static void          create_filestream(const filepath_t& filepath, EFileMode fm, EFileAccess fa, EFileOp fo, stream_t& out_stream);
        static void          destroy(stream_t& stream);

        static void          resolve(filepath_t const&, pathdevice_t*& device, path_t*& dir, pathname_t*& filename, pathname_t*& extension);
        static void          resolve(dirpath_t const&, pathdevice_t*& device, path_t*& dir);
        
        static pathdevice_t * get_pathdevice(dirinfo_t const& dirinfo);
        static pathdevice_t * get_pathdevice(dirpath_t const& dirpath);
        static pathdevice_t * get_pathdevice(filepath_t const& filepath);

        static path_t * get_path(dirinfo_t const& dirinfo);
        static path_t * get_path(dirpath_t const& dirpath);
        static path_t * get_path(filepath_t const& filepath);

        static pathname_t * get_filename(filepath_t const& filepath);
        static pathname_t * get_extension(filepath_t const& filepath);

        static filesys_t*    get_filesystem(dirpath_t const& dirpath);
        static filesys_t*    get_filesystem(filepath_t const& filepath);

        // -----------------------------------------------------------
        bool register_device(const crunes_t& device_name, filedevice_t* device);

        void       filepath(const char* str, filepath_t&);
        void       dirpath(const char* str, dirpath_t&);
        void       filepath(const crunes_t& str, filepath_t&);
        void       dirpath(const crunes_t& str, dirpath_t&);

        void       open(const filepath_t& filename, EFileMode mode, EFileAccess access, EFileOp op, stream_t& out_stream);
        void       close(stream_t&);
        bool       exists(fileinfo_t const&);
        bool       exists(dirinfo_t const&);
        s64        size(fileinfo_t const&);
        void       rename(fileinfo_t const&, filepath_t const&);
        void       move(fileinfo_t const& src, fileinfo_t const& dst);
        void       copy(fileinfo_t const& src, fileinfo_t const& dst);
        void       rm(fileinfo_t const&);
        void       rm(dirinfo_t const&);
    };

}; // namespace xcore

#endif