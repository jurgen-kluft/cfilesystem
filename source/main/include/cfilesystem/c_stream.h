#ifndef __CFILESYSTEM_XSTREAM_H__
#define __CFILESYSTEM_XSTREAM_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "ccore/c_debug.h"
#include "cbase/c_buffer.h"

#include "cfilesystem/private/c_enumerations.h"

namespace ncore
{
    class istream_t;
    namespace nfs
    {
        struct filehandle_t;
        class filedevice_t;

        // The main interface of a stream object, user deals with this object most of the time.
        class stream_t
        {
        public:
            stream_t();
            stream_t(const stream_t&);
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

            s64 read(u8*, s64);
            s64 write(u8 const*, s64);

            stream_t& operator=(const stream_t&);

        protected:
            stream_t(istream_t* impl, filehandle_t* fh);

            filehandle_t*     m_filehandle;
            istream_t*        m_pimpl;
            s64               m_offset;
            EStreamCaps::Enum m_caps;

            friend class filesystem_t;
            friend class filesys_t;
        };

        extern void stream_copy(stream_t& src, stream_t& dst, buffer_t& buffer);

    } // namespace nfs
}; // namespace ncore

#endif
