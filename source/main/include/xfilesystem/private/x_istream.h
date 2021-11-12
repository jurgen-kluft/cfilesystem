#ifndef __XFILESYSTEM_XISTREAM_H__
#define __XFILESYSTEM_XISTREAM_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

namespace xcore
{
    struct filehandle_t;
    class filedevice_t;

    class istream_t
    {
    public:
        virtual u64  getLength(filedevice_t* fd, filehandle_t* fh) const = 0;
        virtual void setLength(filedevice_t* fd, filehandle_t* fh, u64 length) = 0;
        virtual s64  setPos(filedevice_t* fd, filehandle_t* fh, u32 caps, s64& current, s64 pos) = 0;
        virtual void close(filedevice_t* fd, filehandle_t*& fh) = 0;
        virtual s64 read(filedevice_t* fd, filehandle_t* fh, u32 caps, s64& pos, xbyte* buffer, s64 count) = 0;
        virtual s64 write(filedevice_t* fd, filehandle_t* fh, u32 caps, s64& pos, const xbyte* buffer, s64 count) = 0;
    };
};

#endif
