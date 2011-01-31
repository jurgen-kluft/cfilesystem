#ifndef __X_FILESYSTEM_H__
#define __X_FILESYSTEM_H__
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
// xCore namespace
//==============================================================================
namespace xcore
{
	//==============================================================================
	// FORWARD DECLARES
	//==============================================================================
	class xstring_buffer;
	class xdatetime;

	//==============================================================================
	// CLASSES
	//==============================================================================

	namespace xfilesystem
	{
		extern void*	xfilesystem_heap_alloc(s32 size, s32 alignment);
		extern void		xfilesystem_heap_free(void* mem);

#if defined(TARGET_PS3) || defined(TARGET_360) || defined(TARGET_PC)

		typedef u64						uintfs;

		typedef void (*AsyncQueueCallBack)	(u32 nHandle, s32 nID);
		typedef void (*AsyncQueueCallBack2) (u32 nHandle, void* pClass, s32 nID);
		typedef void (*AsyncQueueCallBack3) (u32 nHandle, void *pClass, s32 nID, void *pAddress, uintfs uSize);

#elif defined(TARGET_PSP) || defined(TARGET_WII)

		typedef u32						uintfs;

		typedef void (*AsyncQueueCallBack)  (u32 nHandle, s32 nID);
		typedef void (*AsyncQueueCallBack2) (u32 nHandle, void *pClass, s32 nID);
		typedef void (*AsyncQueueCallBack3) (u32 nHandle, void *pClass, s32 nID, void *pAddress, uintfs uSize);

#endif

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

		enum ELoadFlags
		{
			LOAD_FLAGS_MAIN			= 0x00,
			LOAD_FLAGS_VRAM			= 0x01,
			LOAD_FLAGS_FROM_END		= 0x02,
			LOAD_FLAGS_CACHE		= 0x04,

			LOAD_FLAGS_DEFAULT		= LOAD_FLAGS_MAIN,
		};

		enum ESourceType
		{	
			FS_SOURCE_UNDEFINED		= 0,
			FS_SOURCE_HOST			,
			FS_SOURCE_BDVD			,
			FS_SOURCE_DVD			,
			FS_SOURCE_UMD			,
			FS_SOURCE_HDD			,
			FS_SOURCE_MS			,
			FS_SOURCE_CACHE			,
			FS_SOURCE_REMOTE		,
			FS_SOURCE_USB			,

			FS_SOURCE_DEFAULT		= FS_SOURCE_HOST,
		};

		enum EFilePriority
		{
			FS_PRIORITY_HIGH		= 0,
			FS_PRIORITY_MEDIUM		= 1,
			FS_PRIORITY_LOW			= 2,
			FS_PRIORITY_COUNT,
		};

		enum EFileHandle
		{
			PENDING_FILE_HANDLE		= -2,
			INVALID_FILE_HANDLE		= -1,
		};

		enum ESeekMode
		{
			FS_SEEK_ORIGIN = 1,
			FS_SEEK_CURRENT = 2,
			FS_SEEK_END     = 3,
		};

		enum EError
		{
			FILE_ERROR_OK,
			FILE_ERROR_NO_FILE,						///< File does not exist
			FILE_ERROR_BADF,						///< File descriptor is invalid  
			FILE_ERROR_PRIORITY,					///< Specified priority is invalid  
			FILE_ERROR_MAX_FILES,					///< Exceeded the maximum number of files that can be handled simultaneously  
			FILE_ERROR_MAX_ASYNC,					///< Exceeded the maximum number of async operations that can be scheduled 
			FILE_ERROR_DEVICE,						///< Specified device does not exist  
			FILE_ERROR_DEVICE_READONLY,				///< Specified device does not exist  
			FILE_ERROR_UNSUP,						///< Unsupported function for this device  
			FILE_ERROR_CANNOT_MOUNT,				///< Could not be mounted  
			FILE_ERROR_ASYNC_BUSY,					///< Asynchronous operation has not completed  
			FILE_ERROR_NOASYNC,						///< No asynchronous operation has been performed  
			FILE_ERROR_NOCWD,						///< Current directory does not exist  
			FILE_ERROR_NAMETOOLONG,					///< Filename is too long  
		};

		enum ESearchFlags
		{
			FS_SEARCH_FLAG_LINK	= 0x00000001,
			FS_SEARCH_FLAG_DIR	= 0x00000002,
			FS_SEARCH_FLAG_FILE	= 0x00000004,

			FS_SEARCH_FLAG_ALL = FS_SEARCH_FLAG_LINK | FS_SEARCH_FLAG_DIR | FS_SEARCH_FLAG_FILE,
		};

		struct SearchData
		{
			char	szFindResult[FS_MAX_PATH];
			char	szFindFull[FS_MAX_PATH];

			char	szFindPath[FS_MAX_PATH];
			char	szLastFilename[FS_MAX_PATH];
			u32		uSearchId;
			u32		uSearchFlags;
		};


		extern void 			Initialise			( u32 uAsyncQueueSize, xbool boEnableCache = false );
		extern void				Shutdown			( void );

		extern void				Update				( void );
		extern void 			WaitUntilIdle		( void );

		extern u32				Open				( const char* szName, xbool boWrite = false, xbool boRetry = false );
		extern uintfs			Size				( u32 uHandle );
		extern void				Read				( u32 uHandle, uintfs uOffset, uintfs uSize, void* pBuffer, xbool boRetry = false );	
		extern void				Write				( u32 uHandle, uintfs uOffset, uintfs uSize, const void* pBuffer, xbool boRetry = false );
		extern void 			Close				( u32& uHandle );
		extern void				Delete				( u32& uHandle );

		extern void*			Load				( const char* szName, uintfs* puSize, const u32 uFlags );
		extern void*			LoadAligned			( const u32 uAlignment, const char* szName, uintfs* puSize, const u32 uFlags );
		extern void				Save				( const char* szName, const void* pData, uintfs uSize );

		extern xbool			AsyncDone			( const u32 uHandle );
		extern u32				AsyncPreOpen		( const char* szFilename, xbool boWrite );
		extern u32				AsyncOpen			( const char* szName, xbool boWrite = false, xbool boRetry = false );
		extern void				AsyncOpen			( const u32 uHandle );
		extern void				AsyncRead			( const u32 uHandle, uintfs uOffset, uintfs uSize, void* pBuffer, xbool boRetry = false );
		extern void				AsyncWrite			( const u32 uHandle, uintfs uOffset, uintfs uSize, const void* pBuffer, xbool boRetry = false );
		extern void				AsyncClose			( const u32 uHandle );
		extern void				AsyncDelete			( const u32 uHandle );

		extern void				GetOpenCreatedTime	( u32 uHandle, xdatetime& pTimeAndDate );
		extern void				GetOpenModifiedTime ( u32 uHandle, xdatetime& pTimeAndDate );

		extern xbool			AsyncQueueOpen		( const u32 nHandle, const u32 uPriority, AsyncQueueCallBack callbackFunc, s32 nCallbackID );
		extern xbool			AsyncQueueOpen		( const u32 nHandle, const u32 uPriority, AsyncQueueCallBack2 callbackFunc, void* pClass, s32 nCallbackID );

		extern xbool			AsyncQueueClose		( const u32 nHandle, const u32 uPriority, AsyncQueueCallBack callbackFunc, s32 nCallbackID );
		extern xbool			AsyncQueueClose		( const u32 nHandle, const u32 uPriority, AsyncQueueCallBack2 callbackFunc, void* pClass, s32 nCallbackID );

		extern xbool			AsyncQueueRead		( const u32 nHandle, const u32 uPriority, uintfs uOffset, uintfs uSize, void* pDest, AsyncQueueCallBack callbackFunc, s32 nCallbackID );
		extern xbool			AsyncQueueRead		( const u32 nHandle, const u32 uPriority, uintfs uOffset, uintfs uSize, void* pDest, AsyncQueueCallBack2 callbackFunc, void* pClass, s32 nCallbackID );	
		extern xbool			AsyncQueueRead		( const u32 nHandle, const u32 uPriority, uintfs uOffset, uintfs nSize, void *pDest, AsyncQueueCallBack3 callbackFunc, void *pClass, s32 nCallbackID );	

		extern xbool			AsyncQueueWrite		( const u32 nHandle, const u32 uPriority, uintfs uOffset, uintfs uSize, const void* pSrc, AsyncQueueCallBack callbackFunc, s32 nCallbackID );
		extern xbool			AsyncQueueWrite		( const u32 nHandle, const u32 uPriority, uintfs uOffset, uintfs uSize, const void* pSrc, AsyncQueueCallBack2 callbackFunc, void* pClass, s32 nCallbackID );	

		extern xbool			AsyncQueueReadAndCache (const u32 nHandle, const u32 uPriority, uintfs uOffset, uintfs uSize, void* pDest, AsyncQueueCallBack callbackFunc, s32 nCallbackID);
		extern xbool			AsyncQueueReadAndCache (const u32 nHandle, const u32 uPriority, uintfs uOffset, uintfs uSize, void* pDest, AsyncQueueCallBack2 callbackFunc2, void* pClass, s32 nCallbackID);

		extern xbool			AsyncQueueDelete	( const u32 nHandle, const u32 uPriority, AsyncQueueCallBack callbackFunc, s32 nCallbackID );
		extern xbool			AsyncQueueDelete	( const u32 nHandle, const u32 uPriority, AsyncQueueCallBack2 callbackFunc, void* pClass, s32 nCallbackID );

		extern void				AsyncQueueCancel	( const u32 nHandle );
		extern void				AsyncQueueCancel	( const u32 nHandle, void* pClass );	
		extern void				AsyncQueueCancel	( void *pClass );

		extern xbool			AsyncQueueDone		( const u32 nHandle );

		extern xbool			DoesFileExist		( const char* szName );
		extern void				ReSize				( u32 uHandle, u64 uNewSize );
		extern u64				GetFreeSize			( const char* szPath );

		extern void				CreateFileList		( const char* szPath, xbool boRecursive );
		extern void				DestroyFileList		( void );
		extern s32				GetFileListLength	( void );
		extern const char*		GetFileListData		( u32 nFile );

		extern xbool			SearchGetFirst		( SearchData& rSearch, const char* szDir, u32 uFlags = FS_SEARCH_FLAG_ALL );
		extern xbool			SearchGetNext		( SearchData& rSearch );
		extern void				SearchClose			( SearchData& rSearch );

		extern xbool			HasLastError		( void );
		extern void				ClearLastError		( void );
		extern EError			GetLastError		( void );
		extern const char*		GetLastErrorStr		( void );

	};

	//==============================================================================
	// END xCore namespace
	//==============================================================================
};

//==============================================================================
// END __X_FILESYSTEM_H__
//==============================================================================
#endif