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

		enum EFileQueueStatus
		{	
			FILE_QUEUE_FREE	= 0,
			FILE_QUEUE_CANCELLED,

			FILE_QUEUE_TO_OPEN,
			FILE_QUEUE_OPENING,

			FILE_QUEUE_TO_CLOSE,
			FILE_QUEUE_CLOSING,

			FILE_QUEUE_TO_READ,
			FILE_QUEUE_READING,

			FILE_QUEUE_TO_WRITE,
			FILE_QUEUE_WRITING,

			FILE_QUEUE_TO_STAT,
			FILE_QUEUE_STATING,

			FILE_QUEUE_TO_DELETE,
			FILE_QUEUE_DELETING,

			FILE_QUEUE_TO_MARKER,
			FILE_QUEUE_MARKER
		};


		//////////////////////////////////////////////////////////////////////////
		// Private xfilesystem functionality
		//////////////////////////////////////////////////////////////////////////
		class QueueItem
		{
			QueueItem*			m_pPrev;
			QueueItem*			m_pNext;

		public:
			u32					m_nHandle;
			void*				m_pDestAddr;
			const void*			m_pSrcAddr;
			uintfs				m_uOffset;
			uintfs				m_uSize;
			EFileQueueStatus	m_uStatus;

			AsyncQueueCallBack	m_CallbackFunc;
			AsyncQueueCallBack2	m_CallbackFunc2;
			AsyncQueueCallBack3	m_CallbackFunc3;

			void*				m_pCallbackClass;
			s32					m_nCallbackID;

			u32					m_uPriority;

			QueueItem*			getPrev	()										{ return m_pPrev; }
			QueueItem*			getNext	()										{ return m_pNext; }

			void				setPrev	( QueueItem* pPrev )					{ m_pPrev = pPrev; }
			void				setNext	( QueueItem* pNext )					{ m_pNext = pNext; }

			void				clear	( )
			{
				m_pPrev				= NULL;
				m_pNext				= NULL;

				m_nHandle			= 0;
				m_pDestAddr			= NULL;
				m_pSrcAddr			= NULL;
				m_uOffset			= 0;
				m_uSize				= 0;
				m_uStatus			= FILE_QUEUE_FREE;

				m_CallbackFunc		= NULL;
				m_CallbackFunc2		= NULL;
				m_CallbackFunc3		= NULL;

				m_pCallbackClass	= NULL;
				m_nCallbackID		= 0;

				m_uPriority			= FS_PRIORITY_LOW;
			}
		};

		extern void				setAllocator		( x_iallocator* allocator );
		extern void*			heapAlloc			( s32 size, s32 alignment );
		extern void				heapFree			( void* mem );

		extern void 			initialise			( u32 uAsyncQueueSize, xbool boEnableCache = false );
		extern void				shutdown			( void );

		///< Alias
		extern void				initAlias			( void );
		extern void				exitAlias			( void );

		///< Thread
		extern void				setIoThread			( x_iothread* io_thread );
		extern xbool			ioThreadLoop		( void );
		extern void				ioThreadWait		( void );
		extern void				ioThreadSignal		( void );

		///< Common
		extern void				initialiseCommon	( u32 uAsyncQueueSize, xbool boEnableCache );
		extern void				shutdownCommon		( void );

		extern xfiledevice*		createSystemPath	( const char* szFilename, char* outFilename, s32 inFilenameMaxLen );
		extern const char*		getFileExtension	( const char* szFilename );

		extern xfileinfo*		getFileInfo			( u32 uHandle );
		extern u32				findFreeFileSlot	( void );

		extern s32				asyncIONumFreeSlots	( void );
		extern xfileasync*		getAsyncIOData		( u32 nSlot );
		extern xfileasync*		freeAsyncIOPop		( void );
		extern void				freeAsyncIOAddToTail( xfileasync* asyncIOInfo );
		extern xfileasync*		asyncIORemoveHead	( void );
		extern void				asyncIOAddToTail	( xfileasync* asyncIOInfo );

		extern QueueItem*		freeAsyncQueuePop	( );
		extern void				freeAsyncQueueAddToTail( QueueItem* asyncQueueItem );
		extern QueueItem*		asyncQueueRemoveHead( u32 uPriority );
		extern void				asyncQueueAddToTail	( u32 uPriority, QueueItem* asyncQueueItem );
		extern void				asyncQueueAddToHead	( u32 uPriority, QueueItem* asyncQueueItem );
		extern xbool			asyncQueueAdd		( const EFileQueueStatus uOperation, const u32 nHandle, const u32 uPriority, uintfs uOffset, uintfs uSize, void* pDest, const void* pSrc, AsyncQueueCallBack callbackFunc, AsyncQueueCallBack2 callbackFunc2, void* pClass, s32 nCallbackID );
		extern xbool			asyncQueueUpdate	( void );

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
		extern uintfs			syncSize			( u32 uHandle );
		extern void				syncRead			( u32 uHandle, uintfs uOffset, uintfs uSize, void* pBuffer );	
		extern void				syncWrite			( u32 uHandle, uintfs uOffset, uintfs uSize, const void* pBuffer );
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