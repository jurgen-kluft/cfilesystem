#ifndef __X_FILESYSTEM_COMMON_H__
#define __X_FILESYSTEM_COMMON_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase\x_types.h"

#include "xfilesystem\x_filesystem.h"
#include "xfilesystem\private\x_filesystem_constants.h"
#include "xfilesystem\x_stream.h"

//==============================================================================
// xcore namespace
//==============================================================================
namespace xcore
{
	class xcstring;
	class xdatetime;

	//==============================================================================
	// xfilesystem common, private
	//==============================================================================
	namespace xfilesystem
	{
		//==============================================================================
		// FORWARD DECLARES
		//==============================================================================
		class xfiledevice;
		class xfilepath;
		struct xfiledata;
		struct xfileasync;
		class xiasync_result;
		class xio_thread;
		class xevent_factory;

		//////////////////////////////////////////////////////////////////////////
		// Common xfilesystem functionality
		//////////////////////////////////////////////////////////////////////////

		///< Synchronous file operations
		extern xbool			exists				( const char* szFilename );
		extern u64				getLength			( u32 uHandle );
		extern void				setLength			( u32 uHandle, u64 uFileSize );
		extern xbool			caps				( const xfilepath& szFilename, bool& can_read, bool& can_write, bool& can_seek, bool& can_async );

		extern u32				open				( const char* szFilename, xbool boRead = true, xbool boWrite = false, x_asyncio_callback_struct callback = x_asyncio_callback_struct() );
		extern u64				read				( u32 uHandle, u64 uOffset, u64 uSize, void* pBuffer, x_asyncio_callback_struct callback = x_asyncio_callback_struct() );	
		extern u64				write				( u32 uHandle, u64 uOffset, u64 uSize, const void* pBuffer, x_asyncio_callback_struct callback = x_asyncio_callback_struct() );
		extern u64				getpos				( u32 uHandle );
		extern u64				setpos				( u32 uHandle, u64 uPos );
		extern void 			close				( u32& uHandle, x_asyncio_callback_struct callback = x_asyncio_callback_struct() );
		extern void				closeAndDelete		( u32& uHandle, x_asyncio_callback_struct callback = x_asyncio_callback_struct() );
		extern void				save				( const char* szFilename, const void* pData, u64 uSize );
		extern u32				create				( const char* szFilename, xbool boRead = true, xbool boWrite = false);

		extern xbool			hasLastError		( void );
		extern void				clearLastError		( void );
		extern EError			getLastError		( void );
		extern const char*		getLastErrorStr		( void );

		//////////////////////////////////////////////////////////////////////////
		// Private xfilesystem functionality
		//////////////////////////////////////////////////////////////////////////
		
		extern void				setAllocator		( x_iallocator* allocator );
		extern void*			heapAlloc			( s32 size, s32 alignment );
		extern void				heapFree			( void* mem );

		extern void 			initialise			( u32 uMaxOpenStreams );
		extern void				shutdown			( void );

		///< Threading
		extern void				setIoThreadInterface( xio_thread* io_thread );
		extern xio_thread*		getIoThreadInterface( void );

		///< Common
		extern void				initialiseCommon	( u32 uMaxOpenStreams );
		extern void				shutdownCommon		( void );

		extern xfiledevice*		createSystemPath	( const char* szFilename, xcstring& outFilename );

		extern xfiledata*		getFileInfo			( u32 uHandle );
		extern u32				popFreeFileSlot		( void );
		extern bool				pushFreeFileSlot	( u32 uHandle );

		extern xfileasync*		getAsyncIOData		( u32 nSlot );
		extern xfileasync*		popFreeAsyncIO		( bool wait = false );
		extern void				pushFreeAsyncIO		( xfileasync* asyncIOInfo );

		extern xfileasync*		popAsyncIO			( void );
		extern xasync_id		pushAsyncIO			( xfileasync* asyncIOInfo );
		extern u32				testAsyncId			( xasync_id id );


		extern xbool			isPathUNIXStyle		( void );					///< UNIX = '/', Win32 = '\'

		///< Error
		extern void				setLastError		( EError error );
	};

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

//==============================================================================
// END __X_FILESYSTEM_COMMON_H__
//==============================================================================
#endif