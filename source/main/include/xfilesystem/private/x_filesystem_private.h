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


#if defined(TARGET_PS3) || defined(TARGET_360) || defined(TARGET_PC)

		typedef u64						uintfs;

		typedef void (*AsyncQueueCallBack)	(u32 nHandle, s32 nID);
		typedef void (*AsyncQueueCallBack2) (u32 nHandle, void* pClass, s32 nID);
		typedef void (*AsyncQueueCallBack3) (u32 nHandle, void *pClass, s32 nID, void *pAddress, uintfs uSize);

#elif defined(TARGET_PSP) || defined(TARGET_WII) || defined(TARGET_3DS)

		typedef u32						uintfs;

		typedef void (*AsyncQueueCallBack)  (u32 nHandle, s32 nID);
		typedef void (*AsyncQueueCallBack2) (u32 nHandle, void *pClass, s32 nID);
		typedef void (*AsyncQueueCallBack3) (u32 nHandle, void *pClass, s32 nID, void *pAddress, uintfs uSize);
#else
		#error "Error: Unsupported platform for xfilesystem"
#endif

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
		extern u32				open				( const char* szName, xbool boWrite = false, xbool boRetry = false );
		extern void				seek				( u32 uHandle, uintfs uOffset );
		extern uintfs			size				( u32 uHandle );
		extern void				read				( u32 uHandle, uintfs uOffset, uintfs uSize, void* pBuffer, xbool boRetry = false );	
		extern void				write				( u32 uHandle, uintfs uOffset, uintfs uSize, const void* pBuffer, xbool boRetry = false );
		extern void 			close				( u32& uHandle );
		extern void				closeAndDelete		( u32& uHandle );

		extern void*			load				( const char* szName, uintfs* puSize, const u32 uFlags );
		extern void*			loadAligned			( const u32 uAlignment, const char* szName, uintfs* puSize, const u32 uFlags );
		extern void				save				( const char* szName, const void* pData, uintfs uSize );

		///< Asynchronous file operations
		extern xbool			asyncDone			( const u32 uHandle );
		extern u32				asyncPreOpen		( const char* szFilename, xbool boWrite );
		extern u32				asyncOpen			( const char* szName, xbool boWrite = false, xbool boRetry = false );
		extern void				asyncOpen			( const u32 uHandle );
		extern void				asyncRead			( const u32 uHandle, uintfs uOffset, uintfs uSize, void* pBuffer, xbool boRetry = false );
		extern void				asyncWrite			( const u32 uHandle, uintfs uOffset, uintfs uSize, const void* pBuffer, xbool boRetry = false );
		extern void				asyncClose			( const u32 uHandle );
		extern void				asyncDelete			( const u32 uHandle );

		extern void				getOpenCreatedTime	( u32 uHandle, xdatetime& pTimeAndDate );
		extern void				getOpenModifiedTime ( u32 uHandle, xdatetime& pTimeAndDate );

		extern xbool			asyncQueueOpen		( const u32 nHandle, const u32 uPriority, AsyncQueueCallBack callbackFunc, s32 nCallbackID );
		extern xbool			asyncQueueOpen		( const u32 nHandle, const u32 uPriority, AsyncQueueCallBack2 callbackFunc, void* pClass, s32 nCallbackID );

		///< Asynchronous queue, file operations
		extern xbool			asyncQueueClose		( const u32 nHandle, const u32 uPriority, AsyncQueueCallBack callbackFunc, s32 nCallbackID );
		extern xbool			asyncQueueClose		( const u32 nHandle, const u32 uPriority, AsyncQueueCallBack2 callbackFunc, void* pClass, s32 nCallbackID );

		extern xbool			asyncQueueRead		( const u32 nHandle, const u32 uPriority, uintfs uOffset, uintfs uSize, void* pDest, AsyncQueueCallBack callbackFunc, s32 nCallbackID );
		extern xbool			asyncQueueRead		( const u32 nHandle, const u32 uPriority, uintfs uOffset, uintfs uSize, void* pDest, AsyncQueueCallBack2 callbackFunc, void* pClass, s32 nCallbackID );	
		extern xbool			asyncQueueRead		( const u32 nHandle, const u32 uPriority, uintfs uOffset, uintfs nSize, void *pDest, AsyncQueueCallBack3 callbackFunc, void *pClass, s32 nCallbackID );	

		extern xbool			asyncQueueWrite		( const u32 nHandle, const u32 uPriority, uintfs uOffset, uintfs uSize, const void* pSrc, AsyncQueueCallBack callbackFunc, s32 nCallbackID );
		extern xbool			asyncQueueWrite		( const u32 nHandle, const u32 uPriority, uintfs uOffset, uintfs uSize, const void* pSrc, AsyncQueueCallBack2 callbackFunc, void* pClass, s32 nCallbackID );	

		extern xbool			asyncQueueReadAndCache (const u32 nHandle, const u32 uPriority, uintfs uOffset, uintfs uSize, void* pDest, AsyncQueueCallBack callbackFunc, s32 nCallbackID);
		extern xbool			asyncQueueReadAndCache (const u32 nHandle, const u32 uPriority, uintfs uOffset, uintfs uSize, void* pDest, AsyncQueueCallBack2 callbackFunc2, void* pClass, s32 nCallbackID);

		extern xbool			asyncQueueDelete	( const u32 nHandle, const u32 uPriority, AsyncQueueCallBack callbackFunc, s32 nCallbackID );
		extern xbool			asyncQueueDelete	( const u32 nHandle, const u32 uPriority, AsyncQueueCallBack2 callbackFunc, void* pClass, s32 nCallbackID );

		extern void				asyncQueueCancel	( const u32 nHandle );
		extern void				asyncQueueCancel	( const u32 nHandle, void* pClass );	
		extern void				asyncQueueCancel	( void *pClass );

		extern xbool			asyncQueueDone		( const u32 nHandle );

		extern xbool			doesFileExist		( const char* szName );
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