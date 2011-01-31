//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase\x_target.h"
#include "xbase\x_debug.h"
#include "xbase\x_string_std.h"

#include "xstring\x_string.h"

#include "xfilesystem\x_filesystem.h"
#include "xfilesystem\private\x_filesystem_common.h"
#include "xfilesystem\private\x_filesystem_spsc_queue.h"

//==============================================================================
// xCore namespace
//==============================================================================
namespace xcore
{
	//------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------
	//----------------------- xfilesystem Common Implementations -------------------------------
	//------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------
	namespace xfilesystem
	{
		using namespace __private;

		static FileInfo					m_OpenAsyncFile[FS_MAX_OPENED_FILES];
		static AsyncIOInfo				m_AsyncIOData[FS_MAX_ASYNC_IO_OPS];
		static QueueItem				m_aAsyncQueue[FS_MAX_ASYNC_QUEUE_ITEMS];

		static spsc_cqueue<QueueItem*, FS_MAX_ASYNC_QUEUE_ITEMS>	m_pAsyncQueueList[FS_PRIORITY_COUNT];
		static spsc_cqueue<QueueItem*, FS_MAX_ASYNC_QUEUE_ITEMS>	m_pFreeQueueItemList;

		static spsc_cqueue<AsyncIOInfo*, FS_MAX_ASYNC_IO_OPS>		m_pFreeAsyncIOList;
		static spsc_cqueue<AsyncIOInfo*, FS_MAX_ASYNC_IO_OPS>		m_pAsyncIOList;

		static EError					m_eLastErrorStack[FS_MAX_ERROR_ITEMS];
		static xfilecache*				m_pCache = NULL;

		//------------------------------------------------------------------------------------------

		void				Update( void )
		{
			Sync(FS_SYNC_NOWAIT);
			__private::AsyncQueueUpdate();
		}

		//------------------------------------------------------------------------------------------

		xbool				DoesFileExist( const char* szName )
		{
			u32 uHandle = __private::SyncOpen(szName, false);
			if (uHandle == (u32)INVALID_FILE_HANDLE)
				return false;
			__private::SyncClose(uHandle);
			return true;
		}

		//------------------------------------------------------------------------------------------

		u32					Open ( const char* szFilename, xbool boWrite, xbool boRetry )
		{
			return SyncOpen(szFilename, boWrite, boRetry);
		}

		//------------------------------------------------------------------------------------------

		uintfs				Size				( u32 uHandle )
		{
			return SyncSize(uHandle);
		}

		//------------------------------------------------------------------------------------------

		void				Read (u32 uHandle, uintfs uOffset, uintfs uSize, void* pBuffer, xbool boRetry)
		{
			SyncRead(uHandle, uOffset, uSize, pBuffer, boRetry);
		}

		//------------------------------------------------------------------------------------------

		void				Write( u32 uHandle, uintfs uOffset, uintfs uSize, const void* pBuffer, xbool boRetry )
		{
			SyncWrite(uHandle, uOffset, uSize, pBuffer, boRetry);
		}

		//------------------------------------------------------------------------------------------

		void				Delete( u32& uHandle )
		{
			SyncDelete(uHandle);
		}

		//------------------------------------------------------------------------------------------

		void				Close ( u32 &uHandle )
		{
			SyncClose(uHandle);
		}

		//------------------------------------------------------------------------------------------

		void*				LoadAligned (const u32 uAlignment, const char* szFilename, uintfs* puFileSize, const u32 uFlags)
		{
			u32 nFileHandle = Open(szFilename, false);
			if (nFileHandle == (u32)INVALID_FILE_HANDLE)
			{
				if (puFileSize)
					*puFileSize = 0;

				SetLastError(FILE_ERROR_NO_FILE);
				return (NULL);
			}

			FileInfo* fileInfo = GetFileInfo(nFileHandle);

			u64	u64FileSize;
			if (fileInfo->m_uNumSectors==0)
			{
				u64FileSize = fileInfo->m_uByteSize;
			}
			else
			{
				u64FileSize = fileInfo->m_uNumSectors * fileInfo->m_uSectorSize;
			}

			if (puFileSize)
				*puFileSize	= u64FileSize;

			if (uFlags & LOAD_FLAGS_VRAM)
			{
				//pHeap	= Display::GetLocalHeap();
			}
			else
			{
				//pHeap	= HeapManager::GetHeap();
			}

			void* pData;
			if (uFlags & LOAD_FLAGS_FROM_END)
			{
				// Allocate from the end of the heap
				pData = xfilesystem_heap_alloc((s32)u64FileSize, uAlignment);
			}
			else
			{
				pData = xfilesystem_heap_alloc((s32)u64FileSize, uAlignment);
			}

			Read(nFileHandle, 0, u64FileSize, pData, false);
			Close(nFileHandle);

			if(	(uFlags & LOAD_FLAGS_CACHE) && (m_pCache != NULL) && (fileInfo->m_eSource != FS_SOURCE_CACHE) )
			{
				// File Needs to be cached
				if(m_pCache->GetCachedIndex(szFilename) < 0)
				{
					m_pCache->AddToCache( szFilename, pData, 0, u64FileSize, true );
					m_pCache->WriteCacheIndexFile();
				}
			}

			return pData;
		}

		//------------------------------------------------------------------------------------------

		void*				Load (const char* szFilename, uintfs* puFileSize, const u32 uFlags)
		{
			u32	uAlignment = FS_MEM_ALIGNMENT;
			if(uFlags & LOAD_FLAGS_VRAM)
				uAlignment	= 128;

			return (LoadAligned(uAlignment, szFilename, puFileSize, uFlags));
		}

		//------------------------------------------------------------------------------------------

		void				Save (const char* szFilename, const void* pData, uintfs uSize)
		{
			ASSERTS (szFilename, "Save() : Pointer to name is NULL!");
			ASSERTS (pData,	"Save() : Pointer to data is NULL!");

			u32 uHandle = Open(szFilename, xTRUE);
			if (uHandle != (u32)INVALID_FILE_HANDLE)
			{
				Write(uHandle, 0, uSize, pData, xFALSE);
				Close(uHandle);
			}
		}

		//------------------------------------------------------------------------------------------

		xbool				AsyncDone ( const u32 uHandle )
		{
			if (uHandle==(u32)INVALID_FILE_HANDLE)
			{
				SetLastError(FILE_ERROR_BADF);
				return true;
			}

			bool boDone = true;
			for(u32 nSlot = 0; nSlot < FS_MAX_ASYNC_IO_OPS; nSlot++)
			{
				AsyncIOInfo* pOperation = GetAsyncIOData(nSlot);
				if (pOperation->m_nFileIndex>=0 && (u32)pOperation->m_nFileIndex==uHandle)
				{
					boDone	= false;
					break;
				}
			}
			return boDone;
		}

		//------------------------------------------------------------------------------------------

		u32					AsyncOpen ( const char* szName, xbool boWrite, xbool boRetry )
		{
			u32 uHandle = AsyncPreOpen( szName, boWrite );
			AsyncOpen(uHandle);
			return uHandle;
		}

		//------------------------------------------------------------------------------------------

		void				AsyncOpen (const u32 uHandle)
		{
			if (uHandle==(u32)INVALID_FILE_HANDLE)
				return;

			AsyncIOInfo* pOpen = FreeAsyncIOPop();

			if(pOpen == 0)
			{
				ASSERTS(0, "xfilesystem: " TARGET_PLATFORM_STR " ERROR Out of AsyncIO slots");

				SetLastError(FILE_ERROR_MAX_ASYNC);

				Sync(FS_SYNC_WAIT);
				pOpen = FreeAsyncIOPop();
			}

			FileInfo* pInfo = GetFileInfo(uHandle);

			pOpen->m_nFileIndex 		= uHandle;
			pOpen->m_nStatus 			= FILE_OP_STATUS_OPEN_PENDING;
			pOpen->m_pReadAddress		= NULL;
			pOpen->m_pWriteAddress		= NULL;
			pOpen->m_uReadWriteOffset	= 0;
			pOpen->m_uReadWriteSize		= 0;

			AsyncIOAddToTail(pOpen);
			AsyncIOWorkerResume();
		}

		//------------------------------------------------------------------------------------------

		void				AsyncRead			( const u32 uHandle, uintfs uOffset, uintfs uSize, void* pBuffer, xbool boRetry  )
		{
			if (uHandle==(u32)INVALID_FILE_HANDLE)
			{
				SetLastError(FILE_ERROR_BADF);
				return;
			}

			AsyncIOInfo* pRead = FreeAsyncIOPop();

			if(pRead == 0)
			{
				ASSERTS(0, "xfilesystem:" TARGET_PLATFORM_STR " ERROR Out of AsyncIO slots");

				SetLastError(FILE_ERROR_MAX_ASYNC);

				Sync(FS_SYNC_WAIT);
				pRead = FreeAsyncIOPop();
			}

			FileInfo* pInfo = GetFileInfo(uHandle);

			pRead->m_nFileIndex 		= uHandle;
			pRead->m_nStatus 			= FILE_OP_STATUS_READ_PENDING;
			pRead->m_pReadAddress		= pBuffer;
			pRead->m_pWriteAddress		= NULL;
			pRead->m_uReadWriteOffset	= uOffset;
			pRead->m_uReadWriteSize		= uSize;

			AsyncIOAddToTail(pRead);
			AsyncIOWorkerResume();
		}

		//------------------------------------------------------------------------------------------

		void				AsyncWrite			( const u32 uHandle, uintfs uOffset, uintfs uSize, const void* pBuffer, xbool boRetry  )
		{
			if (uHandle==(u32)INVALID_FILE_HANDLE)
			{
				SetLastError(FILE_ERROR_BADF);
				return;
			}

			AsyncIOInfo* pWrite = FreeAsyncIOPop();

			if(pWrite == 0)
			{
				ASSERTS(0, "xfilesystem:" TARGET_PLATFORM_STR " ERROR Out of AsyncIO slots");

				SetLastError(FILE_ERROR_MAX_ASYNC);

				Sync(FS_SYNC_WAIT);
				pWrite = FreeAsyncIOPop();
			}

			FileInfo* pInfo = GetFileInfo(uHandle);

			pWrite->m_nFileIndex 		= uHandle;
			pWrite->m_nStatus 			= FILE_OP_STATUS_WRITE_PENDING;
			pWrite->m_pReadAddress		= NULL;
			pWrite->m_pWriteAddress		= pBuffer;
			pWrite->m_uReadWriteOffset	= uOffset;
			pWrite->m_uReadWriteSize	= uSize;

			AsyncIOAddToTail(pWrite);
			AsyncIOWorkerResume();
		}

		//------------------------------------------------------------------------------------------

		void				AsyncDelete(const u32 uHandle)
		{
			if (uHandle==(u32)INVALID_FILE_HANDLE)
			{
				SetLastError(FILE_ERROR_BADF);
				return;
			}

			AsyncIOInfo* pDelete = FreeAsyncIOPop();

			if(pDelete == 0)
			{
				ASSERTS(0, "xfilesystem:" TARGET_PLATFORM_STR " ERROR Out of AsyncIO slots");

				SetLastError(FILE_ERROR_MAX_ASYNC);

				Sync(FS_SYNC_WAIT);
				pDelete = FreeAsyncIOPop();
			}

			FileInfo* pInfo = GetFileInfo(uHandle);

			pDelete->m_nFileIndex 		= uHandle;
			pDelete->m_nStatus 			= FILE_OP_STATUS_DELETE_PENDING;
			pDelete->m_pReadAddress		= NULL;
			pDelete->m_pWriteAddress	= NULL;
			pDelete->m_uReadWriteOffset	= 0;
			pDelete->m_uReadWriteSize	= 0;

			AsyncIOAddToTail(pDelete);
			AsyncIOWorkerResume();
		}

		//------------------------------------------------------------------------------------------

		void				AsyncClose (const u32 uHandle)
		{
			if (uHandle==(u32)INVALID_FILE_HANDLE)
			{
				SetLastError(FILE_ERROR_BADF);
				return;
			}

			AsyncIOInfo* pClose = FreeAsyncIOPop();

			if(pClose == 0)
			{
				ASSERTS(0, "xfilesystem:" TARGET_PLATFORM_STR " ERROR Out of AsyncIO slots");

				SetLastError(FILE_ERROR_MAX_ASYNC);

				Sync(FS_SYNC_WAIT);
				pClose = FreeAsyncIOPop();
			}

			FileInfo* pInfo = GetFileInfo(uHandle);

			pClose->m_nFileIndex 		= uHandle;
			pClose->m_nStatus 			= FILE_OP_STATUS_CLOSE_PENDING;
			pClose->m_pReadAddress		= NULL;
			pClose->m_pWriteAddress		= NULL;
			pClose->m_uReadWriteOffset	= 0;
			pClose->m_uReadWriteSize	= 0;

			AsyncIOAddToTail(pClose);
			AsyncIOWorkerResume();
		}


		//------------------------------------------------------------------------------------------

		u32 AsyncPreOpen( const char* szFilename, xbool boWrite )
		{
			//-----------------------------
			// File find a free file slot.
			//-----------------------------
			u32 uHandle = FindFreeFileSlot ();
			if(uHandle == (u32)INVALID_FILE_HANDLE)
			{
				ASSERTS(0, "xfilesystem:" TARGET_PLATFORM_STR " ERROR Too many files opened!");

				SetLastError(FILE_ERROR_MAX_FILES);
				return (u32)INVALID_FILE_HANDLE;
			}

			XSTRING_BUFFER(szFullName, FS_MAX_PATH);
			ESourceType eSource = CreateSystemPath(szFilename, szFullName);

			if (boWrite && IsSourceReadonly(eSource))
			{
				ASSERTS(0, "xfilesystem:" TARGET_PLATFORM_STR " ERROR Device is readonly!");

				SetLastError(FILE_ERROR_DEVICE_READONLY);
				return (u32)INVALID_FILE_HANDLE;
			}

			FileInfo* fileInfo = GetFileInfo(uHandle);
			fileInfo->m_uByteSize		= 0;

			fileInfo->m_uSectorOffset	= 0;
			fileInfo->m_uNumSectors		= 0;
			fileInfo->m_uSectorSize		= 0;

			fileInfo->m_nFileHandle		= (u32)PENDING_FILE_HANDLE;
            fileInfo->m_nFileIndex      = uHandle;

			x_strcpy(fileInfo->m_szFilename, FS_MAX_PATH, szFullName.c_str());

			fileInfo->m_boWriting		= boWrite;
			fileInfo->m_eSource			= eSource;

			return uHandle;
		}

		//------------------------------------------------------------------------------------------

		xbool AsyncQueueOpen( const u32 uHandle, const u32 uPriority, AsyncQueueCallBack callbackFunc, s32 nCallbackID )
		{
			return __private::AsyncQueueAdd (FILE_QUEUE_TO_OPEN, uHandle, uPriority, 0, 0, NULL, NULL, callbackFunc, NULL, NULL, nCallbackID);
		}

		//------------------------------------------------------------------------------------------

		xbool AsyncQueueOpen( const u32 uHandle, const u32 uPriority, AsyncQueueCallBack2 callbackFunc2, void* pClass, s32 nCallbackID )
		{
			return __private::AsyncQueueAdd (FILE_QUEUE_TO_OPEN, uHandle, uPriority, 0, 0, NULL, NULL, NULL, callbackFunc2, pClass, nCallbackID);
		}

		//------------------------------------------------------------------------------------------

		xbool AsyncQueueClose( const u32 uHandle, const u32 uPriority, AsyncQueueCallBack callbackFunc, s32 nCallbackID )
		{
			return __private::AsyncQueueAdd (FILE_QUEUE_TO_CLOSE, uHandle, uPriority, 0, 0, NULL, NULL, callbackFunc, NULL, NULL, nCallbackID);
		}

		//------------------------------------------------------------------------------------------

		xbool AsyncQueueClose( const u32 uHandle, const u32 uPriority, AsyncQueueCallBack2 callbackFunc2, void* pClass, s32 nCallbackID )
		{
			return __private::AsyncQueueAdd (FILE_QUEUE_TO_CLOSE, uHandle, uPriority, 0, 0, NULL, NULL, NULL, callbackFunc2, pClass, nCallbackID);
		}

		//------------------------------------------------------------------------------------------

		xbool AsyncQueueRead (const u32 uHandle, const u32 uPriority, uintfs uOffset, uintfs uSize, void* pDest, AsyncQueueCallBack callbackFunc, s32 nCallbackID)
		{
			return __private::AsyncQueueAdd (FILE_QUEUE_TO_READ, uHandle, uPriority, uOffset, uSize, pDest, NULL, callbackFunc, NULL, NULL, nCallbackID);
		}

		//------------------------------------------------------------------------------------------

		xbool AsyncQueueRead (const u32 uHandle, const u32 uPriority, uintfs uOffset, uintfs uSize, void* pDest, AsyncQueueCallBack2 callbackFunc2, void* pClass, s32 nCallbackID)
		{
			return __private::AsyncQueueAdd (FILE_QUEUE_TO_READ, uHandle, uPriority, uOffset, uSize, pDest, NULL, NULL, callbackFunc2, pClass, nCallbackID);
		}

		//------------------------------------------------------------------------------------------

		xbool AsyncQueueReadAndCache (const u32 uHandle, const u32 uPriority, uintfs uOffset, uintfs uSize, void* pDest, AsyncQueueCallBack callbackFunc, s32 nCallbackID)
		{
			// Is file already cached?
			FileInfo* fileInfo = GetFileInfo(uHandle);
			if (fileInfo->m_eSource != FS_SOURCE_CACHE)
			{
				if (AsyncIONumFreeSlots() >= 5)
				{
					// No - needs to be cached
					XSTRING_BUFFER(szFilename, FS_MAX_PATH);
					szFilename = fileInfo->m_szFilename;
					const xalias* alias = FindAndRemoveAliasFromFilename(szFilename);

					AsyncQueueAdd (FILE_QUEUE_TO_READ, uHandle, uPriority, uOffset, uSize, pDest, NULL, NULL, NULL, NULL, 0);

					xfilecache* filecache = GetFileCache();
					if(	(filecache != NULL) && (filecache->SetCacheData( szFilename.c_str(), pDest, uOffset, uSize, true )) )
					{
						szFilename.insert("cache:\\");
						s32	nWriteHandle = AsyncPreOpen(szFilename, true);

						AsyncQueueAdd (FILE_QUEUE_TO_OPEN,	nWriteHandle, uPriority, uOffset, uSize, pDest, NULL, NULL, NULL, NULL, 0);
						AsyncQueueAdd (FILE_QUEUE_TO_WRITE, nWriteHandle, uPriority, uOffset, uSize, pDest, NULL, NULL, NULL, NULL, 0);
						AsyncQueueAdd (FILE_QUEUE_TO_CLOSE, nWriteHandle, uPriority, uOffset, uSize, pDest, NULL, NULL, NULL, NULL, 0);

						filecache->WriteCacheIndexFileAsync();
					}

					return AsyncQueueAdd (FILE_QUEUE_TO_MARKER, uHandle, uPriority, uOffset, uSize, pDest, NULL, callbackFunc, NULL, NULL, nCallbackID);
				}
				else
				{
					SetLastError(FILE_ERROR_MAX_ASYNC);
					return AsyncQueueAdd (FILE_QUEUE_TO_READ, uHandle, uPriority, uOffset, uSize, pDest, NULL, callbackFunc, NULL, NULL, nCallbackID);
				}
			}
			else
			{
				return AsyncQueueAdd (FILE_QUEUE_TO_READ, uHandle, uPriority, uOffset, uSize, pDest, NULL, callbackFunc, NULL, NULL, nCallbackID);
			}
		}

		//------------------------------------------------------------------------------------------

		xbool AsyncQueueReadAndCache (const u32 uHandle, const u32 uPriority, uintfs uOffset, uintfs uSize, void* pDest, AsyncQueueCallBack2 callbackFunc2, void* pClass, s32 nCallbackID)
		{
			// Is file already cached?
			xfilecache* filecache = GetFileCache();
			FileInfo* fileInfo = GetFileInfo(uHandle);
			if ((filecache != NULL) && (fileInfo->m_eSource != FS_SOURCE_CACHE))
			{
				if ((AsyncIONumFreeSlots() >= 5))
				{
					// No - needs to be cached
					XSTRING_BUFFER(szFilename, FS_MAX_PATH);
					szFilename = fileInfo->m_szFilename;
					const xalias* alias = FindAndRemoveAliasFromFilename(szFilename);

					AsyncQueueAdd (FILE_QUEUE_TO_READ, uHandle, uPriority, uOffset, uSize, pDest, NULL, NULL, NULL, NULL, 0);

					if(filecache->SetCacheData( szFilename.c_str(), pDest, uOffset, uSize, true ))
					{
						szFilename.insert("cache:\\");
						s32	nWriteHandle = AsyncPreOpen(szFilename, true);

						AsyncQueueAdd (FILE_QUEUE_TO_OPEN,	nWriteHandle, uPriority, uOffset, uSize, pDest, NULL, NULL, NULL, NULL, 0);
						AsyncQueueAdd (FILE_QUEUE_TO_WRITE, nWriteHandle, uPriority, uOffset, uSize, pDest, NULL, NULL, NULL, NULL, 0);
						AsyncQueueAdd (FILE_QUEUE_TO_CLOSE, nWriteHandle, uPriority, uOffset, uSize, pDest, NULL, NULL, NULL, NULL, 0);

						filecache->WriteCacheIndexFileAsync();
					}

					return AsyncQueueAdd (FILE_QUEUE_TO_MARKER, uHandle, uPriority, uOffset, uSize, pDest, NULL, NULL, callbackFunc2, pClass, nCallbackID);
				}
				else
				{
					SetLastError(FILE_ERROR_MAX_ASYNC);
					return AsyncQueueAdd (FILE_QUEUE_TO_READ, uHandle, uPriority, uOffset, uSize, pDest, NULL, NULL, callbackFunc2, pClass, nCallbackID);
				}
			}
			else
			{
				return AsyncQueueAdd (FILE_QUEUE_TO_READ, uHandle, uPriority, uOffset, uSize, pDest, NULL, NULL, callbackFunc2, pClass, nCallbackID);
			}
		}

		//------------------------------------------------------------------------------------------

		xbool AsyncQueueDelete( const u32 uHandle, const u32 uPriority, AsyncQueueCallBack callbackFunc, s32 nCallbackID )
		{
			return AsyncQueueAdd (FILE_QUEUE_TO_DELETE, uHandle, uPriority, 0, 0, NULL, NULL, callbackFunc, NULL, NULL, nCallbackID);
		}

		//------------------------------------------------------------------------------------------

		xbool AsyncQueueDelete( const u32 uHandle, const u32 uPriority, AsyncQueueCallBack2 callbackFunc2, void* pClass, s32 nCallbackID )
		{
			return AsyncQueueAdd (FILE_QUEUE_TO_DELETE, uHandle, uPriority, 0, 0, NULL, NULL, NULL, callbackFunc2, pClass, nCallbackID);
		}

		//------------------------------------------------------------------------------------------

		xbool AsyncQueueWrite (const u32 uHandle, const u32 uPriority, uintfs uOffset, uintfs uSize, const void* pSrc, AsyncQueueCallBack callbackFunc, s32 nCallbackID)
		{
			return AsyncQueueAdd (FILE_QUEUE_TO_WRITE, uHandle, uPriority, uOffset, uSize, NULL, pSrc, callbackFunc, NULL, NULL, nCallbackID);
		}

		//------------------------------------------------------------------------------------------

		xbool AsyncQueueWrite (const u32 uHandle, const u32 uPriority, uintfs uOffset, uintfs uSize, const void* pSrc, AsyncQueueCallBack2 callbackFunc2, void* pClass, s32 nCallbackID)
		{
			return AsyncQueueAdd (FILE_QUEUE_TO_WRITE, uHandle, uPriority, uOffset, uSize, NULL, pSrc, NULL, callbackFunc2, pClass, nCallbackID);
		}

		//------------------------------------------------------------------------------------------

		void AsyncQueueCancel( const u32 uHandle )
		{
			for(u32 uPriority = 0; uPriority < (FS_PRIORITY_COUNT); uPriority++)
			{
				QueueItem* pItem		= AsyncQueueRemoveHead(uPriority);
				QueueItem* pStartItem	= pItem;

				while(pItem)
				{
					if(	(pItem->m_uStatus != FILE_QUEUE_FREE) && (pItem->m_nHandle == uHandle) )
					{
						if(	(pItem->m_uStatus == FILE_QUEUE_OPENING) || 
							(pItem->m_uStatus == FILE_QUEUE_CLOSING) || 
							(pItem->m_uStatus == FILE_QUEUE_READING) || 
							(pItem->m_uStatus == FILE_QUEUE_STATING) ||
							(pItem->m_uStatus == FILE_QUEUE_WRITING) )
						{
							pItem->m_CallbackFunc	= NULL;
							pItem->m_CallbackFunc2	= NULL;
							pItem->m_CallbackFunc3	= NULL;
							pItem->m_pCallbackClass	= NULL;
							pItem->m_nCallbackID	= 0;
						}
						else
						{
							pItem->m_uStatus = FILE_QUEUE_CANCELLED;
						}
					}

					AsyncQueueAddToTail(uPriority, pItem);

					// Terminate the loop when we encounter our start item
					pItem = AsyncQueueRemoveHead(uPriority);
					if(pItem == pStartItem)
					{
						AsyncQueueAddToHead(uPriority, pItem);
						break;
					}
				}
			}
		}

		//------------------------------------------------------------------------------------------

		void AsyncQueueCancel( const u32 uHandle, void* pClass )
		{
			for(u32 uPriority = 0; uPriority < (FS_PRIORITY_COUNT); uPriority++)
			{
				QueueItem* pItem		= AsyncQueueRemoveHead(uPriority);
				QueueItem* pStartItem	= pItem;

				while(pItem)
				{
					if(	(pItem->m_uStatus != FILE_QUEUE_FREE) && (pItem->m_nHandle == uHandle) && (pItem->m_pCallbackClass == pClass) )
					{
						if(	(pItem->m_uStatus == FILE_QUEUE_OPENING) ||
							(pItem->m_uStatus == FILE_QUEUE_CLOSING) ||
							(pItem->m_uStatus == FILE_QUEUE_READING) ||
							(pItem->m_uStatus == FILE_QUEUE_STATING) ||
							(pItem->m_uStatus == FILE_QUEUE_WRITING) )
						{
							pItem->m_CallbackFunc	= NULL;
							pItem->m_CallbackFunc2	= NULL;
							pItem->m_CallbackFunc3	= NULL;
							pItem->m_pCallbackClass	= NULL;
							pItem->m_nCallbackID	= 0;
						}
						else
						{
							pItem->m_uStatus = FILE_QUEUE_CANCELLED;
						}
					}

					AsyncQueueAddToTail(uPriority, pItem);

					// Terminate the loop when we encounter our start item
					pItem = AsyncQueueRemoveHead(uPriority);
					if(pItem == pStartItem)
					{
						AsyncQueueAddToHead(uPriority, pItem);
						break;
					}
				}
			}
		}

		//------------------------------------------------------------------------------------------

		xbool				AsyncQueueDone		( const u32 uHandle )
		{
			// Is the file handle still somewhere in the Queue?
			for(u32 uPriority = 0; uPriority < (FS_PRIORITY_COUNT); uPriority++)
			{
				QueueItem* pItem		= AsyncQueueRemoveHead(uPriority);
				QueueItem* pStartItem	= pItem;

				while(pItem)
				{
					if(	(pItem->m_uStatus != FILE_QUEUE_FREE) && (pItem->m_nHandle == uHandle))
					{
						return xFALSE;
					}

					AsyncQueueAddToTail(uPriority, pItem);

					// Terminate the loop when we encounter our start item
					pItem = AsyncQueueRemoveHead(uPriority);
					if(pItem == pStartItem)
					{
						AsyncQueueAddToHead(uPriority, pItem);
						break;
					}
				}
			}
			return xTRUE;
		}


		//------------------------------------------------------------------------------------------

		void WaitUntilIdle(void)
		{
			xbool boDone = __private::AsyncQueueUpdate();
			while(!boDone)
			{
				Sync(FS_SYNC_WAIT);
				boDone = __private::AsyncQueueUpdate();
			}
		}

		//------------------------------------------------------------------------------------------

		u32				FindFileHandle		( const char* szName )
		{
			for (u32 i=0; i<FS_MAX_OPENED_FILES; ++i)
			{
				FileInfo* pInfo = GetFileInfo(i);
				if (pInfo->m_nFileHandle != (u32)INVALID_FILE_HANDLE)
				{
					if (x_stricmp(pInfo->m_szFilename, szName) == 0)
						return i;
				}
			}
			return (u32)INVALID_FILE_HANDLE;
		}

	};


	//------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------
	//----------------------- xfilecache Implementation ----------------------------------------
	//------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------
	namespace xfilesystem
	{
		//---------------------
		// statics
		//---------------------
		xfilecache::CallbackData	xfilecache::m_xCallbacks[MAX_CALLBACKS];


		void xfilecache::FileIOCallback( u32 nHandle, s32 nID )
		{
			if(m_xCallbacks[nID].m_Callback == NULL)
			{
				m_xCallbacks[nID].m_Callback(m_xCallbacks[nID].m_pClass, m_xCallbacks[nID].m_nID);
				m_xCallbacks[nID].m_Callback = NULL;
			}
		}

		xfilecache::xfilecache() 
			: m_nCacheHandle((u32)INVALID_FILE_HANDLE)
			, m_uCacheSize(0)
		{
			x_memset(&m_xHeader, 0, sizeof(CacheHeader));
			x_memset(m_xCallbacks, 0, sizeof(CallbackData) * MAX_CALLBACKS);
		}

		void	xfilecache::Initialise() 
		{
			if (xfilesystem::DoesFileExist(CACHE_FILENAME))
			{
				m_nCacheHandle	= xfilesystem::Open(CACHE_FILENAME, true);
				xfilesystem::Read(m_nCacheHandle, 0, sizeof(CacheHeader), &m_xHeader, false);

				for(s32 nFile = 0; nFile < MAX_CACHED_FILES; nFile++)
				{
					if(m_xHeader.m_xCacheList[nFile].m_uFlags & CACHE_FILE_FLAGS_BUSY)
					{
						if(xfilesystem::DoesFileExist(m_xHeader.m_xCacheList[nFile].m_szName))
						{
							u32 h = xfilesystem::Open(m_xHeader.m_xCacheList[nFile].m_szName, false);
							xfilesystem::Delete(h);
						}
					}
					else if(m_xHeader.m_xCacheList[nFile].m_uFlags & CACHE_FILE_FLAGS_VALID)
					{
						if(m_xHeader.m_xCacheList[nFile].m_uFlags & CACHE_FILE_FLAGS_PERMANENT)
						{
							FileEntry* pEntry = m_xPermanentList.getHead();

							while(pEntry)
							{
								if(m_xHeader.m_xCacheList[nFile].m_uOffset < pEntry->m_uOffset)
								{
									m_xPermanentList.addBefore(&m_xHeader.m_xCacheList[nFile], pEntry);
									break;
								}

								pEntry	= pEntry->getNext();
							}

							if(pEntry == NULL)
							{
								m_xPermanentList.addToTail(&m_xHeader.m_xCacheList[nFile]);
							}
						}
						else
						{
							FileEntry* pEntry = m_xTransientList.getTail();

							while(pEntry)
							{
								if(m_xHeader.m_xCacheList[nFile].m_uOffset > pEntry->m_uOffset)
								{
									m_xTransientList.addAfter(&m_xHeader.m_xCacheList[nFile], pEntry);
									break;
								}

								pEntry	= pEntry->getNext();
							}

							if(pEntry == NULL)
							{
								m_xTransientList.addToHead(&m_xHeader.m_xCacheList[nFile]);
							}
						}
					}
				}
			}
			else
			{
				m_nCacheHandle = xfilesystem::Open(CACHE_FILENAME, true);
				xfilesystem::Write(m_nCacheHandle, 0, sizeof(CacheHeader), &m_xHeader, false);
			}

			m_uCacheSize = xfilesystem::GetFreeSize(CACHE_PATH);
		}

		//------------------------------------------------------------------------------------------

		xfilecache::~xfilecache()
		{
			if (m_nCacheHandle != (u32)INVALID_FILE_HANDLE)
				xfilesystem::Close(m_nCacheHandle);
		}

		//------------------------------------------------------------------------------------------

		void xfilecache::PurgeCache()
		{
			for(s32 nFile = 0; nFile < MAX_CACHED_FILES; nFile++)
			{
				m_xHeader.m_xCacheList[nFile].m_uFlags = CACHE_FILE_FLAGS_INVALID;
			}

			xfilesystem::CreateFileList("cache:\\", true);
			for(s32 nFile = 0; nFile < xfilesystem::GetFileListLength(); nFile++)
			{
				const char* szFile = xfilesystem::GetFileListData(nFile);
				u32 h = xfilesystem::Open(szFile, false);
				xfilesystem::Delete(h);
			}
			xfilesystem::DestroyFileList();
		}

		//------------------------------------------------------------------------------------------

		void xfilecache::InvalidateCacheIndexFile()
		{
			xfilesystem::FileInfo* xInfo = xfilesystem::GetFileInfo(m_nCacheHandle);
			u32 h = xfilesystem::Open(xInfo->m_szFilename, false);
			xfilesystem::Delete(h);
		}

		//------------------------------------------------------------------------------------------

		void xfilecache::InvalidateCacheIndexFileAsync()
		{
			xfilesystem::AsyncQueueDelete(m_nCacheHandle, 0, NULL, 0);
		}

		//------------------------------------------------------------------------------------------

		void xfilecache::WriteCacheIndexFile()
		{
			for(s32 nFile = 0; nFile < MAX_CACHED_FILES; nFile++)
			{
				if(m_xHeader.m_xCacheList[nFile].m_uFlags & CACHE_FILE_FLAGS_BUSY)
				{
					m_xHeader.m_xCacheList[nFile].m_uFlags &= ~CACHE_FILE_FLAGS_BUSY;
					m_xHeader.m_xCacheList[nFile].m_uFlags |= CACHE_FILE_FLAGS_VALID;
				}
			}

			xfilesystem::Write(m_nCacheHandle, 0, sizeof(CacheHeader), &m_xHeader, false);
		}

		//------------------------------------------------------------------------------------------

		void xfilecache::WriteCacheIndexFileAsync()
		{
			for(s32 nFile = 0; nFile < MAX_CACHED_FILES; nFile++)
			{
				if(m_xHeader.m_xCacheList[nFile].m_uFlags & CACHE_FILE_FLAGS_BUSY)
				{
					m_xHeader.m_xCacheList[nFile].m_uFlags &= ~CACHE_FILE_FLAGS_BUSY;
					m_xHeader.m_xCacheList[nFile].m_uFlags |= CACHE_FILE_FLAGS_VALID;
				}
			}

			xfilesystem::AsyncQueueWrite(m_nCacheHandle, xfilesystem::FS_PRIORITY_HIGH, 0, sizeof(CacheHeader), &m_xHeader, NULL, 0);
		}

		//------------------------------------------------------------------------------------------

		s32 xfilecache::GetCachedIndex( const char* szFilename, u64 uOffset )
		{
			for(s32 nFile = 0; nFile < MAX_CACHED_FILES; nFile++)
			{
				if(m_xHeader.m_xCacheList[nFile].m_uFlags & CACHE_FILE_FLAGS_VALID)
				{
					if(x_strnicmp(m_xHeader.m_xCacheList[nFile].m_szName, szFilename, x_strlen(szFilename)) == 0)
					{
						if(m_xHeader.m_xCacheList[nFile].m_uOffset == uOffset)
						{
							return nFile;
						}
					}
				}
			}

			return -1;
		}

		//------------------------------------------------------------------------------------------

		void xfilecache::GetCacheData( s32 nIndex, FileEntry& rxFileEntry )
		{
			if(nIndex >= 0 && nIndex < MAX_CACHED_FILES)
			{
				rxFileEntry	= m_xHeader.m_xCacheList[nIndex];
			}
			else
			{
				ASSERTS(0, "Index out of range\n");
			}
		}

		//------------------------------------------------------------------------------------------

		bool xfilecache::AddToCache( const char* szFilename, void* pData, const u64 uOffset, const u64 uSize, const bool boPermanent )
		{
			for(s32 nFile = 0; nFile < MAX_CACHED_FILES; nFile++)
			{
				if(m_xHeader.m_xCacheList[nFile].m_uFlags == CACHE_FILE_FLAGS_INVALID)
				{
					XSTRING_BUFFER(szPath, xfilesystem::FS_MAX_PATH);

					if (uOffset != 0)
					{
						char szTempOffset[xfilesystem::FS_MAX_PATH];
						xstring_buffer offsetStr(szTempOffset, xfilesystem::FS_MAX_PATH, "@%x", x_va_list((s32)uOffset));
						szPath	+= szFilename;
						szPath  += offsetStr;
					}
					else
					{
						szPath	= szFilename;
					}

					XSTRING_BUFFER(szCachePath, xfilesystem::FS_MAX_PATH);
					xfilesystem::ReplaceAliasOfFilename(szPath, xfilesystem::FindAliasFromFilename(szPath.c_str()));

					u32	nWriteHandle = xfilesystem::Open(szCachePath, true);
					xfilesystem::Write(nWriteHandle, 0, uSize, pData, false);
					xfilesystem::Close(nWriteHandle);

					x_strcpy(m_xHeader.m_xCacheList[nFile].m_szName, CACHED_FILE_NAME_LENGTH, szPath.c_str());
					m_xHeader.m_xCacheList[nFile].m_uOffset	= uOffset;
					m_xHeader.m_xCacheList[nFile].m_uSize	= uSize;
					m_xHeader.m_xCacheList[nFile].m_uFlags	= boPermanent ? CACHE_FILE_FLAGS_PERMANENT | CACHE_FILE_FLAGS_BUSY : CACHE_FILE_FLAGS_BUSY;
					m_xHeader.m_xCacheList[nFile].m_uCRC	= 0;

					if(boPermanent)
					{
						m_xPermanentList.addToTail(&m_xHeader.m_xCacheList[nFile]);
					}
					else
					{
						m_xTransientList.addToHead(&m_xHeader.m_xCacheList[nFile]);
					}

					return true;
				}
			}

			return false;
		}

		//------------------------------------------------------------------------------------------

		bool xfilecache::AddToCacheAsync( const char* szFilename, void* pData, const u64 uOffset, const u64 uSize, const bool boPermanent, CacheCallBack Callback, void* pClass, s32 nID )
		{
			for(s32 nFile = 0; nFile < MAX_CACHED_FILES; nFile++)
			{
				if(m_xHeader.m_xCacheList[nFile].m_uFlags == CACHE_FILE_FLAGS_INVALID)
				{
					s32	nCallback	= -1;

					for(s32 nCB = 0; nCB < MAX_CALLBACKS; nCB++)
					{
						if(m_xCallbacks[nCB].m_Callback == NULL)
						{
							m_xCallbacks[nCB].m_Callback	= Callback;
							m_xCallbacks[nCB].m_pClass		= pClass;
							m_xCallbacks[nCB].m_nID			= nID;

							nCallback	= nCB;
							break;
						}
					}

					if(nCallback == -1)
					{
						return false;
					}

					XSTRING_BUFFER(szPath, xfilesystem::FS_MAX_PATH);
					if(uOffset != 0)
					{
						szPath.format("%s@%x", x_va_list(szFilename, (s32)uOffset));
					}
					else
					{
						szPath	= szFilename;
					}
					xfilesystem::ReplaceAliasOfFilename(szPath, xfilesystem::FindAlias("cache"));

					u32 nWriteHandle = xfilesystem::AsyncPreOpen(szPath, xTRUE);
					xfilesystem::AsyncQueueOpen(nWriteHandle, xfilesystem::FS_PRIORITY_HIGH, NULL, 0);
					xfilesystem::AsyncQueueWrite(nWriteHandle, xfilesystem::FS_PRIORITY_HIGH, 0, uSize, pData, FileIOCallback, nCallback);
					xfilesystem::AsyncQueueClose(nWriteHandle, xfilesystem::FS_PRIORITY_HIGH, NULL, 0);

					x_strcpy(m_xHeader.m_xCacheList[nFile].m_szName, CACHED_FILE_NAME_LENGTH, szFilename);
					m_xHeader.m_xCacheList[nFile].m_uOffset	= uOffset;
					m_xHeader.m_xCacheList[nFile].m_uSize	= uSize;
					m_xHeader.m_xCacheList[nFile].m_uFlags	= boPermanent ? CACHE_FILE_FLAGS_PERMANENT | CACHE_FILE_FLAGS_BUSY : CACHE_FILE_FLAGS_BUSY;
					m_xHeader.m_xCacheList[nFile].m_uCRC	= 0;

					if(boPermanent)
					{
						m_xPermanentList.addToTail(&m_xHeader.m_xCacheList[nFile]);
					}
					else
					{
						m_xTransientList.addToHead(&m_xHeader.m_xCacheList[nFile]);
					}

					return true;
				}
			}

			return false;
		}

		//------------------------------------------------------------------------------------------

		bool xfilecache::SetCacheData( const char* szFilename, void* pData, const u64 uOffset, const u64 uSize, const bool boPermanent )
		{
			for(s32 nFile = 0; nFile < MAX_CACHED_FILES; nFile++)
			{
				if(m_xHeader.m_xCacheList[nFile].m_uFlags == CACHE_FILE_FLAGS_INVALID)
				{
					x_strcpy(m_xHeader.m_xCacheList[nFile].m_szName, CACHED_FILE_NAME_LENGTH, szFilename);
					m_xHeader.m_xCacheList[nFile].m_uOffset	= uOffset;
					m_xHeader.m_xCacheList[nFile].m_uSize	= uSize;
					m_xHeader.m_xCacheList[nFile].m_uFlags	= boPermanent ? CACHE_FILE_FLAGS_PERMANENT | CACHE_FILE_FLAGS_BUSY : CACHE_FILE_FLAGS_BUSY;
					m_xHeader.m_xCacheList[nFile].m_uCRC	= 0;

					if(boPermanent)
					{
						m_xPermanentList.addToTail(&m_xHeader.m_xCacheList[nFile]);
					}
					else
					{
						m_xTransientList.addToHead(&m_xHeader.m_xCacheList[nFile]);
					}

					return true;
				}
			}

			return false;
		}

		//------------------------------------------------------------------------------------------

		void xfilecache::RemoveFromCache( const char* szFilename )
		{
			s32	nIndex	= GetCachedIndex(szFilename);

			if(nIndex >= 0)
			{
				if(m_xHeader.m_xCacheList[nIndex].m_uFlags & CACHE_FILE_FLAGS_PERMANENT)
				{
				}

				XSTRING_BUFFER(cacheFilename, xfilesystem::FS_MAX_PATH);
				cacheFilename = m_xHeader.m_xCacheList[nIndex].m_szName;
				xfilesystem::ReplaceAliasOfFilename(cacheFilename, xfilesystem::FindAlias("cache"));
				u32 h = xfilesystem::Open(cacheFilename.c_str());
				xfilesystem::Delete(h);
				m_xHeader.m_xCacheList[nIndex].m_uFlags = CACHE_FILE_FLAGS_INVALID;
			}
		}

	};


	//------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------
	//----------------------- xalias Implementation ------------------------------------------
	//------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------

	namespace xfilesystem
	{
		enum EFileSystemConfig
		{
			MAX_FILE_ALIASES = 32,
		};


		static s32			sNumAliases = 0;
		static xalias		sAliasList[MAX_FILE_ALIASES];


		//==============================================================================
		// Functions
		//==============================================================================
		xalias::xalias()
			: mAliasStr(NULL)
			, mAliasTargetStr(NULL)
			, mRemapStr(NULL)
			, mSource(FS_SOURCE_UNDEFINED)
		{
		}

		//------------------------------------------------------------------------------

		xalias::xalias(const char* alias, const char* aliasTarget)
			: mAliasStr(alias)
			, mAliasTargetStr(aliasTarget)
			, mRemapStr(NULL)
			, mSource(FS_SOURCE_UNDEFINED)
		{
		}

		//------------------------------------------------------------------------------

		xalias::xalias(const char* alias, ESourceType source, const char* remap)
			: mAliasStr(alias)
			, mAliasTargetStr(NULL)
			, mRemapStr(remap)
			, mSource(source)
		{
		}

		//------------------------------------------------------------------------------

		const char* xalias::remap() const
		{
			if (mAliasTargetStr != NULL)
			{
				const xalias* a = FindAlias(mAliasTargetStr);
				if (a == NULL)
					return mRemapStr;
				mRemapStr = a->remap();
			}
			return mRemapStr;
		}

		//------------------------------------------------------------------------------

		ESourceType	xalias::source() const
		{
			if (mAliasTargetStr != NULL)
			{
				const xalias* a = FindAlias(mAliasTargetStr);
				if (a == NULL)
					return FS_SOURCE_UNDEFINED;
				return a->source();
			}

			return mSource;
		}

		//------------------------------------------------------------------------------

		void            AddAlias(xalias& alias)
		{
			for (s32 i=0; i<sNumAliases; ++i)
			{
				if (x_stricmp(sAliasList[i].alias(), alias.alias()) == 0)
				{
					sAliasList[i] = alias;
					xconsole::writeLine("INFO replaced alias %s", x_va_list(alias.alias()));
					return;
				}
			}

			if (sNumAliases < MAX_FILE_ALIASES)
			{
				sAliasList[sNumAliases] = alias;
				sNumAliases++;
			}
			else
			{
				xconsole::writeLine("ERROR cannot add another xfilesystem alias, maximum amount of aliases reached");
			}
		}

		//------------------------------------------------------------------------------

		const xalias* FindAlias(const char* _alias)
		{
			for (s32 i=0; i<sNumAliases; ++i)
			{
				if (x_strCompareNoCase(sAliasList[i].alias(), _alias) == 0)
				{
					return &sAliasList[i];
				}
			}
			return NULL;
		}

		//------------------------------------------------------------------------------

		const xalias* FindAliasFromFilename(const char* inFilename)
		{
			char deviceStrBuffer[32];
			xstring_buffer deviceStr(deviceStrBuffer, sizeof(deviceStrBuffer));

			xstring_const filename(inFilename);
			s32 pos = filename.find(":\\");
			if (pos>0)
				deviceStr.insert(filename.c_str(), pos);

			const xalias* alias = FindAlias(deviceStr.c_str());
			return alias;
		}

		//------------------------------------------------------------------------------

		const xalias*		FindAndRemoveAliasFromFilename(xstring_buffer& ioFilename)
		{
			const xalias* alias = FindAliasFromFilename(ioFilename);
			s32 pos = ioFilename.find(":\\");
			if (pos>0)
			{
				ioFilename.remove(0, pos+2);
			}
			return alias;
		}

		//------------------------------------------------------------------------------

		void				ReplaceAliasOfFilename(xstring_buffer& ioFilename, const xalias* inNewAlias)
		{
			if (inNewAlias==NULL)
				return;

			s32 pos = ioFilename.find(":\\");
			if (pos>0)
			{
				ioFilename.replace(0, pos, inNewAlias->alias());
			}
		}


		//------------------------------------------------------------------------------

		void				ExitAlias()
		{
			sNumAliases = 0;
			for (s32 i=0; i<MAX_FILE_ALIASES; ++i)
				sAliasList[i] = xalias();
		}


		namespace __private
		{
			void				InitialiseCommon ( u32 uAsyncQueueSize, xbool boEnableCache )
			{
				x_printf ("xfilesystem:"TARGET_PLATFORM_STR" INFO Initialise()\n");

				//--------------------------------
				// Clear the error stack
				//--------------------------------
				for (u32 i=0; i<FS_MAX_ERROR_ITEMS; ++i)
					m_eLastErrorStack[i] = FILE_ERROR_OK;

				//--------------------------------
				// Create the Async loading queue.
				//--------------------------------
				ASSERTS (uAsyncQueueSize > 0 && uAsyncQueueSize <= FS_MAX_ASYNC_QUEUE_ITEMS, "Initialise() : Async Queue is 0 or to large!");

				//--------------------------------------------------
				// Fill in the status of the first element in queue.
				//--------------------------------------------------
				for (u32 uLoop = 0; uLoop < FS_MAX_ASYNC_QUEUE_ITEMS; uLoop++)
				{
					m_aAsyncQueue[uLoop].clear();
					m_pFreeQueueItemList.push(&(m_aAsyncQueue[uLoop]));
				}

				//---------------------------------------
				// Current there is no Async file loaded.
				//---------------------------------------
				for (u32 uFile = 0; uFile < FS_MAX_OPENED_FILES; uFile++)
				{
					m_OpenAsyncFile[uFile].clear();
				}

				for(u32 uSlot = 0; uSlot < FS_MAX_ASYNC_IO_OPS; uSlot++)	
				{
					m_AsyncIOData[uSlot].clear();
					m_pFreeAsyncIOList.push(&m_AsyncIOData[uSlot]);
				}

				if (boEnableCache)
					CreateFileCache();
			}	

			//------------------------------------------------------------------------------------------

			void				ShutdownCommon		( void )
			{
				DestroyFileCache();
				x_printf ("xfilesystem:"TARGET_PLATFORM_STR" INFO Shutdown()\n");
			}

			//------------------------------------------------------------------------------------------

			ESourceType			CreateSystemPath( const char* szFilename, xstring_buffer& outFilename )
			{
				outFilename = szFilename;

				ESourceType sourceType = FS_SOURCE_UNDEFINED;
				const xfilesystem::xalias* alias = xfilesystem::FindAliasFromFilename(szFilename);

				if (alias != NULL)
				{
					// Remove the device part
					outFilename.remove(0, x_strlen(alias->alias()) + 2);

					sourceType = alias->source();
					if (alias->remap() != NULL)
					{
						outFilename.insert(alias->remap());
					}
				}
				else
				{
					SetLastError(FILE_ERROR_DEVICE);
				}

				//-----------------------------------------------------
				// Fix the slash direction - needs to be UNIX style '/'
				//-----------------------------------------------------
				if (IsPathUNIXStyle())		outFilename.replace('\\', '/');
				else						outFilename.replace('/', '\\');

				return sourceType;
			}

			//------------------------------------------------------------------------------------------

			const char*			GetFileExtension( const char* szFilename )
			{
				const char* szExtension	= x_strrchr(szFilename, '.');
				if (szExtension)
				{
					return &szExtension[1];
				}
				return NULL;
			}

			//------------------------------------------------------------------------------------------

			xbool				IsSourceReadonly	( ESourceType eSource )
			{
				switch (eSource)
				{
				case FS_SOURCE_UMD			:
				case FS_SOURCE_BDVD			:
				case FS_SOURCE_DVD			:
					return xTRUE;

				case FS_SOURCE_HOST			:
				case FS_SOURCE_HDD			:
				case FS_SOURCE_MS			:
				case FS_SOURCE_CACHE		:
				case FS_SOURCE_REMOTE		:
				case FS_SOURCE_USB			:
				default:
					break;
				}

				return xFALSE;
			}

			//------------------------------------------------------------------------------------------

			FileInfo*			GetFileInfo			( u32 uHandle )
			{
				ASSERT(uHandle<FS_MAX_OPENED_FILES);
				return &m_OpenAsyncFile[uHandle];
			}

			//------------------------------------------------------------------------------------------

			u32					FindFreeFileSlot (void)
			{
				for (s32 nSlot = 0; nSlot < FS_MAX_OPENED_FILES; nSlot++)
				{
					if (m_OpenAsyncFile[nSlot].m_nFileHandle == (u32)INVALID_FILE_HANDLE)
						return (nSlot);
				}
				return (u32)-1;
			}

			//------------------------------------------------------------------------------------------

			s32					AsyncIONumFreeSlots()
			{
				return m_pFreeQueueItemList.size();
			}

			//------------------------------------------------------------------------------------------

			AsyncIOInfo*		GetAsyncIOData		( u32 nSlot )
			{
				AsyncIOInfo* asyncIOInfo = &m_AsyncIOData[nSlot];
				return asyncIOInfo;
			}

			//------------------------------------------------------------------------------------------

			AsyncIOInfo*		FreeAsyncIOPop		( void )
			{
				AsyncIOInfo* asyncIOInfo;
				if (m_pFreeAsyncIOList.pop(asyncIOInfo))
					return asyncIOInfo;
				else
					return NULL;
			}

			//------------------------------------------------------------------------------------------

			void				FreeAsyncIOAddToTail( AsyncIOInfo* asyncIOInfo )
			{
				asyncIOInfo->m_nFileIndex = INVALID_FILE_HANDLE;
				m_pFreeAsyncIOList.push(asyncIOInfo);
			}

			//------------------------------------------------------------------------------------------

			AsyncIOInfo*		AsyncIORemoveHead	( void )
			{
				AsyncIOInfo* item;
				if (m_pAsyncIOList.pop(item))
					return item;
				return NULL;
			}

			//------------------------------------------------------------------------------------------

			void				AsyncIOAddToTail	( AsyncIOInfo* asyncIOInfo )
			{
				m_pAsyncIOList.push(asyncIOInfo);
			}

			//------------------------------------------------------------------------------------------

			QueueItem*			FreeAsyncQueuePop	( )
			{
				QueueItem* item;
				if (m_pFreeQueueItemList.pop(item))
					return item;
				return NULL;
			}

			//------------------------------------------------------------------------------------------

			void				FreeAsyncQueueAddToTail( QueueItem* asyncQueueItem )
			{
				m_pFreeQueueItemList.push(asyncQueueItem);
			}

			//------------------------------------------------------------------------------------------

			QueueItem*			AsyncQueueRemoveHead( u32 uPriority )
			{
				QueueItem* item;
				if (m_pAsyncQueueList[uPriority].pop(item))
					return item;
				return NULL;
			}

			//------------------------------------------------------------------------------------------

			void				AsyncQueueAddToTail	( u32 uPriority, QueueItem* asyncQueueItem )
			{
				m_pAsyncQueueList[uPriority].push(asyncQueueItem);
			}

			//------------------------------------------------------------------------------------------

			void				AsyncQueueAddToHead	( u32 uPriority, QueueItem* asyncQueueItem )
			{
				m_pAsyncQueueList[uPriority].push(asyncQueueItem);
			}

			//------------------------------------------------------------------------------------------

			QueueItem*			__AsyncQueueAdd (const EFileQueueStatus uOperation, const u32 uHandle, const u32 uPriority, uintfs uOffset, uintfs uSize, void* pDest, const void* pSrc)
			{
				QueueItem*	pQueueItem			= FreeAsyncQueuePop();
				u32			uClampedPriority	= uPriority;

				if(uClampedPriority > FS_PRIORITY_LOW)
				{
					SetLastError(FILE_ERROR_PRIORITY);
					uClampedPriority	= FS_PRIORITY_LOW;
				}

				if(pQueueItem)
				{
					//-----------------------------
					// Add this file onto the queue
					//-----------------------------
					pQueueItem->m_nHandle			= uHandle;
					pQueueItem->m_uStatus			= uOperation;
					pQueueItem->m_uOffset			= uOffset;
					pQueueItem->m_uSize				= uSize;
					pQueueItem->m_pDestAddr			= pDest;
					pQueueItem->m_pSrcAddr			= pSrc;

					pQueueItem->m_CallbackFunc		= NULL;
					pQueueItem->m_CallbackFunc2		= NULL;
					pQueueItem->m_CallbackFunc3		= NULL;
					pQueueItem->m_pCallbackClass	= NULL;
					pQueueItem->m_nCallbackID		= 0;

					pQueueItem->m_uPriority			= uClampedPriority;

					return pQueueItem;
				}

				return NULL;
			}

			//------------------------------------------------------------------------------------------

			xbool				AsyncQueueAdd (const EFileQueueStatus uOperation, const u32 uHandle, const u32 uPriority, uintfs uOffset, uintfs uSize, void* pDest, const void* pSrc, AsyncQueueCallBack callbackFunc, AsyncQueueCallBack2 callbackFunc2, void* pClass, s32 nCallbackID)
			{
				QueueItem* item = __AsyncQueueAdd(uOperation, uHandle, uPriority, uOffset, uSize, pDest, pSrc);
				ASSERTS (item, "Async file could not be added to Queue. Not enough room!");

				// Fill in more details
				item->m_CallbackFunc	= callbackFunc;
				item->m_CallbackFunc2	= callbackFunc2;
				item->m_CallbackFunc3	= NULL;
				item->m_pCallbackClass	= pClass;
				item->m_nCallbackID		= nCallbackID;

				// Add to queue
				AsyncQueueAddToTail(item->m_uPriority, item);

				return item!=NULL;
			}

			//------------------------------------------------------------------------------------------

			xbool				AsyncQueueAdd (const EFileQueueStatus uOperation, const u32 uHandle, const u32 uPriority, uintfs uOffset, uintfs uSize, void* pDest, const void* pSrc, AsyncQueueCallBack callbackFunc, AsyncQueueCallBack2 callbackFunc2, AsyncQueueCallBack3 callbackFunc3, void* pClass, s32 nCallbackID)
			{
				QueueItem* item = __AsyncQueueAdd(uOperation, uHandle, uPriority, uOffset, uSize, pDest, pSrc);
				ASSERTS (item, "Async file could not be added to Queue. Not enough room!");

				// Fill in more details
				item->m_CallbackFunc	= callbackFunc;
				item->m_CallbackFunc2	= callbackFunc2;
				item->m_CallbackFunc3	= callbackFunc3;
				item->m_pCallbackClass	= pClass;
				item->m_nCallbackID		= nCallbackID;

				// Add to queue
				AsyncQueueAddToTail(item->m_uPriority, item);

				return item!=NULL;
			}

			//------------------------------------------------------------------------------------------

			xbool				AsyncQueueUpdate (void)
			{
				bool boDone = true;

				//---------------------------------
				// Is the current Head file loaded?
				//---------------------------------
				for(u32 uPriority=0; uPriority<FS_PRIORITY_COUNT; uPriority++)
				{
					if(!boDone)
					{
						break;
					}

					QueueItem* pQueue = AsyncQueueRemoveHead(uPriority);
					if(	pQueue != NULL )
					{
						if( (pQueue->m_uStatus == FILE_QUEUE_OPENING)	||
							(pQueue->m_uStatus == FILE_QUEUE_CLOSING)	||
							(pQueue->m_uStatus == FILE_QUEUE_READING)	||
							(pQueue->m_uStatus == FILE_QUEUE_WRITING)	||
							(pQueue->m_uStatus == FILE_QUEUE_DELETING)	||
							(pQueue->m_uStatus == FILE_QUEUE_STATING)	||
							(pQueue->m_uStatus == FILE_QUEUE_MARKER) )
						{
							//-------------------------
							// Has it finished loading?
							//-------------------------
							if (Sync (FS_SYNC_NOWAIT))
							{
								if (pQueue->m_CallbackFunc != NULL)
								{
									pQueue->m_CallbackFunc (pQueue->m_nHandle, pQueue->m_nCallbackID);
								}

								if (pQueue->m_CallbackFunc2 != NULL)
								{
									pQueue->m_CallbackFunc2 (pQueue->m_nHandle, pQueue->m_pCallbackClass, pQueue->m_nCallbackID);
								}			

								if (pQueue->m_CallbackFunc3 != NULL)
								{
									pQueue->m_CallbackFunc3(pQueue->m_nHandle, pQueue->m_pCallbackClass, pQueue->m_nCallbackID, pQueue->m_pDestAddr, pQueue->m_uSize);
								}			

								pQueue->m_uStatus = FILE_QUEUE_FREE;

								// Put item back to end of free list
								FreeAsyncQueueAddToTail(pQueue);
							}
							else
							{
								boDone = false;

								// Put item back at head of list
								AsyncQueueAddToHead(uPriority, pQueue);
							}
						}
						else
						{
							// Put item back at head of list
							AsyncQueueAddToHead(uPriority, pQueue);
						}
					}

					pQueue = AsyncQueueRemoveHead(uPriority);
					if(pQueue != NULL)
					{
						//------------------------------------------
						// Should we start loading the current file.
						//------------------------------------------
						if (pQueue->m_uStatus == FILE_QUEUE_TO_OPEN)
						{
							boDone = false;

							AsyncOpen( pQueue->m_nHandle );

							pQueue->m_uStatus = FILE_QUEUE_OPENING;
						}
						else if (pQueue->m_uStatus == FILE_QUEUE_TO_CLOSE)
						{
							boDone = false;

							AsyncClose( pQueue->m_nHandle );

							pQueue->m_uStatus = FILE_QUEUE_CLOSING;
						}
						else if (pQueue->m_uStatus == FILE_QUEUE_TO_READ)
						{
							boDone = false;

							AsyncRead( pQueue->m_nHandle, pQueue->m_uOffset, pQueue->m_uSize, pQueue->m_pDestAddr );

							pQueue->m_uStatus = FILE_QUEUE_READING;
						}
						else if (pQueue->m_uStatus == FILE_QUEUE_TO_WRITE)
						{
							boDone = false;

							AsyncWrite( pQueue->m_nHandle, pQueue->m_uOffset, pQueue->m_uSize, pQueue->m_pDestAddr );

							pQueue->m_uStatus = FILE_QUEUE_WRITING;
						}
						else if (pQueue->m_uStatus == FILE_QUEUE_TO_DELETE)
						{
							boDone = false;

							AsyncDelete( pQueue->m_nHandle );

							pQueue->m_uStatus = FILE_QUEUE_DELETING;
						}
						else if (pQueue->m_uStatus == FILE_QUEUE_TO_STAT)
						{
							boDone = false;

							//@TODO: schedule the stat request
							// AsyncStat( pQueue->m_nHandle );

							pQueue->m_uStatus = FILE_QUEUE_STATING;
						}
						else if (pQueue->m_uStatus == FILE_QUEUE_TO_MARKER)
						{
							boDone = false;

							pQueue->m_uStatus = FILE_QUEUE_MARKER;
						}

						if(!boDone)
						{
							// Put item back at head of list
							AsyncQueueAddToHead(uPriority, pQueue);
						}
						else
						{
							// Put item back to end of free list
							FreeAsyncQueueAddToTail(pQueue);
						}
					}
				}

				return boDone;
			}

			//------------------------------------------------------------------------------------------

			xbool  Sync (u32 uFlag)
			{
				bool boDone = true;

				do
				{
					boDone = true;

					for(u32 nSlot = 0; nSlot < FS_MAX_ASYNC_IO_OPS; nSlot++)
					{
						AsyncIOInfo* pOperation = GetAsyncIOData(nSlot);

						if(	( pOperation->m_nStatus == FILE_OP_STATUS_OPEN_PENDING )	||
							( pOperation->m_nStatus == FILE_OP_STATUS_OPENING )			||
							( pOperation->m_nStatus == FILE_OP_STATUS_CLOSE_PENDING )	||
							( pOperation->m_nStatus == FILE_OP_STATUS_CLOSING )			||
							( pOperation->m_nStatus == FILE_OP_STATUS_READ_PENDING )	||
							( pOperation->m_nStatus == FILE_OP_STATUS_READING )			||
							( pOperation->m_nStatus == FILE_OP_STATUS_STAT_PENDING )	||
							( pOperation->m_nStatus == FILE_OP_STATUS_STATING )			||
							( pOperation->m_nStatus == FILE_OP_STATUS_WRITE_PENDING )	||
							( pOperation->m_nStatus == FILE_OP_STATUS_WRITING ))
						{
							boDone	= false;

							if(uFlag != FS_SYNC_WAIT)
							{
								return boDone;
							}
							else
							{
								// ?????? TODO x_Sleep(1);
							}
						}
						else if( pOperation->m_nStatus == FILE_OP_STATUS_DONE )
						{
							pOperation->m_nStatus		= FILE_OP_STATUS_FREE;
							pOperation->m_nFileIndex	= -1;

							FreeAsyncIOAddToTail(pOperation);
						}
					}

				} while(!boDone && uFlag == FS_SYNC_WAIT);

				return boDone;
			}

			//------------------------------------------------------------------------------------------

			void				CreateFileCache		( void )
			{
				void* mem = xfilesystem_heap_alloc(sizeof(xfilecache), 16);
				m_pCache = (xfilecache*)mem;
				xfilecache* filecache = new(m_pCache) xfilecache;
			}

			//------------------------------------------------------------------------------------------

			void				DestroyFileCache	( void )
			{
				m_pCache->~xfilecache();
				xfilesystem_heap_free(m_pCache);
				m_pCache = NULL;
			}

			//------------------------------------------------------------------------------------------

			xfilecache*			GetFileCache		( )
			{
				return m_pCache;
			}

			//------------------------------------------------------------------------------------------

			void				SetLastError		( EError error )
			{
				// Push everything up
				for (s32 i=1; i<FS_MAX_ERROR_ITEMS; ++i)
				{
					m_eLastErrorStack[i-1] = m_eLastErrorStack[i];
				}
				m_eLastErrorStack[FS_MAX_ERROR_ITEMS-1] = error;
			}
		};


		//------------------------------------------------------------------------------------------

		xbool				HasLastError		( void )
		{
			return m_eLastErrorStack[FS_MAX_ERROR_ITEMS-1] != FILE_ERROR_OK;
		}

		//------------------------------------------------------------------------------------------

		void				ClearLastError		( void )
		{
			SetLastError(FILE_ERROR_OK);
		}

		//------------------------------------------------------------------------------------------

		EError				GetLastError		( )
		{
			return m_eLastErrorStack[FS_MAX_ERROR_ITEMS-1];
		}

		//------------------------------------------------------------------------------------------

		const char*			GetLastErrorStr		( )
		{
			switch (GetLastError())
			{
			case FILE_ERROR_OK:
				return "xfilesystem:"TARGET_PLATFORM_STR" ERROR=Ok";
			case FILE_ERROR_NO_FILE:
				return "xfilesystem:"TARGET_PLATFORM_STR" ERROR=File doesn't exist";
			case FILE_ERROR_BADF:
				return "xfilesystem:"TARGET_PLATFORM_STR" ERROR=Bad file descriptor";
			case FILE_ERROR_PRIORITY:
				return "xfilesystem:"TARGET_PLATFORM_STR" ERROR=Wrong priority";
			case FILE_ERROR_MAX_FILES:
				return "xfilesystem:"TARGET_PLATFORM_STR" ERROR=Maximum number of open files reached";
			case FILE_ERROR_MAX_ASYNC:
				return "xfilesystem:"TARGET_PLATFORM_STR" ERROR=Maximum number of async operations reached";
			case FILE_ERROR_DEVICE:
				return "xfilesystem:"TARGET_PLATFORM_STR" ERROR=Unknown device";
			case FILE_ERROR_UNSUP:
				return "xfilesystem:"TARGET_PLATFORM_STR" ERROR=Unsupported operation";
			case FILE_ERROR_CANNOT_MOUNT:
				return "xfilesystem:"TARGET_PLATFORM_STR" ERROR=Mount failed";
			case FILE_ERROR_ASYNC_BUSY:
				return "xfilesystem:"TARGET_PLATFORM_STR" ERROR=Async busy";
			case FILE_ERROR_NOASYNC:
				return "xfilesystem:"TARGET_PLATFORM_STR" ERROR=No async operation has been performed";
			case FILE_ERROR_NOCWD:
				return "xfilesystem:"TARGET_PLATFORM_STR" ERROR=Current directory doesn't exist";
			case FILE_ERROR_NAMETOOLONG:
				return "xfilesystem:"TARGET_PLATFORM_STR" ERROR=Filename too long";
			default:
				return "xfilesystem:"TARGET_PLATFORM_STR" ERROR=Unknown";
			}
		}
	};


	//==============================================================================
	// END xCore namespace
	//==============================================================================
};
