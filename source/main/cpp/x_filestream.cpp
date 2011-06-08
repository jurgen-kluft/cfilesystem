//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase\x_target.h"
#include "xbase\x_debug.h"
#include "xbase\x_limits.h"
#include "xbase\x_string_std.h"
#include "xbase\x_va_list.h"
#include "xbase\x_bit_field.h"

#include "xfilesystem\x_filesystem.h"
#include "xfilesystem\x_filestream.h"
#include "xfilesystem\x_filepath.h"
#include "xfilesystem\x_stream.h"
#include "xfilesystem\x_istream.h"

#include "xfilesystem\private\x_filesystem_common.h"

//==============================================================================
// xcore namespace
//==============================================================================
namespace xcore
{
	namespace xfilesystem
	{
		//------------------------------------------------------------------------------------------

		class xifilestream : public xistream
		{
			s32						mRefCount;
			u32						mFileHandle;
			s64						mFilePos;

			s64						mFilePosAfterAsyncOp;

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
			x_bitfield<ECaps>		mCaps;

		public:
									xifilestream()
										: mRefCount(2)
										, mFileHandle(INVALID_FILE_HANDLE)
										, mFilePos(0)
										, mFilePosAfterAsyncOp(0)
									{
									}

									xifilestream(const xfilepath& filename, EFileMode mode, EFileAccess access, EFileOp op);
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
			virtual void			setPosition(u64 Pos);												///< Sets the current position of this stream. (Overrides xstream.Position.)

			virtual u64				seek(s64 offset, ESeekOrigin origin);			 					///< When overridden in a derived class, sets the position within the current stream.
			virtual void			close(); 															///< Closes the current stream and releases any resources (such as sockets and file handles) associated with the current stream.
			virtual void			flush();															///< When overridden in a derived class, clears all buffers for this stream and causes any buffered data to be written to the underlying device.

			virtual u64				read(xbyte* buffer, u64 offset, u64 count);		 					///< When overridden in a derived class, reads a sequence of bytes from the current stream and advances the position within the stream by the number of bytes read.
			virtual s32				readByte();											 				///< Reads a byte from the stream and advances the position within the stream by one byte, or returns -1 if at the end of the stream.
			virtual u64				write(const xbyte* buffer, u64 offset, u64 count);					///< When overridden in a derived class, writes a sequence of bytes to the current stream and advances the current position within this stream by the number of bytes written.
			virtual u64				writeByte(xbyte value);								 				///< Writes a byte to the current position in the stream and advances the position within the stream by one byte.

			virtual xasync_id		beginRead(xbyte* buffer, u64 offset, u64 count, AsyncCallback callback);	  	///< Begins an asynchronous read operation.
			virtual void			endRead(xasync_id& asyncResult);												///< Waits for the pending asynchronous read to complete.
			virtual xasync_id		beginWrite(const xbyte* buffer, u64 offset, u64 count, AsyncCallback callback);	///< Begins an asynchronous write operation.
			virtual void			endWrite(xasync_id& asyncResult);												///< Ends an asynchronous write operation.

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

		xfilestream::xfilestream(const xfilepath& filename, EFileMode mode, EFileAccess access, EFileOp op)
		{
			mImplementation = new xifilestream(filename, mode, access, op);
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

		xifilestream::xifilestream(const xfilepath& filename, EFileMode mode, EFileAccess access, EFileOp op)
			: mRefCount(1)
			, mFileHandle(INVALID_FILE_HANDLE)
			, mFilePos(0)
			, mCaps(NONE)
		{
			bool can_read, can_write, can_seek, can_async;
			xfilesystem::caps(filename, can_read, can_write, can_seek, can_async);
			mCaps.set(CAN_WRITE, can_write);
			mCaps.set(CAN_READ, can_read);
			mCaps.set(CAN_SEEK, can_seek);
			mCaps.set(CAN_ASYNC, can_async);

			mCaps.set(USE_READ, true);
			mCaps.set(USE_SEEK, can_seek);
			mCaps.set(USE_WRITE, can_write && ((access&FileAccess_Write)!=0));
			mCaps.set(USE_ASYNC, can_async && (op == FileOp_Async));

			switch(mode)
			{
				case FileMode_CreateNew:
					{
						if (mCaps.isSet(CAN_WRITE))
						{
							if (xfilesystem::exists(filename.c_str()) == xFALSE)
							{
								mFileHandle = xfilesystem::open(filename.c_str(), mCaps.isSet(USE_READ), mCaps.isSet(USE_WRITE), mCaps.isSet(USE_ASYNC));
							}
						}
					} break;
				case FileMode_Create:
					{
						if (mCaps.isSet(CAN_WRITE))
						{
							if (xfilesystem::exists(filename.c_str()) == xTRUE)
							{
								mFileHandle = xfilesystem::open(filename.c_str(), mCaps.isSet(USE_READ), mCaps.isSet(USE_WRITE), mCaps.isSet(USE_ASYNC));
								xfilesystem::setLength(mFileHandle, 0);
							}
							else
							{
								mFileHandle = xfilesystem::open(filename.c_str(), mCaps.isSet(USE_READ), mCaps.isSet(USE_WRITE), mCaps.isSet(USE_ASYNC));
							}
						}
					} break;
				case FileMode_Open:
					{
						if (xfilesystem::exists(filename.c_str()) == xTRUE)
						{
							mFileHandle = xfilesystem::open(filename.c_str(), mCaps.isSet(USE_READ), mCaps.isSet(USE_WRITE), mCaps.isSet(USE_ASYNC));
						}
					} break;
				case FileMode_OpenOrCreate:
					{
						mFileHandle = xfilesystem::open(filename.c_str(), mCaps.isSet(USE_READ), mCaps.isSet(USE_WRITE), mCaps.isSet(USE_ASYNC));
					} break;
				case FileMode_Truncate:
					{
						if (mCaps.isSet(CAN_WRITE))
						{
							if (xfilesystem::exists(filename.c_str()) == xTRUE)
							{
								mFileHandle = xfilesystem::open(filename.c_str(), mCaps.isSet(USE_READ), mCaps.isSet(USE_WRITE), mCaps.isSet(USE_ASYNC));
								if (mFileHandle != INVALID_FILE_HANDLE)
								{
									xfilesystem::setLength(mFileHandle, 0);
								}
							}
						}
					} break;
				case FileMode_Append:
					{
						if (mCaps.isSet(CAN_WRITE))
						{
							if (xfilesystem::exists(filename.c_str()) == xTRUE)
							{
								mFileHandle = xfilesystem::open(filename.c_str(), mCaps.isSet(USE_READ), mCaps.isSet(USE_WRITE), mCaps.isSet(USE_ASYNC));
								if (mFileHandle != INVALID_FILE_HANDLE)
								{
									mCaps.set(USE_READ, false);
									mCaps.set(USE_SEEK, false);
									mCaps.set(USE_WRITE, true);

									mFilePos = xfilesystem::getLength(mFileHandle);
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
			return mCaps.isSet(CAN_READ);
		}

		bool			xifilestream::canSeek() const
		{
			return mCaps.isSet(CAN_SEEK);
		}

		bool			xifilestream::canWrite() const
		{
			return mCaps.isSet(CAN_WRITE);
		}

		bool			xifilestream::isOpen() const
		{
			return mFileHandle != INVALID_FILE_HANDLE;
		}

		bool			xifilestream::isAsync() const
		{
			return mCaps.isSet(USE_ASYNC);
		}

		u64				xifilestream::getLength() const
		{
			return xfilesystem::getLength(mFileHandle);
		}
		
		void			xifilestream::setLength(u64 length)
		{
			xfilesystem::setLength(mFileHandle, length);
		}

		u64				xifilestream::getPosition() const
		{
			return (u64)mFilePos;
		}

		void			xifilestream::setPosition(u64 Pos)
		{
			mFilePos = Pos;
		}

		u64				xifilestream::seek(s64 offset, ESeekOrigin origin)
		{
			if (mCaps.isSet(USE_SEEK))
			{
				switch (origin)
				{
					case Seek_Begin: mFilePos = offset; break;
					case Seek_Current: mFilePos += offset; break;
					case Seek_End: mFilePos = (s64)getLength() + offset; break;
				}
				
				if (mFilePos < 0)
					mFilePos = 0;

				return (u64)mFilePos;
			}
			return 0;
		}

		void			xifilestream::close()
		{
			if (mFileHandle != INVALID_FILE_HANDLE)
			{
				xfilesystem::syncClose(mFileHandle);
			}
			mFilePos = X_S64_MIN;
		}

		void			xifilestream::flush()
		{
		}

		u64				xifilestream::read(xbyte* buffer, u64 offset, u64 count)
		{
			u64 n = xfilesystem::syncRead(mFileHandle, mFilePos, count, &buffer[offset]);
			if (n != X_U64_MAX)
				mFilePos += n;
			return n;
		}

		s32				xifilestream::readByte()
		{
			xbyte data[4];
			u64 n = xfilesystem::syncRead(mFileHandle, mFilePos, 1, data);
			if (n != X_U64_MAX)
				mFilePos += n;
			return (s32)data[0];
		}

		u64				xifilestream::write(const xbyte* buffer, u64 offset, u64 count)
		{
			u64 n = xfilesystem::syncWrite(mFileHandle, mFilePos, count, &buffer[offset]);
			if (n != X_U64_MAX)
				mFilePos += n;
			return n;
		}

		u64				xifilestream::writeByte(xbyte value)
		{
			xbyte data[4];
			data[0] = value;
			u64 n = xfilesystem::syncWrite(mFileHandle, mFilePos, 1, data);
			if (n != X_U64_MAX)
				mFilePos += n;
			return n;
		}

		xasync_id		xifilestream::beginRead(xbyte* buffer, u64 offset, u64 count, AsyncCallback callback)
		{
			xasync_id async;
			xfilesystem::asyncRead(mFileHandle, mFilePos, count, &buffer[offset], async);
			mFilePosAfterAsyncOp = mFilePos + count;
			return async;
		}

		void			xifilestream::endRead(xasync_id& asyncResult)
		{
			//asyncResult.waitUntilCompleted();
			mFilePos = mFilePosAfterAsyncOp;
		}

		xasync_id		xifilestream::beginWrite(const xbyte* buffer, u64 offset, u64 count, AsyncCallback callback)
		{
			xasync_id async;
			xfilesystem::asyncWrite(mFileHandle, mFilePos, count, &buffer[offset], async);
			mFilePosAfterAsyncOp = mFilePos + count;
			return async;
		}

		void			xifilestream::endWrite(xasync_id& asyncResult)
		{
			//asyncResult.waitUntilCompleted();
			mFilePos = mFilePosAfterAsyncOp;
		}

		void			xifilestream::copyTo(xistream* dst)
		{
			
		}

		void			xifilestream::copyTo(xistream* dst, u64 count)
		{
		}

	};

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};
