#ifndef __C_FILESYSTEM_H__
#define __C_FILESYSTEM_H__
#include "cbase/c_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

#include "cbase/c_allocator.h"
#include "cbase/c_buffer.h"
#include "cbase/c_debug.h"
#include "cbase/c_runes.h"

#include "cfilesystem/private/c_enumerations.h"
#include "cfilesystem/c_stream.h"

namespace ncore
{
    class alloc_t;
    class filesystem_t;

    class filepath_t;
    class dirpath_t;
    class filesys_t;
    class filedevice_t;

    class filesystem_t
    {
    public:
        struct context_t
        {
            inline context_t() : m_allocator(nullptr), m_stralloc(nullptr), m_max_open_files(32), m_max_path_objects(8192), m_default_slash('/') {}
            alloc_t*       m_allocator;
            runes_alloc_t* m_stralloc;
            u32            m_max_open_files;
            u32            m_max_path_objects;
            char           m_default_slash;
        };

        static void create(context_t const&);
        static void destroy();

        static bool register_device(const crunes_t& device_name, filedevice_t*);

        static filepath_t filepath(const char* str);
        static dirpath_t  dirpath(const char* str);
        static filepath_t filepath(const crunes_t& str);
        static dirpath_t  dirpath(const crunes_t& str);

        static void        open(const filepath_t& filename, EFileMode mode, EFileAccess access, EFileOp op, stream_t& out_stream);
        static void        close(stream_t&);
        static bool        exists(filepath_t const&);
        static bool        exists(dirpath_t const&);
        static s64         size(filepath_t const&);
        static void        rename(filepath_t const&, filepath_t const&);
        static void        move(filepath_t const& src, filepath_t const& dst);
        static void        copy(filepath_t const& src, filepath_t const& dst);
        static void        rm(filepath_t const&);
        static void        rm(dirpath_t const&);

    protected:
        friend class filesys_t;
        static filesys_t* mImpl;
    };

    // doIO; user has to call this from either the main thread or an IO thread.
    // This call will block the calling thread and it will stay in a do-while
    // until io_thread_t->quit() is true.
    class io_thread_t;
    extern void doIO(io_thread_t*);

}; // namespace ncore

#endif // __C_FILESYSTEM_H__
