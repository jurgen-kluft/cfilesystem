#ifndef __X_FILESYSTEM_H__
#define __X_FILESYSTEM_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

//==============================================================================
#include "xbase/x_debug.h"
#include "xbase/x_buffer.h"

#include "xfilesystem/private/x_enumerations.h"

namespace xcore
{
    ///< Forward declares
    class xalloc;
    class xfilesystem;

    struct xfilesyscfg
    {
        inline xfilesyscfg() : m_max_open_stream(32), m_default_slash('/'), m_allocator(nullptr) {}
        u32     m_max_open_stream;
        char    m_default_slash;
        xalloc* m_allocator;
    };

    ///< Initialization
    extern xfilesystem* create_fs(xfilesyscfg const& cfg);
    extern void         destroy_fs(xfilesystem*);

    class xfile;
    class xwriter;
    class xreader;
    class xfilepath;
    class xfileinfo;
    class xdirpath;
    class xdirinfo;

    class xfilesystem
    {
    public:
        xfile*     open(xfilepath const& filename, EFileMode mode);
        xstream*   open_stream(const xfilepath& filename, EFileMode mode, EFileAccess access, EFileOp op);
        xwriter*   writer(xfile*);
        xreader*   reader(xfile*);
        void       close(xfile*);
        void       close(xfileinfo*);
        void       close(xdirinfo*);
        void       close(xreader*);
        void       close(xwriter*);
        void       close(xstream*);
        xfileinfo* info(xfilepath const& path);
        xdirinfo*  info(xdirpath const& path);
        bool       exists(xfileinfo*);
        bool       exists(xdirinfo*);
        s64        size(xfileinfo*);
        xfile*     open(xfileinfo*, EFileMode mode);
        void       rename(xfileinfo*, xfilepath const&);
        void       move(xfileinfo* src, xfileinfo* dst);
        void       copy(xfileinfo* src, xfileinfo* dst);
        void       rm(xfileinfo*);
        void       rm(xdirinfo*);
        s32        read(xreader*, xbuffer&);
        s32        write(xwriter*, xcbuffer const&);
        void       read_async(xreader*, xbuffer&);
        s32        wait_async(xreader*);

    protected:
		friend class _xfilesystem_;
        _xfilesystem_* mImpl;
    };

    // doIO; user has to call this from either the main thread or an IO thread.
    // This call will block the calling thread and it will stay in a do-while
    // until threading->loop() is false.
    class xio_thread;
    extern void doIO(xio_thread*);

}; // namespace xcore

#endif // __X_FILESYSTEM_H__
