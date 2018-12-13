//==============================================================================
// INCLUDES
//==============================================================================
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
#include "xfilesystem/private/x_istream.h"

//==============================================================================
// xcore namespace
//==============================================================================
namespace xcore
{
    class xifilestream : public xistream
    {
        xfiledevice* mFs;
        s32          mRefCount;
        void*        mFileHandle;
        u64          mOffset;

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
        xbits<u32> mCaps;

    public:
        xifilestream()
            : mRefCount(2)
            , mFileHandle(INVALID_FILE_HANDLE)
            , mCaps(0)
        {
        }

        xifilestream(const xfilepath& filename, EFileMode mode, EFileAccess access, EFileOp op);
        ~xifilestream(void);

        virtual void hold() { mRefCount++; }
        virtual s32  release()
        {
            --mRefCount;
            return mRefCount;
        }
        virtual void destroy()
        {
            close();
            delete this;
        }

        virtual bool canRead() const;
        virtual bool canSeek() const;
        virtual bool canWrite() const;
        virtual bool isOpen() const;
        virtual bool isAsync() const;
        virtual u64  getLength() const;
        virtual void setLength(u64 length);

        virtual u64  seek(s64 offset, ESeekOrigin origin);
        virtual void close();
        virtual void flush();

        virtual u64 read(xbyte* buffer, u64 count);
        virtual u64 write(const xbyte* buffer, u64 count);

        virtual bool beginRead(xbyte* buffer, u64 count);
        virtual s64  endRead(bool block);
        virtual bool beginWrite(const xbyte* buffer, u64 count);
        virtual s64  endWrite(bool block);

        virtual void copyTo(xistream* dst);
        virtual void copyTo(xistream* dst, u64 count);
    };

    // ---------------------------------------------------------------------------------------------

    xifilestream::xifilestream(const xfilepath& filename, EFileMode mode, EFileAccess access, EFileOp op)
        : mRefCount(1)
        , mFileHandle(INVALID_FILE_HANDLE)
        , mCaps(NONE)
    {
        bool can_read, can_write, can_seek, can_async;
        // mFs->caps(filename, can_read, can_write, can_seek, can_async);
        can_read  = true;
        can_write = true;
        can_seek  = true;
        can_async = true;
        mCaps.set(CAN_WRITE, can_write);
        mCaps.set(CAN_READ, can_read);
        mCaps.set(CAN_SEEK, can_seek);
        mCaps.set(CAN_ASYNC, can_async);

        mCaps.set(USE_READ, true);
        mCaps.set(USE_SEEK, can_seek);
        mCaps.set(USE_WRITE, can_write && ((access & FileAccess_Write) != 0));
        mCaps.set(USE_ASYNC, can_async && (op == FileOp_Async));

        switch (mode)
        {
            case FileMode_CreateNew:
            {
                if (mCaps.is_set(CAN_WRITE))
                {
                    if (mFs->hasFile(filename) == xFALSE)
                    {
                        mFs->openFile(filename, mCaps.is_set(USE_READ), mCaps.is_set(USE_WRITE), mFileHandle);
                        mFs->setLengthOfFile(mFileHandle, 0);
                    }
                }
            }
            break;
            case FileMode_Create:
            {
                if (mCaps.is_set(CAN_WRITE))
                {
                    if (mFs->hasFile(filename) == xTRUE)
                    {
                        mFs->openFile(filename, mCaps.is_set(USE_READ), mCaps.is_set(USE_WRITE), mFileHandle);
                        mFs->setLengthOfFile(mFileHandle, 0);
                    }
                    else
                    {
                        mFs->openFile(filename, mCaps.is_set(USE_READ), mCaps.is_set(USE_WRITE), mFileHandle);
                        mFs->setLengthOfFile(mFileHandle, 0);
                    }
                }
            }
            break;
            case FileMode_Open:
            {
                if (mFs->hasFile(filename) == xTRUE)
                {
                    mFs->openFile(filename, mCaps.is_set(USE_READ), mCaps.is_set(USE_WRITE), mFileHandle);
                }
                else
                {
                    mFileHandle = INVALID_FILE_HANDLE;
                }
            }
            break;
            case FileMode_OpenOrCreate:
            {
                {
                    if (mFs->hasFile(filename) == xTRUE)
                    {
                        mFs->openFile(filename, mCaps.is_set(USE_READ), mCaps.is_set(USE_WRITE), mFileHandle);
                        mFs->setLengthOfFile(mFileHandle, 0);
                    }
                    else
                    {
                        // mFileHandle = xfilesystem::open(filename, mCaps.is_set(USE_READ), mCaps.is_set(USE_WRITE), NULL);
                        mFs->createFile(filename, mCaps.is_set(USE_READ), mCaps.is_set(USE_WRITE), mFileHandle);
                        mFs->setLengthOfFile(mFileHandle, 0);
                    }
                }
            }
            break;
            case FileMode_Truncate:
            {
                if (mCaps.is_set(CAN_WRITE))
                {
                    if (mFs->hasFile(filename) == xTRUE)
                    {
                        mFs->openFile(filename, mCaps.is_set(USE_READ), mCaps.is_set(USE_WRITE), mFileHandle);
                        if (mFileHandle != INVALID_FILE_HANDLE)
                        {
                            mFs->setLengthOfFile(mFileHandle, 0);
                        }
                    }
                }
            }
            break;
            case FileMode_Append:
            {
                if (mCaps.is_set(CAN_WRITE))
                {
                    if (mFs->hasFile(filename) == xTRUE)
                    {
                        mFs->openFile(filename, mCaps.is_set(USE_READ), mCaps.is_set(USE_WRITE), mFileHandle);
                        if (mFileHandle != INVALID_FILE_HANDLE)
                        {
                            mCaps.set(USE_READ, false);
                            mCaps.set(USE_SEEK, false);
                            mCaps.set(USE_WRITE, true);
                        }
                    }
                }
            }
            break;
        }
    }

    xifilestream::~xifilestream(void) {}

    bool xifilestream::canRead() const { return mCaps.is_set((CAN_READ | USE_READ)); }

    bool xifilestream::canSeek() const { return mCaps.is_set((CAN_SEEK | USE_SEEK)); }

    bool xifilestream::canWrite() const { return mCaps.is_set((CAN_WRITE | USE_WRITE)); }

    bool xifilestream::isOpen() const { return mFileHandle != INVALID_FILE_HANDLE; }

    bool xifilestream::isAsync() const { return mCaps.is_set((CAN_ASYNC | USE_ASYNC)); }

    u64 xifilestream::getLength() const
    {
        u64 length;
        if (mFs->getLengthOfFile(mFileHandle, length))
            return length;
        return 0;
    }

    void xifilestream::setLength(u64 length) { mFs->setLengthOfFile(mFileHandle, length); }

    u64 xifilestream::seek(s64 offset, ESeekOrigin origin)
    {
        u64 offset = mOffset;
        if (mCaps.is_set(USE_SEEK))
        {
            switch (origin)
            {
                case Seek_Begin: mOffset = offset; break;
                case Seek_Current: mOffset = mOffset + offset; break;
                case Seek_End: mOffset = getLength() - offset; break;
            }
        }
        return offset;
    }

    void xifilestream::close()
    {
        if (mFileHandle != INVALID_FILE_HANDLE)
        {
            mFs->closeFile(mFileHandle);
        }
    }

    void xifilestream::flush() {}

    u64 xifilestream::read(xbyte* buffer, u64 count)
    {
        if (mCaps.is_set(USE_READ))
        {
            u64 n;
            if (mFs->readFile(mFileHandle, mOffset, buffer, count, n))
            {
                mOffset += n;
            }
            return n;
        }
        return 0;
    }

    u64 xifilestream::write(const xbyte* buffer, u64 count)
    {
        if (mCaps.is_set(USE_WRITE))
        {
            u64 n;
            if (mFs->writeFile(mFileHandle, mOffset, buffer, count, n))
            {
                mOffset += n;
            }
            return n;
        }
        return 0;
    }

    s64 xifilestream::endWrite(bool block) { return 0; }

    bool xifilestream::beginRead(xbyte* buffer, u64 count)
    {
        if (mCaps.is_set(USE_READ))
        {
            u64 n;
            if (mFs->readFile(mFileHandle, mOffset, buffer, count, n))
            {

                return true;
            }
        }
        return false;
    }

    s64 xifilestream::endRead(bool block)
    {
        // mOffset += n;
        return 0;
    }

    bool xifilestream::beginWrite(const xbyte* buffer, u64 count)
    {
        if (mCaps.is_set(USE_WRITE))
        {
            u64 n;
            if (mFs->writeFile(mFileHandle, mOffset, buffer, count, n))
            {

                return true;
            }
        }
        return false;
    }

    s64 xifilestream::endWrite(bool block)
    {
        // mOffset += n;
        return 0;
    }

    void xstream_copy(xstream* src, xstream* dst, xbuffer& buffer)
    {
        s64 streamLength = (s64)src->getLength();
        while (streamLength > 0)
        {
            u64 const r = src->read(buffer.m_mutable, buffer.m_len);
            dst->write(buffer.m_mutable, buffer.m_len);
            streamLength -= r;
        }
    }

    void xstream_copy(xstream* src, xstream* dst, u64 count) {}
};
