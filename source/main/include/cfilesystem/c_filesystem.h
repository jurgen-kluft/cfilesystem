#ifndef __C_FILESYSTEM_FILESYSTEM_H__
#define __C_FILESYSTEM_FILESYSTEM_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "cbase/c_allocator.h"
#include "cbase/c_buffer.h"
#include "ccore/c_debug.h"
#include "cbase/c_runes.h"

#include "cfilesystem/private/c_enumerations.h"
#include "cfilesystem/c_stream.h"

namespace ncore
{
    class filepath_t;
    class dirpath_t;

    namespace nfs
    {
        class filesys_t;
        class filedevice_t;

        struct context_t
        {
            inline context_t() : m_allocator(nullptr), m_max_open_files(32), m_max_path_objects(8192), m_default_slash('/') {}
            alloc_t* m_allocator;
            u32      m_max_open_files;
            u32      m_max_path_objects;
            char     m_default_slash;
        };

        void create(context_t const&);
        void destroy();

        bool register_device(const crunes_t& device_name, filedevice_t*);

        filepath_t filepath(const char* str);
        dirpath_t  dirpath(const char* str);
        filepath_t filepath(const crunes_t& str);
        dirpath_t  dirpath(const crunes_t& str);

        void open(const filepath_t& filename, EFileMode::Enum mode, EFileAccess::Enum access, EFileOp::Enum op, stream_t& out_stream);
        void close(stream_t&);
        bool exists(filepath_t const&);
        bool exists(dirpath_t const&);
        s64  size(filepath_t const&);
        void rename(filepath_t const&, filepath_t const&);
        void move(filepath_t const& src, filepath_t const& dst);
        void copy(filepath_t const& src, filepath_t const& dst);
        void rm(filepath_t const&);
        void rm(dirpath_t const&);

        // doIO; user has to call this from either the main thread or an IO thread.
        // This call will block the calling thread and it will stay in a do-while
        // until io_thread_t->quit() is true.
        class io_thread_t;
        extern void doIO(io_thread_t*);
    }; // namespace nfs
}; // namespace ncore

#endif // __C_FILESYSTEM_H__
