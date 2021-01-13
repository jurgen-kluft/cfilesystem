#include "xbase/x_target.h"
#include "xbase/x_debug.h"

#include "xfilesystem/x_stream.h"
#include "xfilesystem/private/x_istream.h"
#include "xfilesystem/private/x_enumerations.h"

namespace xcore
{
    class stream_nil : public istream_t
    {
    public:
        virtual ~stream_nil() {}

        virtual u64  getLength(filedevice_t* fd, filehandle_t* fh) { return 0; }
        virtual void setLength(filedevice_t* fd, filehandle_t* fh, u64 length) { }
        virtual s64  setPos(filedevice_t* fd, filehandle_t* fh, u32 caps, s64& current, s64 pos) { return current; }
        virtual void close(filedevice_t* fd, filehandle_t*& fh) { }
        virtual s64 read(filedevice_t* fd, filehandle_t* fh, u32 caps, s64& pos, xbyte* buffer, s64 count) { return 0; }
        virtual s64 write(filedevice_t* fd, filehandle_t* fh, u32 caps, s64& pos, const xbyte* buffer, s64 count) { return 0; }
    };

    static stream_nil sNullStreamImp;

    stream_t::stream_t()
    {

    }

    stream_t::stream_t(const stream_t&)
    {

    }

    stream_t::~stream_t()
    {

    }

    bool stream_t::canRead() const { return false; }
    bool stream_t::canSeek() const{ return false; }
    bool stream_t::canWrite() const{ return false; }

    bool stream_t::isOpen() const{ return false; }
    bool stream_t::isAsync() const{ return false; }

    u64  stream_t::getLength() const { return 0; }
    void stream_t::setLength(u64 length) { }
    s64  stream_t::getPos() const{ return 0; }
    s64  stream_t::setPos(s64 pos){ return 0; }

    void stream_t::close() {}
    void stream_t::flush() {}

    s64 stream_t::read(xbyte*, s64){ return 0; }
    s64 stream_t::write(xbyte const*, s64){ return 0; }

    reader_t* stream_t::get_reader(){ return 0; }
    writer_t* stream_t::get_writer(){ return 0; }

    stream_t::stream_t(istream_t* impl)
    {

    }

}; // namespace xcore
