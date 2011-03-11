#ifndef __XFILESYSTEM_XSTREAM_H__
#define __XFILESYSTEM_XSTREAM_H__
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

		///< Callback prototype
		typedef void(*AsyncCallback)(void);

		enum ESeekOrigin
		{
			Seek_Begin, 						///< Specifies the beginning of a stream.
			Seek_Current,						///< Specifies the current position within a stream.
			Seek_End,	 						///< Specifies the end of a stream.
		};

		enum EFileMode
		{
			FileMode_CreateNew, 				///< Specifies that the operating system should create a new file. This requires FileIOPermissionAccess.write. If the file already exists, an IOException is thrown.
			FileMode_Create,					///< Specifies that the operating system should create a new file. If the file already exists, it will be overwritten. This requires FileIOPermissionAccess.write. System.IO.FileMode.Create is equivalent to requesting that if the file does not exist, use CreateNew; otherwise, use Truncate. If the file already exists but is a hidden file, an UnauthorizedAccessException is thrown.
			FileMode_Open, 						///< Specifies that the operating system should open an existing file. The ability to open the file is dependent on the value specified by FileAccess. A System.IO.FileNotFoundException is thrown if the file does not exist.
			FileMode_OpenOrCreate, 				///< Specifies that the operating system should open a file if it exists; otherwise, a new file should be created. If the file is opened with FileAccess.Read, FileIOPermissionAccess.Read is required. If the file access is FileAccess.write then FileIOPermissionAccess.write is required. If the file is opened with FileAccess.ReadWrite, both FileIOPermissionAccess.Read and FileIOPermissionAccess.write are required. If the file access is FileAccess.Append, then FileIOPermissionAccess.Append is required.
			FileMode_Truncate, 					///< Specifies that the operating system should open an existing file. Once opened, the file should be truncated so that its size is zero bytes. This requires FileIOPermissionAccess.write. Attempts to read from a file opened with Truncate cause an exception.
			FileMode_Append 					///< Opens the file if it exists and seeks to the end of the file, or creates a new file. FileMode.Append can only be used in conjunction with FileAccess.write. Attempting to seek to a position before the end of the file will throw an IOException and any attempt to read fails and throws an NotSupportedException.
		};

		enum EFileAccess
		{
			FileAccess_Read, 					///< Specifies that the operating system should create a new file. This requires FileIOPermissionAccess.write. If the file already exists, an IOException is thrown.
			FileAccess_Write,					///< Specifies that the operating system should create a new file. If the file already exists, it will be overwritten. This requires FileIOPermissionAccess.write. System.IO.FileMode.Create is equivalent to requesting that if the file does not exist, use CreateNew; otherwise, use Truncate. If the file already exists but is a hidden file, an UnauthorizedAccessException is thrown.
			FileAccess_ReadWrite, 				///< Specifies that the operating system should open an existing file. The ability to open the file is dependent on the value specified by FileAccess. A System.IO.FileNotFoundException is thrown if the file does not exist.
		};

		///< xstream object
		///< The main interface of a stream object, user deals with this object 99% of the time.
		///< The derived class xfilestream is a constructor object and can be constructed as a xstream
		///< object.
		class xstream
		{
		public:
									xstream();
									xstream(const xstream&);

			bool					canRead() const;													///< Gets a value indicating whether the current stream supports reading.
			bool					canSeek() const;													///< Gets a value indicating whether the current stream supports seeking.
			bool					canWrite() const;													///< Gets a value indicating whether the current stream supports writing.
			bool					isAsync() const;													///< Gets a value indicating whether the FileStream was opened asynchronously or synchronously.
			u64						length() const;														///< Gets the length in bytes of the stream.
			u64						position() const;													///< Gets the current position of this stream.
			void					position(u64 Pos);													///< Sets the current position of this stream.

			s64						seek(s64 offset, ESeekOrigin origin);		 						///< When overridden in a derived class, sets the position within the current stream.
			void					close(); 															///< Closes the current stream and releases any resources (such as sockets and file handles) associated with the current stream.
			void					flush();															///< When overridden in a derived class, clears all buffers for this stream and causes any buffered data to be written to the underlying device.

			void					setLength(s64 length);							 					///< When overridden in a derived class, sets the length of the current stream.

			void					read(xbyte* buffer, s32 offset, s32 count); 						///< When overridden in a derived class, reads a sequence of bytes from the current stream and advances the position within the stream by the number of bytes read.
			s32						readByte();										 					///< Reads a byte from the stream and advances the position within the stream by one byte, or returns -1 if at the end of the stream.
			void					write(const xbyte* buffer, s32 offset, s32 count);					///< When overridden in a derived class, writes a sequence of bytes to the current stream and advances the current position within this stream by the number of bytes written.
			void					writeByte(xbyte value);							 					///< Writes a byte to the current position in the stream and advances the position within the stream by one byte.

			xasync_result			beginRead(xbyte* buffer, s32 offset, s32 count, AsyncCallback callback);  		///< Begins an asynchronous read operation.
			void					endRead(xasync_result& asyncResult);											///< Waits for the pending asynchronous read to complete.
			xasync_result			beginWrite(const xbyte* buffer, s32 offset, s32 count, AsyncCallback callback);	///< Begins an asynchronous write operation.
			void					endWrite(xasync_result& asyncResult);											///< Ends an asynchronous write operation.

			void					copyTo(xstream& dst);												///< Reads the bytes from the current stream and writes them to the destination stream.
			void					copyTo(xstream& dst, s32 count);									///< Reads all the bytes from the current stream and writes them to a destination stream, using a specified buffer size.

			xstream&				operator = (const xstream&);
			bool					operator == (const xstream&);
			bool					operator != (const xstream&);

		protected:
									xstream(xistream*);

			xistream*				mImplementation;
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
// END __XFILESYSTEM_XFILESTREAM_H__
//==============================================================================
#endif
