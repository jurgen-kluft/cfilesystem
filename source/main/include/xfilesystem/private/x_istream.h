#ifndef __XFILESYSTEM_XISTREAM_H__
#define __XFILESYSTEM_XISTREAM_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

//==============================================================================
#include "xbase/x_debug.h"

#include "xfilesystem/private/x_enumerations.h"

namespace xcore
{
    class xalloc;
	class xfilepath;

    class xistream
    {
    public:
        virtual ~xistream(void) {}

        virtual void hold()    = 0;
        virtual s32  release() = 0;
        virtual void destroy() = 0;

        virtual bool canRead() const       = 0;
        virtual bool canSeek() const       = 0;
        virtual bool canWrite() const      = 0;
        virtual bool isOpen() const        = 0;
        virtual bool isAsync() const       = 0;
        virtual u64  getLength() const     = 0;
        virtual void setLength(u64 length) = 0;
        virtual s64  getPos() const = 0;
        virtual s64  setPos(s64 pos)       = 0;

        virtual void close() = 0;
        virtual void flush() = 0;

        virtual u64 read(xbyte* buffer, u64 count)        = 0;
        virtual u64 write(const xbyte* buffer, u64 count) = 0;

        virtual bool beginRead(xbyte* buffer, u64 count)        = 0;
        virtual s64  endRead(bool block)                        = 0;
        virtual bool beginWrite(const xbyte* buffer, u64 count) = 0;
        virtual s64  endWrite(bool block)                       = 0;
	
	protected:
		friend class xstream;

		static xistream*	create_filestream(xalloc*, const xfilepath& filepath, EFileMode mode, EFileAccess access, EFileOp op);
		static void			destroy_filestream(xalloc*, xistream*);
    };

};

#endif
