#include "xbase/x_target.h"
#include "xbase/x_bit_field.h"
#include "xbase/x_debug.h"
#include "xbase/x_limits.h"
#include "xbase/x_va_list.h"

#include "xfilesystem/x_filesystem.h"
#include "xfilesystem/x_filepath.h"
#include "xfilesystem/x_stream.h"

#include "xfilesystem/private/x_enumerations.h"
#include "xfilesystem/private/x_filedevice.h"
#include "xfilesystem/private/x_filesystem.h"
#include "xfilesystem/private/x_istream.h"

namespace xcore
{
    class filestream_t : public istream_t
    {
    public:
        virtual u64  getLength(filedevice_t* fd, filehandle_t* fh);
        virtual void setLength(filedevice_t* fd, filehandle_t* fh, u64 length);
        virtual s64  setPos(filedevice_t* fd, filehandle_t* fh, u32 caps, s64& current, s64 pos);
        virtual void close(filedevice_t* fd, filehandle_t*& fh);
        virtual s64 read(filedevice_t* fd, filehandle_t* fh, u32 caps, s64& pos, xbyte* buffer, s64 count);
        virtual s64 write(filedevice_t* fd, filehandle_t* fh, u32 caps, s64& pos, const xbyte* buffer, s64 count);
    };

    enum ECaps
    {
        NONE      = 0x0000,
        CAN_READ  = 0x0001,
        CAN_SEEK  = 0x0002,
        CAN_WRITE = 0x0004,
        CAN_ASYNC = 0x0008,
        USE_READ  = 0x1000,
        USE_SEEK  = 0x2000,
        USE_WRITE = 0x4000,
        USE_ASYNC = 0x8000,
    };

    //extern istream_t* get_filestream();
    static filestream_t s_filestream;
    istream_t* get_filestream()
    {
        return &s_filestream;
    }

    // ---------------------------------------------------------------------------------------------

    void* open_filestream(filedevice_t* fd, const filepath_t& filename, EFileMode mode, EFileAccess access, EFileOp op, u32 out_caps)
    {
        bool can_read, can_write, can_seek, can_async;
        
        // fd->caps(filename, can_read, can_write, can_seek, can_async);
        can_read  = true;
        can_write = true;
        can_seek  = true;
        can_async = true;
        
        enum_t<ECaps> caps;
        caps.test_set(CAN_WRITE, can_write);
        caps.test_set(CAN_READ, can_read);
        caps.test_set(CAN_SEEK, can_seek);
        caps.test_set(CAN_ASYNC, can_async);

        caps.test_set(USE_READ, true);
        caps.test_set(USE_SEEK, can_seek);
        caps.test_set(USE_WRITE, can_write && ((access & FileAccess_Write) != 0));
        caps.test_set(USE_ASYNC, can_async && (op == FileOp_Async));

        void* handle = nullptr;
        switch (mode)
        {
        case FileMode_CreateNew:
        {
            if (caps.is_set(CAN_WRITE))
            {
                if (fd->hasFile(filename) == xFALSE)
                {
                    fd->openFile(filename, mode, access, op, handle);
                    fd->setLengthOfFile(handle, 0);
                }
            }
        }
        break;
        case FileMode_Create:
        {
            if (caps.is_set(CAN_WRITE))
            {
                if (fd->hasFile(filename) == xTRUE)
                {
                    fd->openFile(filename, mode, access, op, handle);
                    fd->setLengthOfFile(handle, 0);
                }
                else
                {
                    fd->openFile(filename, mode, access, op, handle);
                    fd->setLengthOfFile(handle, 0);
                }
            }
        }
        break;
        case FileMode_Open:
        {
            if (fd->hasFile(filename) == xTRUE)
            {
                fd->openFile(filename, mode, access, op, handle);
            }
            else
            {
                handle = INVALID_FILE_HANDLE;
            }
        }
        break;
        case FileMode_OpenOrCreate:
        {
            {
                if (fd->hasFile(filename) == xTRUE)
                {
                    fd->openFile(filename, mode, access, op, handle);
                    fd->setLengthOfFile(handle, 0);
                }
                else
                {
                    fd->openFile(filename, mode, access, op, handle);
                    fd->setLengthOfFile(handle, 0);
                }
            }
        }
        break;
        case FileMode_Truncate:
        {
            if (caps.is_set(CAN_WRITE))
            {
                if (fd->hasFile(filename) == xTRUE)
                {
                    fd->openFile(filename, mode, access, op, handle);
                    if (handle != INVALID_FILE_HANDLE)
                    {
                        fd->setLengthOfFile(handle, 0);
                    }
                }
            }
        }
        break;
        case FileMode_Append:
        {
            if (caps.is_set(CAN_WRITE))
            {
                if (fd->hasFile(filename) == xTRUE)
                {
                    fd->openFile(filename, mode, access, op, handle);
                    if (handle != INVALID_FILE_HANDLE)
                    {
                        caps.test_set(USE_READ, false);
                        caps.test_set(USE_SEEK, false);
                        caps.test_set(USE_WRITE, true);
                    }
                }
            }
        }
        break;
        }

        return handle;
    }

    u64  filestream_t::getLength(filedevice_t* fd, filehandle_t* fh)
    {
        u64 length;
        if (fd->getLengthOfFile(fh, length))
            return length;
        return 0;
    }

    void filestream_t::setLength(filedevice_t* fd, filehandle_t* fh, u64 length) { fd->setLengthOfFile(fh, length); }

    s64 filestream_t::setPos(filedevice_t* fd, filehandle_t* fh, u32 caps, s64& offset, s64 seek)
    {
        s64 old_offset = offset;
        enum_t<ECaps> ecaps(caps);
        if (ecaps.is_set(USE_SEEK))
        {
            offset = seek;
        }
        return old_offset;
    }

    void filestream_t::close(filedevice_t* fd, filehandle_t*& fh)
    {
        if (fh != INVALID_FILE_HANDLE)
        {
            fd->closeFile(fh);
        }
    }

    s64 filestream_t::read(filedevice_t* fd, filehandle_t* fh, u32 caps, s64& pos, xbyte* buffer, s64 count)
    {
        enum_t<ECaps> ecaps(caps);
        if (ecaps.is_set(USE_READ))
        {
            u64 n;
            if (fd->readFile(fh, pos, buffer, count, n))
            {
                pos += n;
            }
            return n;
        }
        return 0;
    }

    s64 filestream_t::write(filedevice_t* fd, filehandle_t* fh, u32 caps, s64& pos, const xbyte* buffer, s64 count)
    {
        enum_t<ECaps> ecaps(caps);
        if (ecaps.is_set(USE_WRITE))
        {
            u64 n;
            if (fd->writeFile(fh, pos, buffer, count, n))
            {
                pos += n;
            }
            return n;
        }
        return 0;
    }


    void xstream_copy(stream_t& src, stream_t& dst, buffer_t& buffer)
    {
        reader_t* reader = src.get_reader();
        writer_t* writer = dst.get_writer();
        s64 streamLength = (s64)src.getLength();
        while (streamLength > 0)
        {
            u64 const r = reader->read(buffer.m_mutable, buffer.m_len);
            writer->write(buffer.m_mutable, buffer.m_len);
            streamLength -= r;
        }
    }

}; // namespace xcore
