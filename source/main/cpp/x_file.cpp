//==============================================================================
// INCLUDES
//==============================================================================
#include "../x_target.h"
#include "../x_debug.h"
#include "../x_stdio.h"
#include "../x_container.h"
#include "../x_string.h"
#include "../x_va_list.h"
#include "../x_time.h"

#include "x_filesystem.h"


//==============================================================================
// xCore namespace
//==============================================================================
namespace xcore
{
	///< Deprecated
	//==============================================================================
	// Functions
	//==============================================================================

	//------------------------------------------------------------------------------
	// Author:
	//     Tomas Arce
	// Summary:
	//     Clear member data. Set member variable 0 or NULL.
	// Arguments:
	//     void
	// Returns:
	//     void
	// Description:
	//     The function reset all member variable. Set them 0 or NULL.
	//------------------------------------------------------------------------------
	void xfile::clear(void)
	{
		mFile     = (u32)xfilesystem::INVALID_FILE_HANDLE;
		mRead     = xFALSE;
		mWrite    = xFALSE;
		mAsync    = xFALSE;
		
		cacheDestroy();
	}

	//------------------------------------------------------------------------------

	xfile::xfile(void)
		: mFile((u32)xfilesystem::INVALID_FILE_HANDLE)
		, mAsync(xFALSE)
		, mCacheWritePos(0)
		, mCacheWriteSize(0)
		, mCacheReadPos(0)
		, mCacheReadSize(0)
		, mCacheSize(0)
		, mCache(NULL)
	{
	}

	//------------------------------------------------------------------------------

	xfile::~xfile(void)
	{
		close();
	}

	// ----------------------------------------------------------------------------
	// Author:
	//     Tomas Arce
	// Summary:
	//     Open the file in the specified access mode.
	// Parameters:
	//     path :  Full path name of the file.
	//     mode :  Access mode.
	//     Access Mode:
	//     <table>
	//     Access Mode   \Description
	//     ============  ----------------------------------------------------------
	//     "r"           for read only, the file must exits. Usefull when
	//                    accessing DVDs or other read only media
	//     "r+"          for reading and writing the file must exits
	//     "w" or "w+"   for writing + reading to a file. The file will be
	//                    created. Note here that the + is automatic.
	//     "a" or "a+"   this is actually different to the standard io. This is
	//                    just "r+" with a seekEnd(0) in both cases.
	//     "@"           for asynchronous access.
	//     "c"           for writing compress files. Reading is detected
	//                    automatically.
	//     "b"           This is always the default so you don't need to put it
	//                    really. I added this just in case people like to put it,
	//                    but it is not necessary at all.
	//     "t"           To write/read text files. If you don't add this assumes
	//                    you are doing binary files. This command basically writes
	//                    an additional character '\\r' whenever it finds a '\\n'
	//                    and when reading it removes it when it finds '\\r\\n'
	//     </table>
	// Returns:
	//     Returns xTRUE if open the file successful, or xFALSE
	//     otherwise.
	// Description:
	//     This function opens the file in specified mode. If open the
	//     file successful, return xTRUE. Otherwise, return xFALSE.
	// See Also:
	//     xfile::open(const char* path, const char* mode)
	// ----------------------------------------------------------------------------
	xbool xfile::open(const char* path, const char* mode)
	{
		ASSERT(mode);

		bool bSeekToEnd = false;
		bool bRead = false;
		bool bCreate = false;
		bool bWrite = false;
		bool bCompression = false;
		bool bAsync = false;
		bool bText = false;
		for(s32 i=0; mode[i]; i++)
		{
			switch(mode[i])
			{
			case 'a':   bRead=true; bWrite=true; bSeekToEnd=true; break;         
			case 'r':   bRead=true;  break;
			case 'w':   bWrite=true; break;
			case '+':   bRead=true; bWrite=true; bCreate=true; break;         
			case 'c':   bCompression=true; break;
			case '@':   bAsync=true; break;
			case 't':   bText=true; break;
			case 'b':   bText=false; break;
			default:
				{
					xstring_tmp tmp("Don't understand this [%c] access mode while opening file (%s)", (s8)mode[i], path);
					ASSERTS(0, tmp.c_str());
				}
			}
		}

		mPos = 0;
		mRead = bRead;
		mWrite = bWrite;

		if (bAsync)
		{
			mAsync = xTRUE;
			mFile = xfilesystem::AsyncOpen(path, bWrite);
		}
		else
		{
			mFile = xfilesystem::Open(path, bWrite);
		}

		mFileLength = -1;
		if (mRead)
		{
			if (bAsync)
			{
				while (	!synchronize( xFALSE ) )
				{
				}
				mFileLength = (u32)xfilesystem::Size(mFile);
			}
			else
			{
				mFileLength = (u32)xfilesystem::Size(mFile);
			}
		}

		return mFile != (u32)xfilesystem::INVALID_FILE_HANDLE;
	}

	// -----------------------------------------------------------------
	// Author:
	//     Tomas Arce
	// Summary:
	//     Close the file.
	// Returns:
	//     void
	// Description:
	//     This function closes the file in current xfile and also clear
	//     all member data
	// See Also:
	//     xfile::clear()
	// -----------------------------------------------------------------
	void xfile::close(void)
	{
		if (mCache != NULL && mWrite)
		{
			cacheWriteFlush();
		}

		if (mFile != (u32)xfilesystem::INVALID_FILE_HANDLE)
		{
			if (mAsync)
				xfilesystem::AsyncClose(mFile);
			else
				xfilesystem::Close(mFile);

			mFile = (u32)xfilesystem::INVALID_FILE_HANDLE;
		}

		//
		// Done with the file
		//
		clear();
	}

	void xfile::enableCache(u32 cacheSize)
	{
		// Cache does not support 'read and write'
		if (mRead && mWrite)
			return;

		cacheSize = (cacheSize<4096) ? 4096 : cacheSize;
		cacheSize = x_Align(cacheSize, (u32)128);
		cacheInit(cacheSize);
	}

	//------------------------------------------------------------------------------
	// Author:
	//     Tomas Arce
	// Summary:
	//     Reads raw data from the file already open. And Starts at the position the file pointer indicates.
	// Arguments:
	//     buffer   - A pointer to the buffer that receives the data read from a file.
	//     size     - The size of the unit data.
	//     count    - The count of unit data to be read.
	// Returns:
	//     If reads file successful, return xTRUE. If reached the end of the file, return xFALSE. Otherwise,, return xTRUE.
	// Description:
	//     Reads data from a file. The number of bytes to read is size*count. The data will be saved in buffer. The position the file pointer indicates usually include both a pointer(mFile) and a offset.
	//     The offset will move during readRaw().
	// See Also:
	//     xfile::writeRaw()
	//------------------------------------------------------------------------------
	xbool xfile::readRaw(void* buffer, s32 size, s32 count)
	{
		ASSERT(mFile != xfilesystem::INVALID_FILE_HANDLE);
		ASSERT(buffer);
		ASSERT(size > 0);
		ASSERT(count >= 0);
		ASSERT(mRead);

		s32 totalCount = size*count;

		if (cacheRead(mFile, buffer, size, count) == xTRUE)
		{
			mPos += totalCount;
			return xTRUE;
		}

		if (mAsync)
			xfilesystem::AsyncRead(mFile, mPos, totalCount, buffer);
		else 
			xfilesystem::Read(mFile, mPos, totalCount, buffer);

		mPos += totalCount;
		return xTRUE;
	}

	//------------------------------------------------------------------------------
	// Author:
	//     Tomas Arce
	// Summary:
	//     Writes data to the specified file at position specified by current file pointer. (The position depends on platform write() function)
	// Arguments:
	//     buffer   - A pointer to the buffer containing the data to be written to the file.
	//     size     - The size of the unit data.
	//     count    - The count of unit data to be written.
	// Returns:
	//     If writes file successful, return xTRUE. Otherwise, return xFALSE.
	// Description:
	//     Writes data to the specified file at position specified by current file pointer. (The position depends on platform write() function) The number of bytes to be written is size*count.
	// See Also:
	//     xfile::readRaw()
	//------------------------------------------------------------------------------
	void xfile::writeRaw(const void* buffer, s32 size, s32 count)
	{
		ASSERT(mFile != xfilesystem::INVALID_FILE_HANDLE);
		ASSERT(buffer);
		ASSERT(size > 0);
		ASSERT(count >= 0);
		ASSERT(mWrite);

		s32 totalCount = size*count;
		
		if (cacheWrite(mFile, buffer, size, count) == xFALSE)
		{
		if (mAsync)
			xfilesystem::AsyncWrite(mFile, mPos, totalCount, buffer);
		else 
			xfilesystem::Write(mFile, mPos, totalCount, buffer);
		}

		mPos += totalCount;
	}

	//------------------------------------------------------------------------------
	// Author:
	//     Tomas Arce
	// Summary:
	//     Print formatted output to the standard output stream.
	// Arguments:
	//     formatStr    - is a specially formatted string which need decoding by this function.
	//                     for more reference about how the formatting works check x_sprintf.
	//                     You should never pass as a <B>NULL</B>.
	//     args         - Optional arguments needed by the Formatted string.
	// Returns:
	//     Returns the number of characters printed.
	// Description:
	//     This function is mainly intended for debug printing. Its main purpose 
	//     is to print in the console screen similar how a command pront works in windows.
	//     The behavior of this function should resemble closely of the standard printf.
	//     This function is usually overwritten by the system using x_SetFunctionPrintF.
	//     So it is not encourage for the user to overwrite its behavior. 
	// See Also:
	//     x_sprintf DefaultPrintFXY x_printfxy x_DebugMsg x_SetFunctionPrintF
	//------------------------------------------------------------------------------
	s32 xfile::printf(const char* formatStr, const x_va_list& args)
	{
		ASSERT(mFile != xfilesystem::INVALID_FILE_HANDLE);
		ASSERT(formatStr);

		xstring_tmp s(formatStr, args);
		writeRaw((const void*)s.c_str(), 1, s.getLength());
		return s.getLength();
	}


	//------------------------------------------------------------------------------
	// Author:
	//     Tomas Arce
	// Summary:
	//     Set force flush flag.
	// Arguments:
	//     inOnOff    - is a xbool. It sets the force flush flag true of false.
	// Returns:
	//     void
	// Description:
	//     Set force flush flag. If the flag is on, the record sequence should flush any internal buffers after write().
	// See Also:
	//     pc_device::flush(void* filePtr)
	//------------------------------------------------------------------------------
	void xfile::forceFlush(xbool isOnOff)
	{
		ASSERT(mFile != xfilesystem::INVALID_FILE_HANDLE);

	}

	//------------------------------------------------------------------------------
	// Author:
	//     Tomas Arce
	// Summary:
	//     Cancels all pending input and output (I/O) operations that are issued by the calling thread for the specified file. The function does not cancel I/O operations that other threads issue for a file handle.
	// Arguments:
	//     void
	// Returns:
	//     void
	// Description:
	//     If the flag ACC_ASYNC is on, marks all pending input/output (I/O) operations as canceled.
	// See Also:
	//     pc_device::asyncAbort(void* filePtr)
	//------------------------------------------------------------------------------
	void xfile::asyncAbort(void)
	{
		ASSERT(mFile != xfilesystem::INVALID_FILE_HANDLE);
		xfilesystem::AsyncQueueCancel(mFile);
	}

	//------------------------------------------------------------------------------
	// Author:
	//     Tomas Arce
	// Summary:
	//     If the flag ACC_ASYNC is on, synchronize input/output (I/O) operations.
	// Arguments:
	//     isBlock    - If this parameter is xTRUE, the function does not return until the operation has been completed. If this parameter is xFALSE and the operation is still pending, the function returns ERROR_IO_INCOMPLETE.
	// Returns:
	//     Retrieves the results of an overlapped operation.
	//     <table>
	//     Returns          \Description
	//     ============     ----------------------------------------------------------
	//     SYNC_COMPLETED   Overlapped operation completed.
	//     SYNC_EOF         We have reached the end of the file during asynchronous operation.
	//     SYNC_INCOMPLETE  There is some error during asynchronous operation.
	//     SYNC_UNKNOWN_ERR There is unknown error during asynchronous operation.
	//     </table>
	// Description:
	//     If the flag ACC_ASYNC is on, synchronize input/output (I/O) operations. Return the results of the overlapped operation.Synchronous I/O means that the method is blocked until the I/O operation is complete, and then the method returns its data.
	// See Also:
	//     pc_device::synchronize(void* filePtr, xbool isOnOff)
	//------------------------------------------------------------------------------
	xbool xfile::synchronize(xbool isBlock)
	{
		ASSERT(mFile != xfilesystem::INVALID_FILE_HANDLE);

		if (mAsync)
		{
			if (isBlock)
			{
				xfilesystem::WaitUntilIdle();
			}
			else
			{
				return xfilesystem::AsyncDone(mFile);
			}
		}
		return xTRUE;
	}

	//------------------------------------------------------------------------------
	// Author:
	//     Tomas Arce
	// Summary:
	//     Flushes the buffer.
	// Arguments:
	//     void
	// Returns:
	//     void
	// Description:
	//     The function use synchronize() to flush the buffer.
	// See Also:
	//     pc_device::flush(void* filePtr)
	//------------------------------------------------------------------------------
	void xfile::flush(void)
	{
		ASSERT(mFile != xfilesystem::INVALID_FILE_HANDLE);
	
	}

	//------------------------------------------------------------------------------
	// Author:
	//     Tomas Arce
	// Summary:
	//     Moves the file pointer(offset) of the specified file.
	// Arguments:
	//     offset    - The number of bytes to move the file pointer. A positive value moves the pointer forward in the file and a negative value moves the file pointer backward.
	// Returns:
	//     void
	// Description:
	//     The starting point for the pointer move is zero or the beginning of the file. The offset parameter is interpreted as an unsigned value.
	// See Also:
	//     seekEnd(), seekCurrent()
	//------------------------------------------------------------------------------
	void xfile::seekOrigin(s32 offset)
	{
		ASSERT(mFile != xfilesystem::INVALID_FILE_HANDLE);
		mPos = offset;
	}
	//------------------------------------------------------------------------------
	// Author:
	//     Tomas Arce
	// Summary:
	//     Moves the file pointer(offset) of the specified file.
	// Arguments:
	//     offset    - The number of bytes to move the file pointer. A positive value moves the pointer forward in the file and a negative value moves the file pointer backward.
	// Returns:
	//     void
	// Description:
	//     The starting point is the current end-of-file position.
	// See Also:
	//     seekOrigin(), seekCurrent()
	//------------------------------------------------------------------------------
	void xfile::seekEnd(s32 offset)
	{
		ASSERT(mFile != xfilesystem::INVALID_FILE_HANDLE);
		mPos = getFileLength() - offset;
		if (mPos<0)
			mPos = 0;
	}

	//------------------------------------------------------------------------------
	// Author:
	//     Tomas Arce
	// Summary:
	//     Moves the file pointer(offset) of the specified file.
	// Arguments:
	//     offset    - The number of bytes to move the file pointer. A positive value moves the pointer forward in the file and a negative value moves the file pointer backward.
	// Returns:
	//     void
	// Description:
	//     The start point is the current value of the file pointer.
	// See Also:
	//     seekOrigin(), seekEnd()
	//------------------------------------------------------------------------------
	void xfile::seekCurrent(s32 offset)
	{
		ASSERT(mFile != xfilesystem::INVALID_FILE_HANDLE);
		mPos += offset;
		if (mPos<0)
			mPos = 0;
	}

	//------------------------------------------------------------------------------
	// Author:
	//     Tomas Arce
	// Summary:
	//     Receive current position of the file pointer(offset).
	// Arguments:
	//     void
	// Returns:
	//     The current position of the file pointer.
	// Description:
	//     The current position of the file pointer is usually (file start position + offset).
	// See Also:
	//     isEof()
	//------------------------------------------------------------------------------
	s32 xfile::tell(void)
	{
		ASSERT(mFile != xfilesystem::INVALID_FILE_HANDLE);
		return mPos;
	}

	//------------------------------------------------------------------------------
	// Author:
	//     Tomas Arce
	// Summary:
	//     Tests for end of file (EOF).
	// Arguments:
	//     void
	// Returns:
	//     Returns xTRUE if the current position is end of file, or xFALSE if it is not or error.
	// Description:
	//.....The return depends on the device function isEof().
	// See Also:
	//     tell()
	//------------------------------------------------------------------------------
	xbool xfile::isEof(void)
	{
		ASSERT(mFile != xfilesystem::INVALID_FILE_HANDLE);
		return mPos >= (u32)getFileLength();
	}

	//------------------------------------------------------------------------------
	// Author:
	//     Tomas Arce
	// Summary:
	//     Read one char from file
	// Arguments:
	//     void
	// Returns:
	//     Returns the char has been read. If failed to read (Only reached the end of the file), return -1;
	// Description:
	//.....The function read one char from the position that file pointer indicates.
	// See Also:
	//     putC()
	//------------------------------------------------------------------------------
	s32 xfile::getC(void)
	{
		u8 c;
		if (read(c) == xFALSE) 
			return -1;
		return c;
	}

	//------------------------------------------------------------------------------
	// Author:
	//     Tomas Arce
	// Summary:
	//     Write a char to file.
	// Arguments:
	//     inC    - The char to be written. This is s32. And the char output is u8 which define as: u8 charOut = inC;
	//     count  - The times that the char to be written.
	//     isUpdatePos  - If it is xTRUE, the file pointer will update its offset in order to indicates a new position.
	//                    If it is xFALSE, the offset of file pointer won't change.
	// Returns:
	//     void
	// Description:
	//.....The function write "inC" for "count" times. The data will be written at position specified by current file pointer.
	// See Also:
	//     getC(), alignPutC()
	//------------------------------------------------------------------------------
	void xfile::putC(s32 inC, s32 count, xbool isUpdatePos)
	{
		s32 pos = 0;
		u8  c = (u8)inC;

		if (isUpdatePos == xFALSE) 
			pos = tell();

		for (s32 i=0; i<count; i++)
			write(c);

		if (isUpdatePos == xFALSE) 
			seekOrigin(pos);
	}

	//------------------------------------------------------------------------------
	// Author:
	//     Tomas Arce
	// Summary:
	//     Write a char for a specified times.
	// Arguments:
	//     c        - The char to be written.
	//     count    - The original times in that the char to be written.
	//     aligment - The alignment value. The real time in that the char to be written will be aligned to this value.
	//     isUpdatePos    - Whether the file pointer (the offset) update.
	// Returns:
	//     void
	// Description:
	//.....The function write a char in a aligned times.
	// See Also:
	//     getC(), putC()
	//------------------------------------------------------------------------------
	void xfile::alignPutC(s32 c, s32 count, s32 aligment, xbool isUpdatePos)
	{
		// First solve the alignment issue
		s32 pos      = tell();
		s32 putCount = x_Align(count+pos, aligment) - pos;

		// Put all the necessary characters
		putC(c, putCount, isUpdatePos);
	}

	//------------------------------------------------------------------------------
	// Author:
	//     Tomas Arce
	// Summary:
	//     Get file length
	// Arguments:
	//     void
	// Returns:
	//     Returns length of the file.
	// Description:
	//.....The function get the length of the file.
	// See Also:
	//     seekOrigin(), seekCurrent(), seekEnd()
	//------------------------------------------------------------------------------
	s32 xfile::getFileLength(void)
	{
		ASSERT(mFile != xfilesystem::INVALID_FILE_HANDLE);
		return (s32)xfilesystem::Size(mFile);
	}

	//------------------------------------------------------------------------------
	// Author:
	//     Tomas Arce
	// Summary:
	//     Gets the time value of a specified file
	// Arguments:
	//     fileName   - Name of the file which dates and times are to be retrieved. 
	//     fileTime   - TODO: Nothing for now.
	// Returns:
	//     Returns xTRUE, if get file time successful. Return xFALSE, if fails to open file.
	// Description:
	//.....If open file successful, fileTime is the time of file get by device. If fails, the fileTime is set 0 and return xFALSE.
	// See Also:
	//     seekOrigin(), seekCurrent(), seekEnd()
	//------------------------------------------------------------------------------
	xbool xfile::getFileTime(u64& fileTime)
	{
		xdatetime tad;
		xfilesystem::GetOpenModifiedTime(mFile, tad);
		fileTime=tad.ticks();
		return xTRUE;
	}

	//------------------------------------------------------------------------------
	// Author:
	//     Tomas Arce
	// Summary:
	//     Deletes an existing file.
	// Arguments:
	//     fileName   - The name of the file to be deleted. 
	// Returns:
	//     TODO: always xFALSE.
	//     If fails to open file, return xFALSE. If open file successful, the return depends on the device return of function erase().
	// Description:
	//.....The function deletes an existing file from device. If open file successful, the function will close the file and then erase the file from device. 
	// See Also:
	//     open()
	//------------------------------------------------------------------------------
	xbool xfile::deleteFile()
	{
		xfilesystem::Delete(mFile);
		return xTRUE;
	}

	//------------------------------------------------------------------------------
	// Author:
	//     Tomas Arce
	// Summary:
	//     Write specified size of data from current xfile to memory buffer.
	// Arguments:
	//     buffer  - The pointer of destination buffer that the data will be written to.
	//     bufferSize  - The size of the data read from the current xfile. This must be small than the file length.
	// Returns:
	//     void
	// Description:
	//.....The function read data from the start of the current xfile. And write the data to memory buffer.
	// See Also:
	//     toFile()
	//------------------------------------------------------------------------------
	void xfile::toMemory(void* buffer, s32 bufferSize)
	{
		// seek at the begging of the file
		seekOrigin(0);
		s32 length = getFileLength();
		ASSERT(length >= bufferSize);
		readRaw(buffer, bufferSize, 1);
	}

	//------------------------------------------------------------------------------
	// Author:
	//     Tomas Arce
	// Summary:
	//     Write a xstring to file.
	// Arguments:
	//     val    - The xstring to be written.
	// Returns:
	//     void
	// Description:
	//.....The function output an empty char '\0' at the end of xstring. The data will be written at position specified by current file pointer.
	// See Also:
	//     getC(), PutC()
	//------------------------------------------------------------------------------
	void xfile::write(const xstring& val)
	{
		for(s32 i=0; val[i]; i++)
			putC(val[i]);
		putC(0);
	}

	//------------------------------------------------------------------------------
	// Author:
	//     Tomas Arce
	// Summary:
	//     Write a xstring to the current file.
	// Arguments:
	//     val    - The xstring to be written.
	// Returns:
	//     void
	// Description:
	//.....The function output an empty char '\0' at the end of xstring. The data will be written at position specified by current file pointer.
	// See Also:
	//     write(const xstring& val), getC(), PutC()
	//------------------------------------------------------------------------------
	void xfile::write(xstring& val)
	{
		write((const xstring&) val);
	}

	//------------------------------------------------------------------------------
	// Author:
	//     Tomas Arce
	// Summary:
	//     Read a xstring from xfile
	// Arguments:
	//     val    - The data read from xfile will be written to val.
	// Returns:
	//     If reads successful, return xTRUE.
	// Description:
	//.....The function seems NOT work, for i wonder if getC() can return 0.
	// See Also:
	//     write(const xstring& val), getC(), PutC()
	//------------------------------------------------------------------------------
	// TODO: Add the fail condition
	// TODO: Where is the end of reading?
	xbool xfile::read(xstring& val)
	{
		char buffer[256];
		s32  times = 0;
		s32  i;

		val.clear();

		for(i=0; (buffer[i] = (char)getC()) != 0; i++)
		{
			if(i > 253)
			{
				buffer[255]=0;
				if(times > 0)
				{
					val.appendFormat(buffer);
				}
				else
				{
					val.format(buffer);
				}

				times++;
				i=0;
			}
		}

		ASSERT(buffer[i]==0);
		if(times > 0)
		{
			val.appendFormat(buffer);
		}
		else
		{
			val.format(buffer);
		}
		return xTRUE;
	}

	void					xfile::cacheInit		(u32 cacheSize)
	{
		mCache = (xbyte*)x_malloc(1, cacheSize, XMEM_FLAG_ALIGN_128B);
		mCacheSize = cacheSize;

		mCacheWritePos = 0;
		mCacheWriteSize = 0;
		mCacheReadPos = 0;
		mCacheReadSize = 0;
	}

	void					xfile::cacheDestroy		(void)
	{
		if (mCache != NULL)
			x_free(mCache);
		mCache = NULL;
		mCacheSize = 0;

		mCacheWritePos = 0;
		mCacheWriteSize = 0;
		mCacheReadPos = 0;
		mCacheReadSize = 0;
	}

	xbool					xfile::cacheRead		(u32 handle, void* buffer, s32 size, s32 count)
	{
		if (mCache == NULL)
			return xFALSE;

		const u32 totalCount = size*count;
		if (mPos>=mCacheReadPos && ((mPos+totalCount) <= (mCacheReadPos + mCacheReadSize)))
		{
			x_memcpy(buffer, mCache + mPos-mCacheReadPos, totalCount);
		}
		else
		{
			if ((u32)totalCount > (mCacheSize / 4))
				return xFALSE;

			// Clamp the read against the file length
			u32 readSize = mCacheSize;
			if ((mPos + readSize) > mFileLength)
				readSize = mFileLength - mPos;

			xfilesystem::Read(mFile, mPos, readSize, mCache);

			mCacheReadPos = mPos;
			mCacheReadSize = readSize;

			x_memcpy(buffer, mCache + mPos-mCacheReadPos, totalCount);
		}

		return xTRUE;
	}

	xbool					xfile::cacheWrite		(u32 handle, const void* buffer, s32 size, s32 count)
	{
		if (mCache == NULL)
			return xFALSE;

		// Put the cache at the position of the first write
		if (mCacheWriteSize==0)
			mCacheWritePos = mPos;

		const u32 totalCount = size*count;
		if (mPos>=mCacheWritePos && ((mPos+totalCount) < (mCacheWritePos + mCacheSize)))
		{
			x_memcpy(mCache + mPos-mCacheWritePos, buffer, totalCount);

			if (((mPos+totalCount)-mCacheWritePos) > mCacheWriteSize)
				mCacheWriteSize = ((mPos+totalCount)-mCacheWritePos);
		}
		else
		{
			// Flush the cache to disk
			cacheWriteFlush();

			mCacheWritePos = mPos;
			if (totalCount < mCacheSize)
			{
				x_memcpy(mCache, buffer, totalCount);
				mCacheWriteSize = totalCount;
			}
			else
			{
				return xFALSE;
			}
		}
		return xTRUE;
	}

	void					xfile::cacheWriteFlush()
	{
		if (mCacheWriteSize > 0)
		{
			xfilesystem::Write(mFile, mCacheWritePos, mCacheWriteSize, mCache);
			mCacheWriteSize = 0;
		}
	}

//==============================================================================
// END xCore namespace
//==============================================================================
};
