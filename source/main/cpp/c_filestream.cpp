#include "ccore/c_target.h"
#include "cbase/c_bit_field.h"
#include "ccore/c_debug.h"
#include "cbase/c_limits.h"
#include "cbase/c_va_list.h"

#include "cfilesystem/c_filesystem.h"
#include "cfilesystem/c_stream.h"

#include "cfilesystem/private/c_enumerations.h"
#include "cfilesystem/private/c_filedevice.h"
#include "cfilesystem/private/c_filesystem.h"
#include "cfilesystem/private/c_istream.h"

namespace ncore
{
    namespace nfs
    {
        class filestream_t : public istream_t
        {
        public:
            virtual u64  getLength(filedevice_t* fd, filehandle_t* fh) const;
            virtual void setLength(filedevice_t* fd, filehandle_t* fh, u64 length);
            virtual s64  setPos(filedevice_t* fd, filehandle_t* fh, u32 caps, s64& current, s64 pos);
            virtual void flush(filedevice_t* fd, filehandle_t*& fh);
            virtual void close(filedevice_t* fd, filehandle_t*& fh);

            virtual s64 read(filedevice_t* fd, filehandle_t* fh, u32 caps, s64& pos, u8* buffer, s64 count);
            virtual s64 write(filedevice_t* fd, filehandle_t* fh, u32 caps, s64& pos, const u8* buffer, s64 count);

            virtual bool vcanSeek() const { return true; }
            virtual bool vcanRead() const { return true; }
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

        // extern istream_t* get_filestream();
        static filestream_t s_filestream;
        istream_t*          get_filestream() { return &s_filestream; }

        // ---------------------------------------------------------------------------------------------

        void* open_filestream(alloc_t* a, filedevice_t* fd, const filepath_t& filename, EFileMode::Enum mode, EFileAccess::Enum access, EFileOp::Enum op, u32 out_caps)
        {
            bool can_read, can_write, can_seek, can_async;

            // fd->caps(filename, can_read, can_write, can_seek, can_async);
            can_read  = access.IsRead() || access.IsReadWrite();
            can_write = access.IsWrite() || access.IsReadWrite();
            can_seek  = true;
            can_async = true;

            enum_t<ECaps> caps;
            caps.test_set(CAN_WRITE, can_write);
            caps.test_set(CAN_READ, can_read);
            caps.test_set(CAN_SEEK, can_seek);
            caps.test_set(CAN_ASYNC, can_async);

            caps.test_set(USE_READ, true);
            caps.test_set(USE_SEEK, can_seek);
            caps.test_set(USE_WRITE, can_write);
            caps.test_set(USE_ASYNC, can_async);

            void* handle = nullptr;
            switch (mode.value)
            {
                case EFileMode::Value_CreateNew:
                {
                    if (caps.is_set(CAN_WRITE))
                    {
                        if (fd->hasFile(filename) == false)
                        {
                            fd->openFile(filename, mode, access, op, handle);
                            fd->setLengthOfFile(handle, 0);
                        }
                    }
                }
                break;
                case EFileMode::Value_Create:
                {
                    if (caps.is_set(CAN_WRITE))
                    {
                        if (fd->hasFile(filename) == true)
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
                case EFileMode::Value_Open:
                {
                    if (fd->hasFile(filename) == true)
                    {
                        fd->openFile(filename, mode, access, op, handle);
                    }
                    else
                    {
                        handle = INVALID_FILE_HANDLE;
                    }
                }
                break;
                case EFileMode::Value_OpenOrCreate:
                {
                    {
                        if (fd->hasFile(filename) == true)
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
                case EFileMode::Value_Truncate:
                {
                    if (caps.is_set(CAN_WRITE))
                    {
                        if (fd->hasFile(filename) == true)
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
                case EFileMode::Value_Append:
                {
                    if (caps.is_set(CAN_WRITE))
                    {
                        if (fd->hasFile(filename) == true)
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

        u64 filestream_t::getLength(filedevice_t* fd, filehandle_t* fh) const
        {
            u64 length = 0;
            if (fh->m_handle != INVALID_FILE_HANDLE)
            {
                if (fd->getLengthOfFile(fh->m_handle, length))
                    return length;
            }
            return 0;
        }

        void filestream_t::setLength(filedevice_t* fd, filehandle_t* fh, u64 length)
        {
            if (fh->m_handle != INVALID_FILE_HANDLE)
            {
                fd->setLengthOfFile(fh->m_handle, length);
            }
        }

        s64 filestream_t::setPos(filedevice_t* fd, filehandle_t* fh, u32 caps, s64& offset, s64 seek)
        {
            s64           old_offset = offset;
            enum_t<ECaps> ecaps(caps);
            if (ecaps.is_set(USE_SEEK))
            {
                offset = seek;
            }
            return old_offset;
        }

        void filestream_t::flush(filedevice_t* fd, filehandle_t*& fh)
        {
            if (fh->m_handle != INVALID_FILE_HANDLE)
            {
                fd->flushFile(fh->m_handle);
            }
        }

        void filestream_t::close(filedevice_t* fd, filehandle_t*& fh)
        {
            if (fh->m_handle != INVALID_FILE_HANDLE)
            {
                fd->closeFile(fh->m_handle);
            }
        }

        s64 filestream_t::read(filedevice_t* fd, filehandle_t* fh, u32 caps, s64& pos, u8* buffer, s64 count)
        {
            enum_t<ECaps> ecaps(caps);
            if (ecaps.is_set(USE_READ))
            {
                if (fh->m_handle != INVALID_FILE_HANDLE)
                {
                    u64 n;
                    if (fd->readFile(fh->m_handle, pos, buffer, count, n))
                    {
                        pos += n;
                    }
                    return n;
                }
            }
            return 0;
        }

        s64 filestream_t::write(filedevice_t* fd, filehandle_t* fh, u32 caps, s64& pos, const u8* buffer, s64 count)
        {
            enum_t<ECaps> ecaps(caps);
            if (ecaps.is_set(USE_WRITE))
            {
                if (fh->m_handle != INVALID_FILE_HANDLE)
                {
                    u64 n;
                    if (fd->writeFile(fh->m_handle, pos, buffer, count, n))
                    {
                        pos += n;
                    }
                    return n;
                }
            }
            return 0;
        }

        void stream_copy(stream_t& src, stream_t& dst, buffer_t& buffer)
        {
            s64 streamLength = (s64)src.getLength();
            while (streamLength > 0)
            {
                u64 const r = src.read(buffer.m_begin, buffer.size());
                dst.write(buffer.m_begin, buffer.size());
                streamLength -= r;
            }
        }
    } // namespace nfs
}; // namespace ncore
