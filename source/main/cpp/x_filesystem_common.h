#ifndef _X_FILESYSTEM_COMMON_H__
#define __X_FILESYSTEM_COMMON_H__
#include "..\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

//==============================================================================
// INCLUDES
//==============================================================================
#include "..\x_types.h"
#include "..\x_llist.h"

#include "x_filesystem.h"

//==============================================================================
// xCore namespace
//==============================================================================
namespace xcore
{
	//==============================================================================
	// FORWARD DECLARES
	//==============================================================================
	class xstring_buffer;

	//==============================================================================
	// xfilecache
	//==============================================================================
	namespace xfilesystem
	{
		class xfilecache;

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

		enum EFileOpStatus
		{
			FILE_OP_STATUS_FREE,

			FILE_OP_STATUS_OPEN_PENDING,
			FILE_OP_STATUS_OPENING,

			FILE_OP_STATUS_CLOSE_PENDING,
			FILE_OP_STATUS_CLOSING,

			FILE_OP_STATUS_READ_PENDING,
			FILE_OP_STATUS_READING,

			FILE_OP_STATUS_WRITE_PENDING,
			FILE_OP_STATUS_WRITING,

			FILE_OP_STATUS_STAT_PENDING,
			FILE_OP_STATUS_STATING,

			FILE_OP_STATUS_DELETE_PENDING,
			FILE_OP_STATUS_DELETING,

			FILE_OP_STATUS_DONE,
		};

		struct FileInfo
		{
			u64 				m_uByteOffset;
			u64 				m_uByteSize;

			u64 				m_uSectorOffset;
			u64 				m_uNumSectors;
			u64 				m_uSectorSize;

			s32 				m_nFileIndex;
			s32 				m_nLastError;

			char				m_szFilename[FS_MAX_PATH];
			xbool				m_boWriting;
			xbool				m_boWaitAsync;
			ESourceType			m_eSource;

			// Async IO worker thread will write here:
			volatile u32 		m_nFileHandle;

			void				clear()
			{
				m_uByteOffset		= 0;
				m_uByteSize			= 0;

				m_uSectorOffset		= 0;
				m_uNumSectors		= 0;
				m_uSectorSize		= 0;

				m_nFileIndex		= 0;
				m_nFileHandle		= (u32)INVALID_FILE_HANDLE;
				m_nLastError		= 0;

				m_szFilename[0] = '\0';
				m_szFilename[FS_MAX_PATH - 1] = '\0';

				m_boWriting			= false;
				m_boWaitAsync		= false;
				m_eSource			= FS_SOURCE_DEFAULT;
			}
		};



		//////////////////////////////////////////////////////////////////////////
		// Private xfilesystem functionality
		//////////////////////////////////////////////////////////////////////////
		namespace __private
		{
			struct AsyncIOInfo
			{
				AsyncIOInfo*		m_pPrev;
				AsyncIOInfo*		m_pNext;

				s32					m_nFileIndex;
				s32					m_nStatus;
				const void*			m_pWriteAddress;
				void*				m_pReadAddress;

				u64					m_uReadWriteOffset;
				u64					m_uReadWriteSize;

				AsyncIOInfo*		getPrev	()										{ return m_pPrev; }
				AsyncIOInfo*		getNext	()										{ return m_pNext; }

				void				setPrev	( AsyncIOInfo* pPrev )					{ m_pPrev = pPrev; }
				void				setNext	( AsyncIOInfo* pNext )					{ m_pNext = pNext; }

				void				clear()
				{
					m_pPrev			= NULL;
					m_pNext			= NULL;

					m_nFileIndex	= -1;
					m_nStatus		= FILE_OP_STATUS_FREE;
					
					m_pWriteAddress	= NULL;
					m_pReadAddress	= NULL;

					m_uReadWriteOffset	= 0;
					m_uReadWriteSize	= 0;
				}
			};

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

			///< Common
			extern void				InitialiseCommon	( u32 uAsyncQueueSize, xbool boEnableCache );
			extern void				ShutdownCommon		( void );

			extern ESourceType		CreateSystemPath	( const char* szFilename, xstring_buffer& outFilename );
			extern const char*		GetFileExtension	( const char* szFilename );
			extern xbool			IsSourceReadonly	( ESourceType eSource );

			extern FileInfo*		GetFileInfo			( u32 uHandle );
			extern u32				FindFreeFileSlot	( void );

			extern s32				AsyncIONumActiveSlots( void );
			extern s32				AsyncIONumFreeSlots	( void );
			extern AsyncIOInfo*		GetAsyncIOData		( u32 nSlot );
			extern AsyncIOInfo*		FreeAsyncIOPop		( void );
			extern void				FreeAsyncIOAddToTail( AsyncIOInfo* asyncIOInfo );
			extern AsyncIOInfo*		AsyncIORemoveHead	( void );
			extern void				AsyncIOAddToTail	( AsyncIOInfo* asyncIOInfo );

			extern QueueItem*		FreeAsyncQueuePop	( );
			extern void				FreeAsyncQueueAddToTail( QueueItem* asyncQueueItem );
			extern QueueItem*		AsyncQueueRemoveHead( u32 uPriority );
			extern void				AsyncQueueAddToTail	( u32 uPriority, QueueItem* asyncQueueItem );
			extern void				AsyncQueueAddToHead	( u32 uPriority, QueueItem* asyncQueueItem );
			extern xbool			AsyncQueueAdd		( const EFileQueueStatus uOperation, const u32 nHandle, const u32 uPriority, uintfs uOffset, uintfs uSize, void* pDest, const void* pSrc, AsyncQueueCallBack callbackFunc, AsyncQueueCallBack2 callbackFunc2, void* pClass, s32 nCallbackID );
			extern xbool			AsyncQueueUpdate	( void );

			extern xbool			Sync				( u32 uFlag = FS_SYNC_WAIT );

			extern void				ParseDir			( const char* szDir, xbool boRecursive, u32& ruFileList, char** pszFileList );

			extern void				CreateFileCache		( void );
			extern void				DestroyFileCache	( void );
			extern xfilecache*		GetFileCache		( );

			extern u32				FindFileHandle		( const char* szName );
			extern xbool			IsPathUNIXStyle		( void );					///< UNIX = '/', Win32 = '\'

			///< Error
			extern void				SetLastError		( EError error );

			///< Async worker
			extern void				AsyncIOWorkerResume	( void );

			///< Synchronous file operations
			extern u32				SyncOpen			( const char* szName, xbool boWrite = false, xbool boRetry = false );
			extern uintfs			SyncSize			( u32 uHandle );
			extern void				SyncRead			( u32 uHandle, uintfs uOffset, uintfs uSize, void* pBuffer, xbool boRetry = false );	
			extern void				SyncWrite			( u32 uHandle, uintfs uOffset, uintfs uSize, const void* pBuffer, xbool boRetry = false );
			extern void 			SyncClose			( u32& uHandle );
			extern void				SyncDelete			( u32& uHandle );

		};

	};


	//==============================================================================
	// xfilecache
	//==============================================================================
	namespace xfilesystem
	{

		//--------
		// Defines
		//--------
		enum ECacheSettings
		{
			CACHED_FILE_NAME_LENGTH		= 256,
			MAX_CALLBACKS				= 16,
			MAX_CACHED_FILES			= 1024,
			CACHE_FILE_ALIGNMENT		= 2048
		};

		#define	CACHE_PATH					"cache:\\"
		#define	CACHE_FILENAME				CACHE_PATH "DataCache.dat"

		//---------
		// Typedefs
		//---------
		typedef void (*CacheCallBack) (void* pClass, s32 nID);

		//------
		// Enums
		//------
		enum ECacheFileFlags
		{
			CACHE_FILE_FLAGS_INVALID	= 0x00,		// Data not set
			CACHE_FILE_FLAGS_VALID		= 0x01,		// Valid cached data
			CACHE_FILE_FLAGS_PERMANENT	= 0x02,		// Must not be automatically de-cached
			CACHE_FILE_FLAGS_BUSY		= 0x04,		// Operation in progress
		};

		//-----------
		// Data Types
		//-----------

		class xfilecache
		{
			class FileEntry
			{
			public:
				FileEntry*		m_pPrev;
				FileEntry*		m_pNext;

				char			m_szName[CACHED_FILE_NAME_LENGTH];
				u64				m_uOffset;
				u64				m_uSize;

				u32				m_uFlags;
				u32				m_uCRC;

				FileEntry*		getPrev	()											{ return m_pPrev; }
				FileEntry*		getNext	()											{ return m_pNext; }

				void			setPrev	( FileEntry* pPrev )						{ m_pPrev = pPrev; }
				void			setNext	( FileEntry* pNext )						{ m_pNext = pNext; }
			};

			class CallbackData
			{
			public:
				CacheCallBack	m_Callback;
				void*			m_pClass;
				s32				m_nID;
			};

			class CacheHeader
			{
			public:
				u32				m_uFlags;
				FileEntry		m_xCacheList[MAX_CACHED_FILES];
			};

			u32								m_nCacheHandle;
			u64								m_uCacheSize;
			CacheHeader						m_xHeader;
			xllist<FileEntry>				m_xPermanentList;
			xllist<FileEntry>				m_xTransientList;

			static	CallbackData			m_xCallbacks[MAX_CALLBACKS];

			static void	FileIOCallback		( u32 nHandle, s32 nID );

		public:
						xfilecache						();
						~xfilecache						();

			void		Initialise						();
			void		PurgeCache						();

			s32			GetCachedIndex					( const char* szFilename, u64 uOffset = 0 );
			void		GetCacheData					( s32 nIndex, FileEntry& rxFileEntry );

			bool		AddToCache						( const char* szFilename, void* pData, const u64 uOffset, const u64 uSize, const bool boPermanent );
			bool		AddToCacheAsync					( const char* szFilename, void* pData, const u64 uOffset, const u64 uSize, const bool boPermanent, CacheCallBack Callback, void* pClass, s32 nID );
			bool		SetCacheData					( const char* szFilename, void* pData, const u64 uOffset, const u64 uSize, const bool boPermanent );

			void		RemoveFromCache					( const char* szFilename );

			void		InvalidateCacheIndexFile		();
			void		InvalidateCacheIndexFileAsync	();

			void		WriteCacheIndexFile				();
			void		WriteCacheIndexFileAsync		();
		};
	};



	//==============================================================================
	// xalias
	//==============================================================================

	namespace xfilesystem
	{
		//------------------------------------------------------------------------------
		// Description:
		//     This class maps an alias to a source.
		//     This acts as an indirection and is useful for defining folders
		//     as drives, remap drives etc..
		//
		//     source:\folder\filename.ext, where source = c:\temp
		//     data:\folder\filename.ext, where data = g:\
		//     dvd:\folder\filename.ext, where dvd = g:\
		//     
		//------------------------------------------------------------------------------
		class xalias
		{
		public:
			xalias ();
			xalias (const char* alias, const char* aliasTarget);
			xalias (const char* alias, ESourceType source, const char* remap=NULL);

			const char*			alias() const								{ return mAliasStr; }
			const char*			aliasTarget() const							{ return mAliasTargetStr; }
			const char*			remap() const;
			ESourceType			source() const;

		private:
			const char*         mAliasStr;									///< data
			const char*         mAliasTargetStr;							///< d
			mutable const char* mRemapStr;									///< e.g. "d:\project\data\bin.pc", data:\file.txt to d:\project\data\bin.pc\file.txt
			ESourceType			mSource;
		};

		extern void				ExitAlias();
		extern void				AddAlias(xalias& alias);
		extern const xalias*	FindAlias(const char* alias);
		extern const xalias*	FindAliasFromFilename(const char* filename);
		extern const xalias*	FindAndRemoveAliasFromFilename(xstring_buffer& ioFilename);
		extern void				ReplaceAliasOfFilename(xstring_buffer& ioFilename, const xalias* inNewAlias);
	};

	//==============================================================================
	// END xCore namespace
	//==============================================================================
};

//==============================================================================
// END __X_FILESYSTEM_COMMON_H__
//==============================================================================
#endif