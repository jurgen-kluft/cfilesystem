#ifndef __XFILESYSTEM_XSTREAM_H__
#define __XFILESYSTEM_XSTREAM_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

#include "xbase/x_debug.h"
#include "xbase/x_buffer.h"

#include "xfilesystem/private/x_enumerations.h"

namespace xcore
{
    class istream_t;

    ///< stream_t object
    ///< The main interface of a stream object, user deals with this object most of the time.
    class stream_t
    {
    public:
        stream_t();
        ~stream_t();

        bool canRead() const;
        bool canSeek() const;
        bool canWrite() const;

        bool isOpen() const;
        bool isAsync() const;

        u64  getLength() const;
        void setLength(u64 length);
        s64  getPos() const;
        s64  setPos(s64 pos);

        void close();
        void flush();

        u64  read(xbyte* buffer, u64 count);
        u64  write(const xbyte* buffer, u64 count);

        bool beginRead(xbyte* buffer, u64 count);
        s64  endRead(bool block = true);
        
		bool beginWrite(const xbyte* buffer, u64 count);
        s64  endWrite(bool block = true);

    protected:
        stream_t(istream_t*);
        stream_t(const stream_t&);

        stream_t& operator=(const stream_t&) { return *this; }

        istream_t* m_pimpl;

        friend class filesystem_t;
		friend class filesys_t;
    };

    void xstream_copy(stream_t* src, stream_t* dst, buffer_t& buffer);

}; // namespace xcore

#endif
