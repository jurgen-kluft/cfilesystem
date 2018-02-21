#ifndef __XFILESYSTEM_XSTREAM_H__
#define __XFILESYSTEM_XSTREAM_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase/x_types.h"
#include "xbase/x_debug.h"

#include "xfilesystem/x_async_result.h"
#include "xfilesystem/private/x_filesystem_constants.h"

//==============================================================================
// xcore namespace
//==============================================================================
namespace xcore
{
	namespace xfilesystem
	{
		// Forward declares
		class xfilepath;
		class xistream;



		enum ESeekOrigin
		{
			Seek_Begin, 						///< Specifies the beginning of a stream.
			Seek_Current,						///< Specifies the current position within a stream.
			Seek_End,	 						///< Specifies the end of a stream.
		};

		///< xstream object
		///< The main interface of a stream object, user deals with this object most of the time.
		///< The derived class xfilestream is a constructor object and can be constructed as a xstream
		///< object.
		class xstream
		{
		public:
								xstream();
								xstream(const xstream&);
								~xstream();

			bool				canRead() const;																///< Gets a value indicating whether the current stream supports reading.
			bool				canSeek() const;																///< Gets a value indicating whether the current stream supports seeking.
			bool				canWrite() const;																///< Gets a value indicating whether the current stream supports writing.

			bool				isOpen() const;																	///< Gets a value indicating whether the FileStream is open (false == closed)
			bool				isAsync() const;																///< Gets a value indicating whether the FileStream was opened asynchronously or synchronously.

			u64					getLength() const;																///< Gets the length in bytes of the stream.
			void				setLength(u64 length);							 								///< When overridden in a derived class, sets the length of the current stream.
			u64					getPosition() const;															///< Gets the current position of this stream.
			u64					setPosition(u64 Pos);															///< Sets the current position of this stream and returns the resulting position.

			u64					seek(s64 offset, ESeekOrigin origin);		 									///< When overridden in a derived class, sets the position within the current stream.
			void				close(); 																		///< Closes the current stream and releases any resources (such as sockets and file handles) associated with the current stream.
			void				flush();																		///< When overridden in a derived class, clears all buffers for this stream and causes any buffered data to be written to the underlying device.

			u64					read(xbyte* buffer, u64 offset, u64 count); 									///< When overridden in a derived class, reads a sequence of bytes from the current stream and advances the position within the stream by the number of bytes read.
			u64					readByte(xbyte& outByte);										 								///< Reads a byte from the stream and advances the position within the stream by one byte, or returns -1 if at the end of the stream.
			u64					write(const xbyte* buffer, u64 offset, u64 count);								///< When overridden in a derived class, writes a sequence of bytes to the current stream and advances the current position within this stream by the number of bytes written.
			u64					writeByte(xbyte inByte);							 							///< Writes a byte to the current position in the stream and advances the position within the stream by one byte.

			bool				beginRead(xbyte* buffer, u64 offset, u64 count);
			bool				beginRead(xbyte* buffer, u64 offset, u64 count, x_asyncio_callback_struct callback);  		///< Begins an asynchronous read operation.
//			bool				endRead(xasync_result& asyncResult, bool block=true);												///< Waits for the pending asynchronous read to complete.
			
			bool				beginWrite(const xbyte* buffer, u64 offset, u64 count);
			bool				beginWrite(const xbyte* buffer, u64 offset, u64 count, x_asyncio_callback_struct callback);	///< Begins an asynchronous write operation.
//			bool				endWrite(xasync_result& asyncResult, bool block=true);												///< Ends an asynchronous write operation.

			void				copyTo(xstream& dst);															///< Reads the bytes from the current stream and writes them to the destination stream.
			void				copyTo(xstream& dst, u64 count);												///< Reads all the bytes from the current stream and writes them to a destination stream, using a specified buffer size.

			xstream&			operator = (const xstream&);
			bool				operator == (const xstream&);
			bool				operator != (const xstream&);

		protected:
								xstream(xistream*);

			xistream*			mImplementation;
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
// END __XFILESYSTEM_XSTREAM_H__
//==============================================================================
#endif
