#include "xbase/x_target.h"
#include "xbase/x_debug.h"

#include "xfilesystem/x_stream.h"
#include "xfilesystem/private/x_filedevice.h"
#include "xfilesystem/private/x_filesystem.h"
#include "xfilesystem/private/x_istream.h"
#include "xfilesystem/private/x_enumerations.h"

namespace ncore
{
    class stream_nil : public istream_t
    {
    public:
        virtual ~stream_nil() {}

        virtual u64  getLength(filedevice_t* fd, filehandle_t* fh) const { return 0; }
        virtual void setLength(filedevice_t* fd, filehandle_t* fh, u64 length) { }
        virtual s64  setPos(filedevice_t* fd, filehandle_t* fh, u32 caps, s64& current, s64 pos) { return current; }
        virtual void flush(filedevice_t* fd, filehandle_t*& fh) { }
        virtual void close(filedevice_t* fd, filehandle_t*& fh) { }
        virtual s64 read(filedevice_t* fd, filehandle_t* fh, u32 caps, s64& pos, u8* buffer, s64 count) { return 0; }
        virtual s64 write(filedevice_t* fd, filehandle_t* fh, u32 caps, s64& pos, const u8* buffer, s64 count) { return 0; }
        virtual void release() {}
    };

    static stream_nil sNullStreamImp;

    istream_t* get_nullstream()
    {
        return &sNullStreamImp;
    }

    stream_t::stream_t() : m_caps(0), m_filehandle(nullptr), m_offset(0), m_pimpl(&sNullStreamImp)
    {

    }

    stream_t::stream_t(const stream_t& str) 
        : m_caps(str.m_caps)
        , m_filehandle(str.m_filehandle)
        , m_offset(str.m_offset)
        , m_pimpl(str.m_pimpl)
    {

    }

    stream_t::~stream_t()
    {

    }



    bool stream_t::canRead() const { return true; }
    bool stream_t::canSeek() const { return (m_caps & STREAM_CAPS_SEEK) != 0; }
    bool stream_t::canWrite() const { return (m_caps & STREAM_CAPS_WRITE) != 0; }

    bool stream_t::isOpen() const { return m_filehandle!=INVALID_FILE_HANDLE; }
    bool stream_t::isAsync() const { return (m_caps & STREAM_CAPS_ASYNC) != 0; }

    u64  stream_t::getLength() const { return m_pimpl->getLength(m_filehandle->m_filedevice,m_filehandle); }
    void stream_t::setLength(u64 length) { m_pimpl->setLength(m_filehandle->m_filedevice, m_filehandle, length); }
    s64  stream_t::getPos() const { return m_offset; }
    s64  stream_t::setPos(s64 pos) { m_pimpl->setPos(m_filehandle->m_filedevice, m_filehandle, m_caps, m_offset, pos); return m_offset; }

    void stream_t::close() 
    { 
        m_pimpl->close(m_filehandle->m_filedevice, m_filehandle);
    }

    void stream_t::flush() 
    {
        m_pimpl->flush(m_filehandle->m_filedevice, m_filehandle);
    }

    s64 stream_t::read(u8* buffer, s64 length) 
    { 
        u64 bytesRead = m_pimpl->read(m_filehandle->m_filedevice, m_filehandle, m_caps, m_offset, buffer, length);
        return bytesRead;
    }

    s64 stream_t::write(u8 const* buffer, s64 length)
    {
        u64 bytesWritten = m_pimpl->write(m_filehandle->m_filedevice, m_filehandle, m_caps, m_offset, buffer, length);
        return bytesWritten;
    }

    stream_t& stream_t::operator=(const stream_t& str)
    {
        m_filehandle = str.m_filehandle;
        m_pimpl = str.m_pimpl;
        m_caps = str.m_caps;
        m_offset = str.m_offset;
        return *this;
    }

    stream_t::stream_t(istream_t* impl, filehandle_t* fh)
    {
        m_filehandle = fh;
        m_pimpl = impl;
        m_caps = 0;
        m_offset = 0;
    }

}; // namespace ncore
