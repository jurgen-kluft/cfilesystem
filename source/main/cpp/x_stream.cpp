#include "xbase/x_target.h"
#include "xbase/x_debug.h"

#include "xfilesystem/x_stream.h"
#include "xfilesystem/private/x_istream.h"
#include "xfilesystem/private/x_enumerations.h"

namespace xcore
{
    class xstream_nil : public istream_t
    {
    public:
        virtual ~xstream_nil() {}

        virtual void hold() {}
        virtual s32  release() { return 1; }
        virtual void destroy() {}

        virtual bool canRead() const { return false; }
        virtual bool canSeek() const { return false; }
        virtual bool canWrite() const { return false; }
        virtual bool isOpen() const { return false; }
        virtual bool isAsync() const { return false; }
        virtual u64  getLength() const { return 0; }
        virtual void setLength(u64 length) {}

        virtual s64 getPos() const { return 0; }
        virtual s64 setPos(s64 length) { return 0; }

        virtual void close() {}
        virtual void flush() {}

        virtual u64 read(xbyte* buffer, u64 count) { return 0; }
        virtual u64 write(const xbyte* buffer, u64 count) { return 0; }

        virtual bool beginRead(xbyte* buffer, u64 count) { return false; }
        virtual s64  endRead(bool block) { return 0; }
        virtual bool beginWrite(const xbyte* buffer, u64 count) { return false; }
        virtual s64  endWrite(bool block) { return 0; }
    };

    static xstream_nil sNullStreamImp;

    //------------------------------------------------------------------------------------------
    stream_t::stream_t() : m_pimpl(&sNullStreamImp) {}
    stream_t::stream_t(istream_t* pimpl) : m_pimpl(pimpl) { pimpl->hold(); }
    stream_t::stream_t(const stream_t& other) : m_pimpl(other.m_pimpl) { m_pimpl->hold(); }
    stream_t::~stream_t()
    {
        if (m_pimpl->release() == 0)
        {
            m_pimpl->destroy();
        }
        m_pimpl = 0;
    }

    bool stream_t::canRead() const { return m_pimpl->canRead(); }
    bool stream_t::canSeek() const { return m_pimpl->canSeek(); }
    bool stream_t::canWrite() const { return m_pimpl->canWrite(); }
    bool stream_t::isOpen() const { return m_pimpl->isOpen(); }
    bool stream_t::isAsync() const { return m_pimpl->isAsync(); }

    u64  stream_t::getLength() const { return m_pimpl->getLength(); }
    void stream_t::setLength(u64 length) { m_pimpl->setLength(length); }

    s64 stream_t::getPos() const { return m_pimpl->getPos(); }
    s64 stream_t::setPos(s64 pos) { return m_pimpl->setPos(pos); }

    void stream_t::close()
    {
        m_pimpl->close();
        if (m_pimpl->release() == 0)
        {
            m_pimpl->destroy();
        }

        m_pimpl = &sNullStreamImp;
    }

    void stream_t::flush() { m_pimpl->flush(); }

    u64 stream_t::read(xbyte* buffer, u64 count) { return m_pimpl->read(buffer, count); }
    u64 stream_t::write(const xbyte* buffer, u64 count) { return m_pimpl->write(buffer, count); }

    bool stream_t::beginRead(xbyte* buffer, u64 count) { return m_pimpl->beginRead(buffer, count); }
    s64  stream_t::endRead(bool block) { return m_pimpl->endRead(block); }

    bool stream_t::beginWrite(const xbyte* buffer, u64 count) { return m_pimpl->beginWrite(buffer, count); }
    s64  stream_t::endWrite(bool block) { return m_pimpl->endWrite(block); }
}; // namespace xcore
