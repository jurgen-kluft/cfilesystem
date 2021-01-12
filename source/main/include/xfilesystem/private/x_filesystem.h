#ifndef __X_FILESYSTEM_FILESYSTEM_IMP_H__
#define __X_FILESYSTEM_FILESYSTEM_IMP_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

//==============================================================================
#include "xbase/x_allocator.h"
#include "xbase/x_buffer.h"
#include "xbase/x_runes.h"

#include "xfilesystem/private/x_enumerations.h"
#include "xfilesystem/private/x_path.h"

namespace xcore
{
    class filesys_t;
    class filedevice_t;
    class fileinfo_t;
    class dirinfo_t;
    class devicemanager_t;
    class stream_t;
    class istream_t;

    struct filehandle_t
    {
        void*         m_handle;
        filesys_t*    m_owner;
        s32           m_refcount;
        s32           m_salt;
        path_t        m_path;
        filehandle_t* m_prev;
        filehandle_t* m_next;
    };

    class filesys_t
    {
    public:
        char             m_slash;
        alloc_t*         m_allocator;
        runes_alloc_t*   m_stralloc;
        devicemanager_t* m_devman;

        filehandle_t* m_filehandle_list_free;
        filehandle_t* m_filehandle_list_active;
        filehandle_t* m_filehandle_array;

        XCORE_CLASS_PLACEMENT_NEW_DELETE

        static stream_t      create_filestream(const filepath_t& filepath, EFileMode fm, EFileAccess fa, EFileOp fo);
        static void          destroy(stream_t& stream);
        static filepath_t    resolve(filepath_t const&, filedevice_t*& device);
        static dirpath_t     resolve(dirpath_t const&, filedevice_t*& device);
        static path_t&       get_path(dirinfo_t& dirinfo);
        static path_t const& get_path(dirinfo_t const& dirinfo);
        static path_t&       get_path(dirpath_t& dirpath);
        static path_t const& get_path(dirpath_t const& dirpath);
        static path_t&       get_path(filepath_t& filepath);
        static path_t const& get_path(filepath_t const& filepath);
        static filesys_t*    get_filesystem(dirpath_t const& dirpath);
        static filesys_t*    get_filesystem(filepath_t const& filepath);

        // -----------------------------------------------------------
        bool register_device(const crunes_t& device_name, filedevice_t* device);

        filepath_t filepath(const char* str);
        dirpath_t  dirpath(const char* str);
        filepath_t filepath(const crunes_t& str);
        dirpath_t  dirpath(const crunes_t& str);

        stream_t   open(const filepath_t& filename, EFileMode mode, EFileAccess access, EFileOp op);
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