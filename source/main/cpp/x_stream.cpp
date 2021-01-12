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

}; // namespace xcore
