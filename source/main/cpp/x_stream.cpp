//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase/x_target.h"
#include "xbase/x_debug.h"

#include "xfilesystem/x_stream.h"
#include "xfilesystem/private/x_istream.h"
#include "xfilesystem/private/x_enumerations.h"

//==============================================================================
// xcore namespace
//==============================================================================
namespace xcore
{
    class xstream_nil : public xistream
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

        virtual s64  getPos() const { return 0; }
        virtual s64  setPos(s64 length) {}

        virtual void close() {}
        virtual void flush() {}

        virtual u64 read(xbyte* buffer, u64 count) { return 0; }
        virtual u64 write(const xbyte* buffer, u64 count) { return 0; }

        virtual bool beginRead(xbyte* buffer, u64 count) { return false; }
        virtual s64 endRead(bool block) { return 0; }
        virtual bool beginWrite(const xbyte* buffer, u64 count) { return false; }
        virtual s64 endWrite(bool block) { return 0; }
    };

    static xstream_nil sNullStreamImp;

    //------------------------------------------------------------------------------------------
    xstream::xstream()
        : m_pimpl(&sNullStreamImp)
    {
    }

    xstream::xstream(xistream* pimpl)
        : m_pimpl(pimpl)
    {
        pimpl->hold();
    }

    xstream::xstream(const xstream& other)
        : m_pimpl(other.m_pimpl)
    {
        m_pimpl->hold();
    }

    xstream::~xstream()
    {
        if (m_pimpl->release() == 0)
            m_pimpl->destroy();
        m_pimpl = 0;
    }

    bool xstream::canRead() const { return m_pimpl->canRead(); }

    bool xstream::canSeek() const { return m_pimpl->canSeek(); }

    bool xstream::canWrite() const { return m_pimpl->canWrite(); }

    bool xstream::isOpen() const { return m_pimpl->isOpen(); }

    bool xstream::isAsync() const { return m_pimpl->isAsync(); }

    u64 xstream::getLength() const { return m_pimpl->getLength(); }

    void xstream::setLength(u64 length) { m_pimpl->setLength(length); }

    s64 xstream::getPos() const { return m_pimpl->getPos(); }
    s64 xstream::setPos(s64 pos) { return m_pimpl->setPos(pos); }

    void xstream::close()
    {
        m_pimpl->close();

        if (m_pimpl->release() == 0)
            m_pimpl->destroy();

        m_pimpl = &sNullStreamImp;
    }

    void xstream::flush() { m_pimpl->flush(); }

    u64 xstream::read(xbyte* buffer, u64 count) { return m_pimpl->read(buffer, count); }

    u64 xstream::write(const xbyte* buffer, u64 count) { return m_pimpl->write(buffer, count); }

    bool xstream::beginRead(xbyte* buffer, u64 count) { return m_pimpl->beginRead(buffer, count); }
    s64 xstream::endRead(bool block) { return m_pimpl->endRead(block); }

    bool xstream::beginWrite(const xbyte* buffer, u64 count) { return m_pimpl->beginWrite(buffer, count); }
    s64 xstream::endWrite(bool block) { return m_pimpl->endWrite(block); }
};
