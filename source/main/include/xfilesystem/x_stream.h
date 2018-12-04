#ifndef __XFILESYSTEM_XSTREAM_H__
#define __XFILESYSTEM_XSTREAM_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase/x_debug.h"
#include "xfilesystem/private/x_enumerations.h"

namespace xcore
{
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

        u64 seek(s64 offset, ESeekOrigin origin);

        void close();
        void flush();

        u64 read(xbyte* buffer, u64 count);
        u64 write(const xbyte* buffer, u64 count);

        bool beginRead(xbyte* buffer, u64 count);
        bool endRead(bool block = true);

        bool beginWrite(const xbyte* buffer, u64 count);
        bool endWrite(bool block = true);

    protected:
        xstream(xistream*);
        xstream(const xstream&);

        xstream&  operator=(const xstream&) { return *this; }
        xistream* mImplementation;

        friend class xfilesystem;
    };

	bool operator==(const xstream*, const xstream*);
	bool operator!=(const xstream*, const xstream*);

    void xstream_copy(xstream* src, xstream* dst);
    void xstream_copy(xstream* src, xstream* dst, u64 count);

}; // namespace xcore

#endif
