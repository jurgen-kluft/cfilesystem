#ifndef __X_FILESYSTEM_H__
#define __X_FILESYSTEM_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

#include "xbase/x_allocator.h"
#include "xbase/x_buffer.h"
#include "xbase/x_debug.h"
#include "xbase/x_runes.h"

#include "xfilesystem/private/x_enumerations.h"
#include "xfilesystem/x_stream.h"

namespace xcore
{
    class alloc_t;
    class filesystem_t;

    struct filesyscfg_t
    {
        inline filesyscfg_t() : m_max_open_files(32), m_default_slash('/'), m_allocator(nullptr) {}
        u32      m_max_open_files;
        char     m_default_slash;
        alloc_t* m_allocator;
    };

    class filepath_t;
    class fileinfo_t;
    class dirpath_t;
    class dirinfo_t;
    class filesys_t;
    class filedevice_t;

    class filesystem_t
    {
    public:
        static void create(filesyscfg_t const&);
        static void destroy();

        static bool register_device(const crunes_t& device_name, filedevice_t*);

        static filepath_t filepath(const char* str);
        static dirpath_t  dirpath(const char* str);
        static filepath_t filepath(const crunes_t& str);
        static dirpath_t  dirpath(const crunes_t& str);

        static stream_t    open(const filepath_t& filename, EFileMode mode, EFileAccess access, EFileOp op);
        static void        close(stream_t&);
        static fileinfo_t  info(filepath_t const& path);
        static dirinfo_t   info(dirpath_t const& path);
        static bool        exists(fileinfo_t const&);
        static bool        exists(dirinfo_t const&);
        static s64         size(fileinfo_t const&);
        static void        rename(fileinfo_t const&, filepath_t const&);
        static void        move(fileinfo_t const& src, fileinfo_t const& dst);
        static void        copy(fileinfo_t const& src, fileinfo_t const& dst);
        static void        rm(fileinfo_t const&);
        static void        rm(dirinfo_t const&);

    protected:
        friend class filesys_t;
        static filesys_t* mImpl;
    };

    // doIO; user has to call this from either the main thread or an IO thread.
    // This call will block the calling thread and it will stay in a do-while
    // until io_thread_t->quit() is true.
    class io_thread_t;
    extern void doIO(io_thread_t*);

}; // namespace xcore

#endif // __X_FILESYSTEM_H__
