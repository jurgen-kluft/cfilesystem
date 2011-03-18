#ifndef __X_FILESYSTEM_PRIVATE_H__
#define __X_FILESYSTEM_PRIVATE_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase\x_types.h"
#include "xbase\x_debug.h"

//==============================================================================
// xcore namespace
//==============================================================================
namespace xcore
{
	//==============================================================================
	// FORWARD DECLARES
	//==============================================================================
	class xdatetime;
	class xiasync_result;
	class xfilepath;

	//==============================================================================
	// CLASSES
	//==============================================================================

	namespace xfilesystem
	{
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

		///< Synchronous file operations
		extern u32				open				( const char* szFilename, xbool boRead = true, xbool boWrite = false, xbool boAsync = false );
		extern xbool			caps				( const xfilepath& szFilename, bool& can_read, bool& can_write, bool& can_seek, bool& can_async );
		extern void				seek				( u32 uHandle, u64 uOffset );
		extern u64				size				( u32 uHandle );
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
		extern void				asyncRead			( const u32 uHandle, u64 uOffset, u64 uSize, void* pBuffer );
		extern void				asyncWrite			( const u32 uHandle, u64 uOffset, u64 uSize, const void* pBuffer );
		extern void				asyncClose			( const u32 uHandle );
		extern void				asyncDelete			( const u32 uHandle );
		extern xbool			asyncDone			( const u32 uHandle );
		extern xiasync_result*	asyncResult			( const u32 uHandle );

		extern void				getOpenCreatedTime	( u32 uHandle, xdatetime& pTimeAndDate );
		extern void				getOpenModifiedTime ( u32 uHandle, xdatetime& pTimeAndDate );

		extern xbool			doesFileExist		( const char* szFilename );
		extern void				reSize				( u32 uHandle, u64 uNewSize );
		extern u64				getFreeSize			( const char* szPath );

		extern void				createFileList		( const char* szPath, xbool boRecursive );
		extern void				destroyFileList		( void );
		extern s32				getFileListLength	( void );
		extern const char*		getFileListData		( u32 nFile );

		extern xbool			hasLastError		( void );
		extern void				clearLastError		( void );
		extern EError			getLastError		( void );
		extern const char*		getLastErrorStr		( void );

	};

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

//==============================================================================
// END __X_FILESYSTEM_PRIVATE_H__
//==============================================================================
#endif