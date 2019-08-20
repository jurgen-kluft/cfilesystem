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

namespace xcore
{
    class xdatetime;

    class xfile;
    class xfiledevice;
    class xdevicemanager;
    class xfileinfo;
    class xdirinfo;
    class xfilepath;
    class xdirpath;
    struct xfileattrs;
    struct xfiletimes;
    class xstream;
    class xistream;

    class xfilesys
    {
    public:
        char            m_slash;
        xalloc*         m_allocator;
        utf32::alloc*   m_stralloc;
        xdevicemanager* m_devman;

        static xfilepath resolve(xfilepath const&, xfiledevice*& device);
        static xdirpath  resolve(xdirpath const&, xfiledevice*& device);

        static xpath&       get_xpath(xdirinfo& dirinfo);
        static xpath const& get_xpath(xdirinfo const& dirinfo);
        static xpath&       get_xpath(xdirpath& dirpath);
        static xpath const& get_xpath(xdirpath const& dirpath);
        static xpath&       get_xpath(xfilepath& filepath);
        static xpath const& get_xpath(xfilepath const& filepath);
        static xfilesys*    get_filesystem(xdirpath const& dirpath);
        static xfilesys*    get_filesystem(xfilepath const& filepath);

        static xfiledevice* get_filedevice(xfilepath const& filepath);
        static xfiledevice* get_filedevice(xdirpath const& dirpath);

        static xistream* create_filestream(const xfilepath& filepath, EFileMode, EFileAccess, EFileOp);
        static void      destroy(xistream* stream);

        // -----------------------------------------------------------
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
    };

}; // namespace xcore

#endif