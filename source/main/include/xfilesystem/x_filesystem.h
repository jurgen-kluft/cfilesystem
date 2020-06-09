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

namespace xcore
{
    class xalloc;
    class xfilesystem;

    struct xfilesyscfg
    {
        inline xfilesyscfg()
            : m_max_open_stream(32)
            , m_default_slash('/')
            , m_allocator(nullptr)
        {
        }
        u32     m_max_open_stream;
        char    m_default_slash;
        xalloc* m_allocator;
    };

    class xfile;
    class xstream;
    class xwriter;
    class xreader;
    class xfilepath;
    class xfileinfo;
    class xdirpath;
    class xdirinfo;

    class xfilesystem
    {
    public:
        static void	      create(xfilesyscfg const&);
        static void       destroy();

		static xfilepath  filepath(const char* str);
		static xdirpath   dirpath(const char* str);
        static xfilepath  filepath(const utf32::crunes& str);
		static xdirpath   dirpath(const utf32::crunes& str);
        static void       to_ascii(xfilepath const& fp, ascii::runes& str);
		static void       to_ascii(xdirpath const& dp, ascii::runes& str);

		static xfile*     open(xfilepath const& filename, EFileMode mode);
        static xstream*   open_stream(const xfilepath& filename, EFileMode mode, EFileAccess access, EFileOp op);
        static xwriter*   writer(xfile*);
        static xreader*   reader(xfile*);
        static void       close(xfile*);
        static void       close(xfileinfo*);
        static void       close(xdirinfo*);
        static void       close(xreader*);
        static void       close(xwriter*);
        static void       close(xstream*);
        static xfileinfo* info(xfilepath const& path);
        static xdirinfo*  info(xdirpath const& path);
        static bool       exists(xfileinfo*);
        static bool       exists(xdirinfo*);
        static s64        size(xfileinfo*);
        static xfile*     open(xfileinfo*, EFileMode mode);
        static void       rename(xfileinfo*, xfilepath const&);
        static void       move(xfileinfo* src, xfileinfo* dst);
        static void       copy(xfileinfo* src, xfileinfo* dst);
        static void       rm(xfileinfo*);
        static void       rm(xdirinfo*);
        static s32        read(xreader*, xbuffer&);
        static s32        write(xwriter*, xcbuffer const&);
        static void       read_async(xreader*, xbuffer&);
        static s32        wait_async(xreader*);

    protected:
        friend class xfilesys;
        static xfilesys* mImpl;
    };

    // doIO; user has to call this from either the main thread or an IO thread.
    // This call will block the calling thread and it will stay in a do-while
    // until xio_thread->quit() is true.
    class xio_thread;
    extern void doIO(xio_thread*);

}; // namespace xcore

#endif // __X_FILESYSTEM_H__
