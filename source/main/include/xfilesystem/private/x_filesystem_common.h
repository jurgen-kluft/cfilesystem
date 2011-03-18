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
#include "xfilesystem\private\x_filesystem_private.h"

#include "xfilesystem\private\x_filesystem_spsc_queue.h"
#include "xfilesystem\private\x_filesystem_llist.h"

//==============================================================================
// xcore namespace
//==============================================================================
namespace xcore
{
	//==============================================================================
	// xfilesystem common, private
	//==============================================================================
	namespace xfilesystem
	{
		//==============================================================================
		// FORWARD DECLARES
		//==============================================================================
		class xfiledevice;
		struct xfileinfo;
		struct xfileasync;
		class xthreading;

		class xfilecache;

		enum ESettings
		{
			FS_SYNC_WAIT				= 0x00,
			FS_SYNC_NOWAIT				= 0x01,
			FS_MAX_PATH					= 256,
			FS_MAX_OPENED_FILES 		= 32,
			FS_MAX_ASYNC_IO_OPS			= 32,
			FS_MAX_ASYNC_QUEUE_ITEMS	= 64,
			FS_MAX_ERROR_ITEMS	 		= 32,
			FS_MEM_ALIGNMENT			= 128,
		};


		//////////////////////////////////////////////////////////////////////////
		// Private xfilesystem functionality
		//////////////////////////////////////////////////////////////////////////
		
		extern void				setAllocator		( x_iallocator* allocator );
		extern void*			heapAlloc			( s32 size, s32 alignment );
		extern void				heapFree			( void* mem );

		extern void 			initialise			( u32 uAsyncQueueSize, xbool boEnableCache = false );
		extern void				shutdown			( void );

		///< Alias
		extern void				initAlias			( void );
		extern void				exitAlias			( void );

		///< Threading
		extern void				setIoThread			( xthreading* io_thread );
		extern xthreading*		getIoThread			( void );

		///< Common
		extern void				initialiseCommon	( u32 uAsyncQueueSize, xbool boEnableCache );
		extern void				shutdownCommon		( void );

		extern xfiledevice*		createSystemPath	( const char* szFilename, char* outFilename, s32 inFilenameMaxLen );
		extern const char*		getFileExtension	( const char* szFilename );

		extern xfileinfo*		getFileInfo			( u32 uHandle );
		extern u32				findFreeFileSlot	( void );

		extern xfileasync*		getAsyncIOData		( u32 nSlot );
		extern s32				asyncIONumFreeSlots	( void );
		extern xfileasync*		freeAsyncIOPop		( void );
		extern void				freeAsyncIOAddToTail( xfileasync* asyncIOInfo );
		extern xfileasync*		asyncIORemoveHead	( void );
		extern void				asyncIOAddToTail	( xfileasync* asyncIOInfo );

		extern xbool			sync				( u32 uFlag = FS_SYNC_WAIT );

		extern void				parseDir			( const char* szDir, xbool boRecursive, u32& ruFileList, char** pszFileList );

		extern void				createFileCache		( void );
		extern void				destroyFileCache	( void );
		extern xfilecache*		getFileCache		( );

		extern u32				findFileHandle		( const char* szName );
		extern xbool			isPathUNIXStyle		( void );					///< UNIX = '/', Win32 = '\'

		///< Error
		extern void				setLastError		( EError error );

		///< Async worker
		extern void				asyncIOWorkerResume	( void );

		///< Synchronous file operations
		extern u32				syncOpen			( const char* szName, xbool boRead = true, xbool boWrite = false );
		extern u64				syncSize			( u32 uHandle );
		extern void				syncRead			( u32 uHandle, u64 uOffset, u64 uSize, void* pBuffer );	
		extern void				syncWrite			( u32 uHandle, u64 uOffset, u64 uSize, const void* pBuffer );
		extern void 			syncClose			( u32& uHandle );
		extern void				syncDelete			( u32& uHandle );

	};

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

//==============================================================================
// END __X_FILESYSTEM_COMMON_H__
//==============================================================================
#endif