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
    class xistream;

    ///< xstream object
    ///< The main interface of a stream object, user deals with this object most of the time.
    class xstream
    {
    public:
        xstream();
        ~xstream();

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
        xstream(xistream*);
        xstream(const xstream&);

        xstream& operator=(const xstream&) { return *this; }

        xistream* m_pimpl;

        friend class xfilesystem;
    };

    void xstream_copy(xstream* src, xstream* dst, xbuffer& buffer);
    void xstream_copy(xstream* src, xstream* dst, u64 count, xbuffer& buffer);

}; // namespace xcore

#endif
