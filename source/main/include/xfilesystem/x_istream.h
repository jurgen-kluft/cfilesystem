#ifndef __XFILESYSTEM_XISTREAM_H__
#define __XFILESYSTEM_XISTREAM_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase\x_types.h"
#include "xbase\x_debug.h"

#include "xfilesystem\x_async_result.h"
#include "xfilesystem\x_stream.h"

//==============================================================================
// xcore namespace
//==============================================================================
namespace xcore
{
	namespace xfilesystem
	{
		// Forward declares
		class xfilepath;

		///< xstream implementation interface
		///< User doesn't deal with this object, it is meant for extendability
		class xistream
		{
		public:
			virtual					~xistream(void) {}

			virtual void			hold() = 0;															///< Reference count, increment
			virtual s32				release() = 0;														///< Reference count, decrement, ==0 means it should be 'destroy'ed
			virtual void			destroy() = 0;

			virtual bool			canRead() const = 0;												///< Gets a value indicating whether the current stream supports reading.
			virtual bool			canSeek() const = 0;												///< Gets a value indicating whether the current stream supports seeking.
			virtual bool			canWrite() const = 0;												///< Gets a value indicating whether the current stream supports writing.
			virtual bool			isOpen() const = 0;													///< Gets a value indicating whether the FileStream was opened asynchronously or synchronously.
			virtual bool			isAsync() const = 0;												///< Gets a value indicating whether the FileStream was opened asynchronously or synchronously.
			virtual u64				getLength() const = 0;												///< Gets the length in bytes of the stream.
			virtual void			setLength(u64 length) = 0;							 				///< When overridden in a derived class, sets the length of the current stream.
			virtual u64				getPosition() const = 0;											///< Gets the current position of this stream.
			virtual void			setPosition(u64 Pos) = 0;											///< Sets the current position of this stream.

			virtual u64				seek(s64 offset, ESeekOrigin origin) = 0;		 					///< When overridden in a derived class, sets the position within the current stream.
			virtual void			close() = 0; 														///< Closes the current stream and releases any resources (such as sockets and file handles) associated with the current stream.
			virtual void			flush() = 0;														///< When overridden in a derived class, clears all buffers for this stream and causes any buffered data to be written to the underlying device.

			virtual u64				read(xbyte* buffer, u64 offset, u64 count) = 0; 					///< When overridden in a derived class, reads a sequence of bytes from the current stream and advances the position within the stream by the number of bytes read.
			virtual s32				readByte() = 0;										 				///< Reads a byte from the stream and advances the position within the stream by one byte, or returns -1 if at the end of the stream.
			virtual u64				write(const xbyte* buffer, u64 offset, u64 count) = 0;				///< When overridden in a derived class, writes a sequence of bytes to the current stream and advances the current position within this stream by the number of bytes written.
			virtual void			writeByte(xbyte value) = 0;							 				///< Writes a byte to the current position in the stream and advances the position within the stream by one byte.

			virtual xasync_result	beginRead(xbyte* buffer, u64 offset, u64 count, AsyncCallback callback) = 0;  		///< Begins an asynchronous read operation.
			virtual void			endRead(xasync_result& asyncResult) = 0;											///< Waits for the pending asynchronous read to complete.
			virtual xasync_result	beginWrite(const xbyte* buffer, u64 offset, u64 count, AsyncCallback callback) = 0;	///< Begins an asynchronous write operation.
			virtual void			endWrite(xasync_result& asyncResult) = 0;											///< Ends an asynchronous write operation.

			virtual void			copyTo(xistream* dst) = 0;											///< Reads the bytes from the current stream and writes them to the destination stream.
			virtual void			copyTo(xistream* dst, u64 count) = 0;								///< Reads all the bytes from the current stream and writes them to a destination stream, using a specified buffer size.
		};


		//==============================================================================
		// END xfilesystem namespace
		//==============================================================================
	};

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

//==============================================================================
// END __XFILESYSTEM_XISTREAM_H__
//==============================================================================
#endif
