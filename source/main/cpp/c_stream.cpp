#include "ccore/c_target.h"
#include "ccore/c_debug.h"

#include "cfilesystem/c_stream.h"
#include "cfilesystem/private/c_filedevice.h"
#include "cfilesystem/private/c_filesystem.h"
#include "cfilesystem/private/c_istream.h"
#include "cfilesystem/private/c_enumerations.h"

namespace ncore
{
    namespace nfs
    {
        class stream_nil : public istream_t
        {
        public:
            virtual ~stream_nil() {}

            virtual bool vcanSeek() const { return false; }
            virtual bool vcanRead() const { return false; }
            virtual bool vcanWrite() const { return false; }
            virtual bool vcanView() const { return false; }
            virtual void vflush() {}
            virtual void vclose() {}
            virtual u64  vgetLength() const { return 0; }
            virtual void vsetLength(u64 length) {}
            virtual s64  vsetPos(s64 pos) { return 0; }
            virtual s64  vgetPos() const { return 0; }
            virtual s64  vread(u8* buffer, s64 count) { return 0; }
            virtual s64  vview(u8 const*& buffer, s64 count) { return 0; }
            virtual s64  vwrite(const u8* buffer, s64 count) { return 0; }
        };

        static stream_nil sNullStreamImp;

        istream_t* get_nullstream() { return &sNullStreamImp; }

        stream_t::stream_t() : m_caps(0), m_filehandle(nullptr), m_offset(0), m_pimpl(&sNullStreamImp) {}

        stream_t::stream_t(const stream_t& str) : m_caps(str.m_caps), m_filehandle(str.m_filehandle), m_offset(str.m_offset), m_pimpl(str.m_pimpl) {}

        stream_t::~stream_t() {}

        bool stream_t::canRead() const { return true; }
        bool stream_t::canSeek() const { return (m_caps & STREAM_CAPS_SEEK) != 0; }
        bool stream_t::canWrite() const { return (m_caps & STREAM_CAPS_WRITE) != 0; }

        bool stream_t::isOpen() const { return m_filehandle != INVALID_FILE_HANDLE; }
        bool stream_t::isAsync() const { return (m_caps & STREAM_CAPS_ASYNC) != 0; }

        u64  stream_t::getLength() const { return m_pimpl->getLength(); }
        void stream_t::setLength(u64 length) { m_pimpl->setLength(length); }
        s64  stream_t::getPos() const { return m_offset; }
        s64  stream_t::setPos(s64 pos) { return m_pimpl->setPos(pos); }

        void stream_t::close() { m_pimpl->close(); }

        void stream_t::flush() { m_pimpl->flush(); }

        s64 stream_t::read(u8* buffer, s64 length)
        {
            u64 bytesRead = m_pimpl->read(buffer, length);
            return bytesRead;
        }

        s64 stream_t::write(u8 const* buffer, s64 length)
        {
            u64 bytesWritten = m_pimpl->write(buffer, length);
            return bytesWritten;
        }

        stream_t& stream_t::operator=(const stream_t& str)
        {
            m_filehandle = str.m_filehandle;
            m_pimpl      = str.m_pimpl;
            m_caps       = str.m_caps;
            m_offset     = str.m_offset;
            return *this;
        }

        stream_t::stream_t(istream_t* impl, filehandle_t* fh)
        {
            m_filehandle = fh;
            m_pimpl      = impl;
            m_caps       = 0;
            m_offset     = 0;
        }
    } // namespace nfs
}; // namespace ncore
