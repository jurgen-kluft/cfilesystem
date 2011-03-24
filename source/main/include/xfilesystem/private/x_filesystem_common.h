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

#include "xfilesystem\private\x_filesystem_spsc_queue.h"
#include "xfilesystem\private\x_filesystem_llist.h"

//==============================================================================
// xcore namespace
//==============================================================================
namespace xcore
{
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
		class xthreading;
		class xfilecache;
		class xiasync_result;

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

		#define XFILESYSTEM_OBJECT_NEW_DELETE()																					\
			void*	operator new(xcore::xsize_t num_bytes)				{ return heapAlloc(num_bytes, X_ALIGNMENT_DEFAULT); }	\
			void*	operator new(xcore::xsize_t num_bytes, void* mem)	{ return mem; }											\
			void	operator delete(void* mem)							{ heapFree(mem); }										\
			void	operator delete(void* mem, void* )					{ }						

		enum ELoadFlags
		{
			LOAD_FLAGS_MAIN				= 0x00,
			LOAD_FLAGS_VRAM				= 0x01,
			LOAD_FLAGS_FROM_END			= 0x02,
			LOAD_FLAGS_CACHE			= 0x04,

			LOAD_FLAGS_DEFAULT			= LOAD_FLAGS_MAIN,
		};

		enum EFilePriority
		{
			FS_PRIORITY_HIGH			= 0,
			FS_PRIORITY_MEDIUM			= 1,
			FS_PRIORITY_LOW				= 2,
			FS_PRIORITY_COUNT,
		};

		enum EFileHandle
		{
			PENDING_FILE_HANDLE			= -2,
			INVALID_FILE_HANDLE			= -1,
		};

		enum EError
		{
			FILE_ERROR_OK,
			FILE_ERROR_NO_FILE,			///< File does not exist
			FILE_ERROR_BADF,			///< File descriptor is invalid  
			FILE_ERROR_PRIORITY,		///< Specified priority is invalid  
			FILE_ERROR_MAX_FILES,		///< Exceeded the maximum number of files that can be handled simultaneously  
			FILE_ERROR_MAX_ASYNC,		///< Exceeded the maximum number of async operations that can be scheduled 
			FILE_ERROR_DEVICE,			///< Specified device does not exist  
			FILE_ERROR_DEVICE_READONLY,	///< Specified device does not exist  
			FILE_ERROR_UNSUP,			///< Unsupported function for this device  
			FILE_ERROR_CANNOT_MOUNT,	///< Could not be mounted  
			FILE_ERROR_ASYNC_BUSY,		///< Asynchronous operation has not completed  
			FILE_ERROR_NOASYNC,			///< No asynchronous operation has been performed  
			FILE_ERROR_NOCWD,			///< Current directory does not exist  
			FILE_ERROR_NAMETOOLONG,		///< Filename is too long  
		};

		//////////////////////////////////////////////////////////////////////////
		// Common xfilesystem functionality
		//////////////////////////////////////////////////////////////////////////

		///< Synchronous file operations
		extern xbool			exists				( const char* szFilename );
		extern u32				open				( const char* szFilename, xbool boRead = true, xbool boWrite = false, xbool boAsync = false );
		extern xbool			caps				( const xfilepath& szFilename, bool& can_read, bool& can_write, bool& can_seek, bool& can_async );
		extern void				seek				( u32 uHandle, u64 uOffset );
		extern u64				getLength			( u32 uHandle );
		extern void				setLength			( u32 uHandle, u64 uNewSize );
		extern void				read				( u32 uHandle, u64 uOffset, u64 uSize, void* pBuffer );	
		extern void				write				( u32 uHandle, u64 uOffset, u64 uSize, const void* pBuffer );
		extern void 			close				( u32& uHandle );
		extern void				closeAndDelete		( u32& uHandle );

		extern void*			load				( const char* szFilename, u64* puSize, const u32 uFlags );
		extern void*			loadAligned			( const u32 uAlignment, const char* szFilename, u64* puSize, const u32 uFlags );
		extern void				save				( const char* szFilename, const void* pData, u64 uSize );

		///< Asynchronous file operations
		extern u32				asyncPreOpen		( const char* szFilename, xbool boRead = true, xbool boWrite = false );
		extern u32				asyncOpen			( const char* szFilename, xbool boRead = true, xbool boWrite = false );
		extern void				asyncOpen			( const u32 uHandle );
		extern void				asyncRead			( const u32 uHandle, u64 uOffset, u64 uSize, void* pBuffer, xiasync_result*& pAsyncResult );
		extern void				asyncWrite			( const u32 uHandle, u64 uOffset, u64 uSize, const void* pBuffer, xiasync_result*& pAsyncResult );
		extern void				asyncClose			( const u32 uHandle );
		extern void				asyncDelete			( const u32 uHandle );
		extern xbool			asyncDone			( const u32 uHandle );
		extern xiasync_result*	asyncResult			( const u32 uHandle );

		extern void				getOpenCreatedTime	( u32 uHandle, xdatetime& pTimeAndDate );
		extern void				getOpenModifiedTime ( u32 uHandle, xdatetime& pTimeAndDate );

		extern u64				getFreeSize			( const char* szPath );

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

		extern void 			initialise			( xbool boEnableCache = false );
		extern void				shutdown			( void );

		///< Threading
		extern void				setThreading		( xthreading* threading );
		extern xthreading*		getThreading		( void );

		///< Common
		extern void				initialiseCommon	( xbool boEnableCache );
		extern void				shutdownCommon		( void );

		extern xfiledevice*		createSystemPath	( const char* szFilename, char* outFilename, s32 inFilenameMaxLen );
		extern const char*		getFileExtension	( const char* szFilename );

		extern xfiledata*		getFileInfo			( u32 uHandle );
		extern u32				findFreeFileSlot	( void );

		extern xfileasync*		getAsyncIOData		( u32 nSlot );
		extern s32				asyncIONumFreeSlots	( void );
		extern xfileasync*		freeAsyncIOPop		( void );
		extern void				freeAsyncIOAddToTail( xfileasync* asyncIOInfo );
		extern xfileasync*		asyncIORemoveHead	( void );
		extern void				asyncIOAddToTail	( xfileasync* asyncIOInfo );

		extern xbool			sync				( u32 uFlag = FS_SYNC_WAIT );

		extern void				createFileCache		( void );
		extern void				destroyFileCache	( void );
		extern xfilecache*		getFileCache		( );

		extern u32				findFileHandle		( const char* szFilename );
		extern xbool			isPathUNIXStyle		( void );					///< UNIX = '/', Win32 = '\'

		///< Error
		extern void				setLastError		( EError error );

		///< Synchronous file operations
		extern u32				syncOpen			( const char* szFilename, xbool boRead = true, xbool boWrite = false );
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