//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase/x_target.h"
#include "xbase/x_debug.h"
#include "xbase/x_limits.h"
#include "xbase/x_string_std.h"
#include "xbase/x_va_list.h"
#include "xbase/x_bit_field.h"

#include "xfilesystem/x_filesystem.h"
#include "xfilesystem/x_filestream.h"
#include "xfilesystem/x_filepath.h"
#include "xfilesystem/x_stream.h"
#include "xfilesystem/x_istream.h"

#include "xfilesystem/private/x_filesystem_common.h"

//==============================================================================
// xcore namespace
//==============================================================================
namespace xcore
{
	class xifilestream : public xistream
	{
		s32						mRefCount;
		s32						mFileHandle;

		enum ECaps
		{
			NONE		= 0x0000,
			CAN_READ	= 0x0001,
			CAN_SEEK	= 0x0002,
			CAN_WRITE	= 0x0004,
			CAN_ASYNC	= 0x0008,
			USE_READ	= 0x1000,
			USE_SEEK	= 0x2000,
			USE_WRITE	= 0x4000,
			USE_ASYNC	= 0x8000,
		};
		x_bitfield<u32>		mCaps;
		x_asyncio_callback_struct mCallback;

	public:
		xifilestream()
			: mRefCount(2)
			, mFileHandle(INVALID_FILE_HANDLE)
		{
		}

		xifilestream(const xfilepath& filename, EFileMode mode, EFileAccess access, EFileOp op, x_asyncio_callback_struct callback);
		~xifilestream(void);

		XFILESYSTEM_OBJECT_NEW_DELETE()

		virtual void			hold()																{ mRefCount++; }
		virtual s32				release()															{ --mRefCount; return mRefCount; }
		virtual void			destroy()															{ close(); delete this; }

		virtual bool			canRead() const;													///< Gets a value indicating whether the current stream supports reading. (Overrides xstream.CanRead.)
		virtual bool			canSeek() const;													///< Gets a value indicating whether the current stream supports seeking. (Overrides xstream.CanSeek.)
		virtual bool			canWrite() const;													///< Gets a value indicating whether the current stream supports writing. (Overrides xstream.CanWrite.)
		virtual bool			isOpen() const;														///< Gets a value indicating whether the stream is open or closed
		virtual bool			isAsync() const;													///< Gets a value indicating whether the stream was opened asynchronously or synchronously.
		virtual u64				getLength() const;													///< Gets the length in bytes of the stream. (Overrides xstream.Length.)
		virtual void			setLength(u64 length);								 				///< When overridden in a derived class, sets the length of the current stream.
		virtual u64				getPosition() const;												///< Gets the current position of this stream. (Overrides xstream.Position.)
		virtual u64				setPosition(u64 Pos);												///< Sets the current position of this stream. (Overrides xstream.Position.)

		virtual u64				seek(s64 offset, ESeekOrigin origin);			 					///< When overridden in a derived class, sets the position within the current stream.
		virtual void			close(); 															///< Closes the current stream and releases any resources (such as sockets and file handles) associated with the current stream.
		virtual void			flush();															///< When overridden in a derived class, clears all buffers for this stream and causes any buffered data to be written to the underlying device.

		virtual u64				read(xbyte* buffer, u64 offset, u64 count);		 					///< When overridden in a derived class, reads a sequence of bytes from the current stream and advances the position within the stream by the number of bytes read.
		virtual u64				readByte(xbyte& outByte);							 				///< Reads a byte from the stream and advances the position within the stream by one byte, or returns -1 if at the end of the stream.
		virtual u64				write(const xbyte* buffer, u64 offset, u64 count);					///< When overridden in a derived class, writes a sequence of bytes to the current stream and advances the current position within this stream by the number of bytes written.
		virtual u64				writeByte(xbyte value);								 				///< Writes a byte to the current position in the stream and advances the position within the stream by one byte.

		virtual bool			beginRead(xbyte* buffer, u64 offset, u64 count, x_asyncio_callback_struct callback);	  	///< Begins an asynchronous read operation.
//			virtual void			endRead(xasync_result& asyncResult);												///< Waits for the pending asynchronous read to complete.
		virtual bool			beginWrite(const xbyte* buffer, u64 offset, u64 count, x_asyncio_callback_struct callback);	///< Begins an asynchronous write operation.
//			virtual void			endWrite(xasync_result& asyncResult);												///< Ends an asynchronous write operation.

		virtual void			copyTo(xistream* dst);												///< Reads the bytes from the current stream and writes them to the destination stream.
		virtual void			copyTo(xistream* dst, u64 count);									///< Reads all the bytes from the current stream and writes them to a destination stream, using a specified buffer size.
	};

	// -------------------------- xfilestream --------------------------

	xfilestream::xfilestream() 
		: xstream()
	{
	}

	xfilestream::xfilestream(const xfilestream& other)
		: xstream(other.mImplementation)
	{
	}

	xfilestream::xfilestream(const xfilepath& filename, EFileMode mode, EFileAccess access, EFileOp op, x_asyncio_callback_struct callback)
	{
		mImplementation = new xifilestream(filename, mode, access, op, callback);
	}

	xfilestream::~xfilestream()
	{
	}

	xfilestream&			xfilestream::operator =			(const xfilestream& other)
	{
		if (this == &other)
			return *this;

		if (mImplementation->release() == 0)
			mImplementation->destroy();
		mImplementation = other.mImplementation;
		mImplementation->hold();
		return *this;
	}

	bool					xfilestream::operator ==		(const xfilestream& other) const
	{
		return other.mImplementation == mImplementation;
	}

	bool					xfilestream::operator !=		(const xfilestream& other) const
	{
		return other.mImplementation != mImplementation;
	}

	// ---------------------------------------------------------------------------------------------

	xifilestream::xifilestream(const xfilepath& filename, EFileMode mode, EFileAccess access, EFileOp op, x_asyncio_callback_struct callback)
		: mRefCount(1)
		, mFileHandle(INVALID_FILE_HANDLE)
		, mCaps(NONE)
		, mCallback(callback)
	{
		bool can_read, can_write, can_seek, can_async;
		xfilesystem::xfs_common::s_instance()->caps(filename, can_read, can_write, can_seek, can_async);
		mCaps.set(CAN_WRITE, can_write);
		mCaps.set(CAN_READ, can_read);
		mCaps.set(CAN_SEEK, can_seek);
		mCaps.set(CAN_ASYNC, can_async);

		mCaps.set(USE_READ, true);
		mCaps.set(USE_SEEK, can_seek);
		mCaps.set(USE_WRITE, can_write && ((access&FileAccess_Write)!=0));
		mCaps.set(USE_ASYNC, can_async && (op == FileOp_Async));

		// check if we use async, it is properly setup
		ASSERT(op == FileOp_Sync || callback.callback != NULL);

		switch(mode)
		{
			case FileMode_CreateNew:
				{
					if (mCaps.isSet(CAN_WRITE))
					{
						if (xfilesystem::xfs_common::s_instance()->exists(filename.c_str()) == xFALSE)
						{
							//mFileHandle = xfilesystem::open(filename.c_str(), mCaps.isSet(USE_READ), mCaps.isSet(USE_WRITE), NULL);
							mFileHandle = xfilesystem::xfs_common::s_instance()->createFile(filename.c_str(), mCaps.isSet(USE_READ), mCaps.isSet(USE_WRITE));
							xfilesystem::xfs_common::s_instance()->setLength(mFileHandle, 0);
						}
					}
				} break;
			case FileMode_Create:
				{
					if (mCaps.isSet(CAN_WRITE))
					{
						if (xfilesystem::xfs_common::s_instance()->exists(filename.c_str()) == xTRUE)
						{
							mFileHandle = xfilesystem::xfs_common::s_instance()->open(filename.c_str(), mCaps.isSet(USE_READ), mCaps.isSet(USE_WRITE));
							xfilesystem::xfs_common::s_instance()->setLength(mFileHandle, 0);
						}
						else
						{
							//mFileHandle = xfilesystem::open(filename.c_str(), mCaps.isSet(USE_READ), mCaps.isSet(USE_WRITE), NULL);
							mFileHandle = xfilesystem::xfs_common::s_instance()->createFile(filename.c_str(), mCaps.isSet(USE_READ), mCaps.isSet(USE_WRITE));
							xfilesystem::xfs_common::s_instance()->setLength(mFileHandle, 0);
						}
					}
				} break;
			case FileMode_Open:
				{
					if (xfilesystem::xfs_common::s_instance()->exists(filename.c_str()) == xTRUE)
					{
						mFileHandle = xfilesystem::xfs_common::s_instance()->open(filename.c_str(), mCaps.isSet(USE_READ), mCaps.isSet(USE_WRITE));
					}
					else
					{
						mFileHandle = INVALID_FILE_HANDLE;
					}
				} break;
			case FileMode_OpenOrCreate:
				{
					{
						if (xfilesystem::xfs_common::s_instance()->exists(filename.c_str()) == xTRUE)
						{
							mFileHandle = xfilesystem::xfs_common::s_instance()->open(filename.c_str(), mCaps.isSet(USE_READ), mCaps.isSet(USE_WRITE));
							xfilesystem::xfs_common::s_instance()->setLength(mFileHandle, 0);
						}
						else
						{
							//mFileHandle = xfilesystem::open(filename.c_str(), mCaps.isSet(USE_READ), mCaps.isSet(USE_WRITE), NULL);
							mFileHandle = xfilesystem::xfs_common::s_instance()->createFile(filename.c_str(), mCaps.isSet(USE_READ), mCaps.isSet(USE_WRITE));
							xfilesystem::xfs_common::s_instance()->setLength(mFileHandle, 0);
						}
					}
				} break;
			case FileMode_Truncate:
				{
					if (mCaps.isSet(CAN_WRITE))
					{
						if (xfilesystem::xfs_common::s_instance()->exists(filename.c_str()) == xTRUE)
						{
							mFileHandle = xfilesystem::xfs_common::s_instance()->open(filename.c_str(), mCaps.isSet(USE_READ), mCaps.isSet(USE_WRITE));
							if (mFileHandle != INVALID_FILE_HANDLE)
							{
								xfilesystem::xfs_common::s_instance()->setLength(mFileHandle, 0);
							}
						}
					}
				} break;
			case FileMode_Append:
				{
					if (mCaps.isSet(CAN_WRITE))
					{
						if (xfilesystem::xfs_common::s_instance()->exists(filename.c_str()) == xTRUE)
						{
							mFileHandle = xfilesystem::xfs_common::s_instance()->open(filename.c_str(), mCaps.isSet(USE_READ), mCaps.isSet(USE_WRITE));
							if (mFileHandle != INVALID_FILE_HANDLE)
							{
								mCaps.set(USE_READ, false);
								mCaps.set(USE_SEEK, true);
								mCaps.set(USE_WRITE, true);
							}
						}
					}
				} break;
		}
	}

	xifilestream::~xifilestream(void)
	{
	}

	bool			xifilestream::canRead() const
	{
		return mCaps.isSet( (CAN_READ|USE_READ) );
	}

	bool			xifilestream::canSeek() const
	{
		return mCaps.isSet((CAN_SEEK|USE_SEEK));
	}

	bool			xifilestream::canWrite() const
	{
		return mCaps.isSet((CAN_WRITE|USE_WRITE));
	}

	bool			xifilestream::isOpen() const
	{
		return mFileHandle != INVALID_FILE_HANDLE;
	}

	bool			xifilestream::isAsync() const
	{
		return mCaps.isSet((CAN_ASYNC|USE_ASYNC));
	}

	u64				xifilestream::getLength() const
	{
		return xfilesystem::xfs_common::s_instance()->getLength(mFileHandle);
	}
	
	void			xifilestream::setLength(u64 length)
	{
		xfilesystem::xfs_common::s_instance()->setLength(mFileHandle, length);
	}

	u64				xifilestream::getPosition() const
	{
		return (u64)xfilesystem::xfs_common::s_instance()->getpos(mFileHandle);
	}

	u64				xifilestream::setPosition(u64 Pos)
	{
		return xfilesystem::xfs_common::s_instance()->setpos(mFileHandle, Pos);
	}

	u64				xifilestream::seek(s64 offset, ESeekOrigin origin)
	{
		u64 pos=0;
		if (mCaps.isSet(USE_SEEK))
		{
			switch (origin)
			{
				case Seek_Begin  : pos=xfilesystem::xfs_common::s_instance()->setpos(mFileHandle, offset); break;
				case Seek_Current: pos=xfilesystem::xfs_common::s_instance()->setpos(mFileHandle, getPosition() + offset); break;
				case Seek_End    : pos=xfilesystem::xfs_common::s_instance()->setpos(mFileHandle, (s64)getLength() + offset); break;
			}
		}
		return pos;
	}

	void			xifilestream::close()
	{
		if (mFileHandle != INVALID_FILE_HANDLE)
		{
			xfilesystem::xfs_common::s_instance()->close(mFileHandle);
		}
	}

	void			xifilestream::flush()
	{
	}

	u64				xifilestream::read(xbyte* buffer, u64 offset, u64 count)
	{
		if (mCaps.isSet(USE_READ))
		{
			//u64 p = getPosition();
			u64 n = xfilesystem::xfs_common::s_instance()->read(mFileHandle, offset, count, buffer);
			return n;
		} else return 0;
	}

	u64				xifilestream::readByte(xbyte& outByte)
	{
		if (mCaps.isSet(USE_READ))
		{
			xbyte data[4];
			u64 p = getPosition();
			u64 n = xfilesystem::xfs_common::s_instance()->read(mFileHandle, p, 1, data);
			outByte = (s32)data[0];
			return n;
		}
		else return 0;
	}

	u64				xifilestream::write(const xbyte* buffer, u64 offset, u64 count)
	{
		if (mCaps.isSet(USE_WRITE))
		{
			//u64 p = getPosition();
			u64 n = xfilesystem::xfs_common::s_instance()->write(mFileHandle, offset, count, buffer);
			return n;
		}
		else return 0;
	}

	u64				xifilestream::writeByte(xbyte value)
	{
		if (mCaps.isSet(USE_WRITE))
		{
			xbyte data[4];
			data[0] = value;
			u64 p = getPosition();
			u64 n = xfilesystem::xfs_common::s_instance()->write(mFileHandle, p, 1, data);
			return n;
		}
		else return 0;
	}

	bool	xifilestream::beginRead(xbyte* buffer, u64 offset, u64 count, x_asyncio_callback_struct callback)
	{
		if (mCaps.isSet(USE_READ))
		{
			u64 p = 0; // call get position in the IO thread, not here. //getPosition();

			x_asyncio_callback_struct currCallback = mCallback;

			if(callback.callback != NULL)
				currCallback = callback;

			xfilesystem::xfs_common::s_instance()->read(mFileHandle, p, count, &buffer[offset], currCallback);
			return true;
		}
		else return false;
	}


	bool xifilestream::beginWrite(const xbyte* buffer, u64 offset, u64 count, x_asyncio_callback_struct callback)
	{
		if (mCaps.isSet(USE_WRITE))
		{
			// TODO: fix proper offset??
			u64 p = 0; //= getPosition(); // This is not synchronized, must do this in the io thread NOT main thread!!!!

			x_asyncio_callback_struct currCallback = mCallback;

			if(callback.callback != NULL)
				currCallback = callback;

			xfilesystem::xfs_common::s_instance()->write(mFileHandle, p, count, &buffer[offset], currCallback);
			return true;	
		}
		return false;
	}


	void			xifilestream::copyTo(xistream* dst)
	{
		u64 streamLength = getLength();
		xbyte* buffer = (xbyte*)heapAlloc(u32(streamLength) ,X_ALIGNMENT_DEFAULT);
		seek(0,Seek_Begin);
		dst->seek(0,Seek_Begin);
		read(buffer,0,streamLength);
		dst->write(buffer,0,streamLength);
		heapFree(buffer);
	}

	void			xifilestream::copyTo(xistream* dst, u64 count)
	{
		u64 streamLength = getLength();
		if (count > streamLength)
			count = streamLength;
		xbyte* buffer =(xbyte*)heapAlloc(u32(count),X_ALIGNMENT_DEFAULT);
		seek(0,Seek_Begin);
		dst->seek(0,Seek_Begin);
		read(buffer,0,count);
		dst->write(buffer,0,count);
		heapFree(buffer);
	}

};
