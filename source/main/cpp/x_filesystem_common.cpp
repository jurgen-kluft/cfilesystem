//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase\x_target.h"
#include "xbase\x_debug.h"
#include "xbase\x_allocator.h"
#include "xbase\x_string_std.h"

#include "xfilesystem\private\x_filesystem_common.h"
#include "xfilesystem\private\x_filesystem_spsc_queue.h"
#include "xfilesystem\private\x_filecache.h"

#include "xfilesystem\x_filesystem.h"
#include "xfilesystem\private\x_fileinfo.h"
#include "xfilesystem\private\x_fileasync.h"
#include "xfilesystem\x_filedevice.h"
#include "xfilesystem\x_filepath.h"
#include "xfilesystem\x_alias.h"
#include "xfilesystem\x_iothread.h"

//==============================================================================
// xcore namespace
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
		struct FilenameBuffer
		{
			char		mFilename[FS_MAX_PATH];
		};

		static FilenameBuffer			m_Filenames[FS_MAX_OPENED_FILES];

		static xfileinfo				m_OpenAsyncFile[FS_MAX_OPENED_FILES];
		static xfileasync				m_AsyncIOData[FS_MAX_ASYNC_IO_OPS];
		static QueueItem				m_aAsyncQueue[FS_MAX_ASYNC_QUEUE_ITEMS];

		static EError					m_eLastErrorStack[FS_MAX_ERROR_ITEMS];
		static xfilecache*				m_pCache = NULL;

		static spsc_cqueue<QueueItem*, FS_MAX_ASYNC_QUEUE_ITEMS>*	m_pAsyncQueueList[FS_PRIORITY_COUNT];
		static spsc_cqueue<QueueItem*, FS_MAX_ASYNC_QUEUE_ITEMS>*	m_pFreeQueueItemList = NULL;

		static spsc_cqueue<xfileasync*, FS_MAX_ASYNC_IO_OPS>*		m_pFreeAsyncIOList = NULL;
		static spsc_cqueue<xfileasync*, FS_MAX_ASYNC_IO_OPS>*		m_pAsyncIOList = NULL;

		//------------------------------------------------------------------------------------------

		void				update( void )
		{
			sync(FS_SYNC_NOWAIT);
			asyncQueueUpdate();
		}

		//------------------------------------------------------------------------------------------

		xbool				doesFileExist( const char* szName )
		{
			u32 uHandle = syncOpen(szName, false);
			if (uHandle == (u32)INVALID_FILE_HANDLE)
				return false;
			syncClose(uHandle);
			return true;
		}

		//------------------------------------------------------------------------------------------

		u32					open ( const char* szFilename, xbool boRead, xbool boWrite, xbool boAsync)
		{
			if (boAsync)
				return asyncOpen(szFilename, boRead, boWrite);
			else
				return syncOpen(szFilename, boRead, boWrite);
		}

		//------------------------------------------------------------------------------------------

		void				caps				( u32 uHandle, bool& can_read, bool& can_write, bool& can_seek, bool& can_async )
		{
			if (uHandle == (u32)INVALID_FILE_HANDLE)
			{
				xfileinfo* fileInfo = getFileInfo(uHandle);
				can_read  = fileInfo->m_boReading;
				can_write = fileInfo->m_boWriting;
				can_seek  = fileInfo->m_pFileDevice->canSeek();
				can_async = true;
			}
		}

		//------------------------------------------------------------------------------------------

		uintfs				size				( u32 uHandle )
		{
			return syncSize(uHandle);
		}

		//------------------------------------------------------------------------------------------

		void				read (u32 uHandle, uintfs uOffset, uintfs uSize, void* pBuffer )
		{
			syncRead(uHandle, uOffset, uSize, pBuffer);
		}

		//------------------------------------------------------------------------------------------

		void				write( u32 uHandle, uintfs uOffset, uintfs uSize, const void* pBuffer )
		{
			syncWrite(uHandle, uOffset, uSize, pBuffer);
		}

		//------------------------------------------------------------------------------------------

		void				closeAndDelete( u32& uHandle )
		{
			syncDelete(uHandle);
		}

		//------------------------------------------------------------------------------------------

		void				close ( u32 &uHandle )
		{
			syncClose(uHandle);
		}

		//------------------------------------------------------------------------------------------

		void*				loadAligned (const u32 uAlignment, const char* szFilename, uintfs* puFileSize, const u32 uFlags)
		{
			u32 uHandle = open(szFilename, false);
			if (uHandle == (u32)INVALID_FILE_HANDLE)
			{
				if (puFileSize)
					*puFileSize = 0;

				setLastError(FILE_ERROR_NO_FILE);
				return (NULL);
			}

			xfileinfo* fileInfo = getFileInfo(uHandle);

			u64	u64FileSize = fileInfo->m_uByteSize;

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
				pData = heapAlloc((s32)u64FileSize, uAlignment);
			}
			else
			{
				pData = heapAlloc((s32)u64FileSize, uAlignment);
			}

			read(uHandle, 0, u64FileSize, pData);
			close(uHandle);

			if(	(uFlags & LOAD_FLAGS_CACHE) && (m_pCache != NULL) && (fileInfo->m_pFileDevice != NULL) )
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

		void*				load (const char* szFilename, uintfs* puFileSize, const u32 uFlags)
		{
			u32	uAlignment = FS_MEM_ALIGNMENT;
			if(uFlags & LOAD_FLAGS_VRAM)
				uAlignment	= 128;

			return (loadAligned(uAlignment, szFilename, puFileSize, uFlags));
		}

		//------------------------------------------------------------------------------------------

		void				save (const char* szFilename, const void* pData, uintfs uSize)
		{
			ASSERTS (szFilename, "Save() : Pointer to name is NULL!");
			ASSERTS (pData,	"Save() : Pointer to data is NULL!");

			u32 uHandle = open(szFilename, xTRUE);
			if (uHandle != (u32)INVALID_FILE_HANDLE)
			{
				write(uHandle, 0, uSize, pData);
				close(uHandle);
			}
		}

		//------------------------------------------------------------------------------------------

		xbool				asyncDone ( const u32 uHandle )
		{
			if (uHandle==(u32)INVALID_FILE_HANDLE)
			{
				setLastError(FILE_ERROR_BADF);
				return true;
			}

			bool boDone = true;
			for(u32 nSlot = 0; nSlot < FS_MAX_ASYNC_IO_OPS; nSlot++)
			{
				xfileasync* pOperation = getAsyncIOData(nSlot);
				if (pOperation->getFileIndex()>=0 && (u32)pOperation->getFileIndex()==uHandle)
				{
					boDone	= false;
					break;
				}
			}
			return boDone;
		}

		//------------------------------------------------------------------------------------------

		u32					asyncOpen ( const char* szName, xbool boRead, xbool boWrite )
		{
			u32 uHandle = asyncPreOpen( szName, boRead, boWrite );
			asyncOpen(uHandle);
			return uHandle;
		}

		//------------------------------------------------------------------------------------------

		void				asyncOpen (const u32 uHandle)
		{
			if (uHandle==(u32)INVALID_FILE_HANDLE)
				return;

			xfileasync* pOpen = freeAsyncIOPop();

			if(pOpen == 0)
			{
				ASSERTS(0, "xfilesystem: " TARGET_PLATFORM_STR " ERROR Out of AsyncIO slots");

				setLastError(FILE_ERROR_MAX_ASYNC);

				sync(FS_SYNC_WAIT);
				pOpen = freeAsyncIOPop();
			}

			xfileinfo* pInfo = getFileInfo(uHandle);

			pOpen->setFileIndex			(uHandle);
			pOpen->setStatus			(FILE_OP_STATUS_OPEN_PENDING);
			pOpen->setReadAddress		(NULL);
			pOpen->setWriteAddress		(NULL);
			pOpen->setReadWriteOffset	(0);
			pOpen->setReadWriteSize		(0);

			asyncIOAddToTail(pOpen);
			asyncIOWorkerResume();
		}

		//------------------------------------------------------------------------------------------

		void				asyncRead			( const u32 uHandle, uintfs uOffset, uintfs uSize, void* pBuffer  )
		{
			if (uHandle==(u32)INVALID_FILE_HANDLE)
			{
				setLastError(FILE_ERROR_BADF);
				return;
			}

			xfileasync* pRead = freeAsyncIOPop();

			if(pRead == 0)
			{
				ASSERTS(0, "xfilesystem:" TARGET_PLATFORM_STR " ERROR Out of AsyncIO slots");

				setLastError(FILE_ERROR_MAX_ASYNC);

				sync(FS_SYNC_WAIT);
				pRead = freeAsyncIOPop();
			}

			xfileinfo* pInfo = getFileInfo(uHandle);

			pRead->setFileIndex			(uHandle);
			pRead->setStatus 			(FILE_OP_STATUS_READ_PENDING);
			pRead->setReadAddress		(pBuffer);
			pRead->setWriteAddress		(NULL);
			pRead->setReadWriteOffset	(uOffset);
			pRead->setReadWriteSize		(uSize);

			asyncIOAddToTail(pRead);
			asyncIOWorkerResume();
		}

		//------------------------------------------------------------------------------------------

		void				asyncWrite			( const u32 uHandle, uintfs uOffset, uintfs uSize, const void* pBuffer )
		{
			if (uHandle==(u32)INVALID_FILE_HANDLE)
			{
				setLastError(FILE_ERROR_BADF);
				return;
			}

			xfileasync* pWrite = freeAsyncIOPop();

			if(pWrite == 0)
			{
				ASSERTS(0, "xfilesystem:" TARGET_PLATFORM_STR " ERROR Out of AsyncIO slots");

				setLastError(FILE_ERROR_MAX_ASYNC);

				sync(FS_SYNC_WAIT);
				pWrite = freeAsyncIOPop();
			}

			xfileinfo* pInfo = getFileInfo(uHandle);

			pWrite->setFileIndex		(uHandle);
			pWrite->setStatus 			(FILE_OP_STATUS_WRITE_PENDING);
			pWrite->setReadAddress		(NULL);
			pWrite->setWriteAddress		(pBuffer);
			pWrite->setReadWriteOffset	(uOffset);
			pWrite->setReadWriteSize	(uSize);

			asyncIOAddToTail(pWrite);
			asyncIOWorkerResume();
		}

		//------------------------------------------------------------------------------------------

		void				asyncDelete(const u32 uHandle)
		{
			if (uHandle==(u32)INVALID_FILE_HANDLE)
			{
				setLastError(FILE_ERROR_BADF);
				return;
			}

			xfileasync* pDelete = freeAsyncIOPop();

			if(pDelete == 0)
			{
				ASSERTS(0, "xfilesystem:" TARGET_PLATFORM_STR " ERROR Out of AsyncIO slots");

				setLastError(FILE_ERROR_MAX_ASYNC);

				sync(FS_SYNC_WAIT);
				pDelete = freeAsyncIOPop();
			}

			xfileinfo* pInfo = getFileInfo(uHandle);

			pDelete->setFileIndex		(uHandle);
			pDelete->setStatus 			(FILE_OP_STATUS_DELETE_PENDING);
			pDelete->setReadAddress		(NULL);
			pDelete->setWriteAddress	(NULL);
			pDelete->setReadWriteOffset	(0);
			pDelete->setReadWriteSize	(0);

			asyncIOAddToTail(pDelete);
			asyncIOWorkerResume();
		}

		//------------------------------------------------------------------------------------------

		void				asyncClose (const u32 uHandle)
		{
			if (uHandle==(u32)INVALID_FILE_HANDLE)
			{
				setLastError(FILE_ERROR_BADF);
				return;
			}

			xfileasync* pClose = freeAsyncIOPop();

			if(pClose == 0)
			{
				ASSERTS(0, "xfilesystem:" TARGET_PLATFORM_STR " ERROR Out of AsyncIO slots");

				setLastError(FILE_ERROR_MAX_ASYNC);

				sync(FS_SYNC_WAIT);
				pClose = freeAsyncIOPop();
			}

			xfileinfo* pInfo = getFileInfo(uHandle);

			pClose->setFileIndex		(uHandle);
			pClose->setStatus 			(FILE_OP_STATUS_CLOSE_PENDING);
			pClose->setReadAddress		(NULL);
			pClose->setWriteAddress		(NULL);
			pClose->setReadWriteOffset	(0);
			pClose->setReadWriteSize	(0);

			asyncIOAddToTail(pClose);
			asyncIOWorkerResume();
		}


		//------------------------------------------------------------------------------------------

		u32				asyncPreOpen( const char* szFilename, xbool boRead, xbool boWrite )
		{
			//-----------------------------
			// File find a free file slot.
			//-----------------------------
			u32 uHandle = findFreeFileSlot ();
			if(uHandle == (u32)INVALID_FILE_HANDLE)
			{
				ASSERTS(0, "xfilesystem:" TARGET_PLATFORM_STR " ERROR Too many files opened!");

				setLastError(FILE_ERROR_MAX_FILES);
				return (u32)INVALID_FILE_HANDLE;
			}

			char szFullName[FS_MAX_PATH];
			xfiledevice* device = createSystemPath(szFilename, szFullName, FS_MAX_PATH);

			if (device==NULL)
			{
				setLastError(FILE_ERROR_DEVICE);
				return (u32)INVALID_FILE_HANDLE;
			}

			if (device==NULL || (boWrite && !device->canWrite()))
			{
				ASSERTS(0, "xfilesystem:" TARGET_PLATFORM_STR " ERROR Device is readonly!");

				setLastError(FILE_ERROR_DEVICE_READONLY);
				return (u32)INVALID_FILE_HANDLE;
			}

			xfileinfo* fileInfo = getFileInfo(uHandle);
			fileInfo->m_uByteSize		= 0;

			fileInfo->m_nFileHandle		= (u32)PENDING_FILE_HANDLE;
			fileInfo->m_nFileIndex		= uHandle;

			x_strcpy(fileInfo->m_szFilename, FS_MAX_PATH, szFullName);

			fileInfo->m_boReading		= boRead;
			fileInfo->m_boWriting		= boWrite;
			fileInfo->m_pFileDevice		= device;

			return uHandle;
		}

		//------------------------------------------------------------------------------------------

		xbool asyncQueueOpen( const u32 uHandle, const u32 uPriority, AsyncQueueCallBack callbackFunc, s32 nCallbackID )
		{
			return asyncQueueAdd (FILE_QUEUE_TO_OPEN, uHandle, uPriority, 0, 0, NULL, NULL, callbackFunc, NULL, NULL, nCallbackID);
		}

		//------------------------------------------------------------------------------------------

		xbool asyncQueueOpen( const u32 uHandle, const u32 uPriority, AsyncQueueCallBack2 callbackFunc2, void* pClass, s32 nCallbackID )
		{
			return asyncQueueAdd (FILE_QUEUE_TO_OPEN, uHandle, uPriority, 0, 0, NULL, NULL, NULL, callbackFunc2, pClass, nCallbackID);
		}

		//------------------------------------------------------------------------------------------

		xbool asyncQueueClose( const u32 uHandle, const u32 uPriority, AsyncQueueCallBack callbackFunc, s32 nCallbackID )
		{
			return asyncQueueAdd (FILE_QUEUE_TO_CLOSE, uHandle, uPriority, 0, 0, NULL, NULL, callbackFunc, NULL, NULL, nCallbackID);
		}

		//------------------------------------------------------------------------------------------

		xbool asyncQueueClose( const u32 uHandle, const u32 uPriority, AsyncQueueCallBack2 callbackFunc2, void* pClass, s32 nCallbackID )
		{
			return asyncQueueAdd (FILE_QUEUE_TO_CLOSE, uHandle, uPriority, 0, 0, NULL, NULL, NULL, callbackFunc2, pClass, nCallbackID);
		}

		//------------------------------------------------------------------------------------------

		xbool asyncQueueRead (const u32 uHandle, const u32 uPriority, uintfs uOffset, uintfs uSize, void* pDest, AsyncQueueCallBack callbackFunc, s32 nCallbackID)
		{
			return asyncQueueAdd (FILE_QUEUE_TO_READ, uHandle, uPriority, uOffset, uSize, pDest, NULL, callbackFunc, NULL, NULL, nCallbackID);
		}

		//------------------------------------------------------------------------------------------

		xbool asyncQueueRead (const u32 uHandle, const u32 uPriority, uintfs uOffset, uintfs uSize, void* pDest, AsyncQueueCallBack2 callbackFunc2, void* pClass, s32 nCallbackID)
		{
			return asyncQueueAdd (FILE_QUEUE_TO_READ, uHandle, uPriority, uOffset, uSize, pDest, NULL, NULL, callbackFunc2, pClass, nCallbackID);
		}

		//------------------------------------------------------------------------------------------

		xbool asyncQueueReadAndCache (const u32 uHandle, const u32 uPriority, uintfs uOffset, uintfs uSize, void* pDest, AsyncQueueCallBack callbackFunc, s32 nCallbackID)
		{
			// Is file already cached?
			xfileinfo* fileInfo = getFileInfo(uHandle);
			const xalias* alias = gFindAliasFromFilename(xfilepath(fileInfo->m_szFilename));
			if (alias!=NULL && x_strCompare(alias->alias(), "cache")==0)
			{
				if (asyncIONumFreeSlots() >= 5)
				{
					// No - needs to be cached
					char szFilenameBuffer[FS_MAX_PATH];
					xfilepath szFilename(szFilenameBuffer, sizeof(szFilenameBuffer), fileInfo->m_szFilename);

					const xalias* alias = gFindAndRemoveAliasFromFilename(szFilename);

					asyncQueueAdd (FILE_QUEUE_TO_READ, uHandle, uPriority, uOffset, uSize, pDest, NULL, NULL, NULL, NULL, 0);

					xfilecache* filecache = getFileCache();
					if(	(filecache != NULL) && (filecache->SetCacheData( szFilename.c_str(), pDest, uOffset, uSize, true )) )
					{
						gReplaceAliasOfFilename(szFilename, "cache");
						s32	nWriteHandle = asyncPreOpen(szFilename.c_str(), true);

						asyncQueueAdd (FILE_QUEUE_TO_OPEN,	nWriteHandle, uPriority, uOffset, uSize, pDest, NULL, NULL, NULL, NULL, 0);
						asyncQueueAdd (FILE_QUEUE_TO_WRITE, nWriteHandle, uPriority, uOffset, uSize, pDest, NULL, NULL, NULL, NULL, 0);
						asyncQueueAdd (FILE_QUEUE_TO_CLOSE, nWriteHandle, uPriority, uOffset, uSize, pDest, NULL, NULL, NULL, NULL, 0);

						filecache->WriteCacheIndexFileAsync();
					}

					return asyncQueueAdd (FILE_QUEUE_TO_MARKER, uHandle, uPriority, uOffset, uSize, pDest, NULL, callbackFunc, NULL, NULL, nCallbackID);
				}
				else
				{
					setLastError(FILE_ERROR_MAX_ASYNC);
					return asyncQueueAdd (FILE_QUEUE_TO_READ, uHandle, uPriority, uOffset, uSize, pDest, NULL, callbackFunc, NULL, NULL, nCallbackID);
				}
			}
			else
			{
				return asyncQueueAdd (FILE_QUEUE_TO_READ, uHandle, uPriority, uOffset, uSize, pDest, NULL, callbackFunc, NULL, NULL, nCallbackID);
			}
		}

		//------------------------------------------------------------------------------------------

		xbool asyncQueueReadAndCache (const u32 uHandle, const u32 uPriority, uintfs uOffset, uintfs uSize, void* pDest, AsyncQueueCallBack2 callbackFunc2, void* pClass, s32 nCallbackID)
		{
			// Is file already cached?
			xfilecache* filecache = getFileCache();
			xfileinfo* fileInfo = getFileInfo(uHandle);
			const xalias* alias = gFindAliasFromFilename(xfilepath(fileInfo->m_szFilename));
			if (filecache != NULL && alias!=NULL && x_strCompare(alias->alias(), "cache")==0)
			{
				if ((asyncIONumFreeSlots() >= 5))
				{
					// No - needs to be cached
					char szFilenameBuffer[FS_MAX_PATH];
					xfilepath szFilename(szFilenameBuffer, sizeof(szFilenameBuffer), "");
					szFilename += fileInfo->m_szFilename;

					const xalias* alias = gFindAndRemoveAliasFromFilename(szFilename);

					asyncQueueAdd (FILE_QUEUE_TO_READ, uHandle, uPriority, uOffset, uSize, pDest, NULL, NULL, NULL, NULL, 0);

					if(filecache->SetCacheData(szFilename.c_str(), pDest, uOffset, uSize, true))
					{
						gReplaceAliasOfFilename(szFilename, "cache");
						s32	nWriteHandle = asyncPreOpen(szFilename.c_str(), true);

						asyncQueueAdd (FILE_QUEUE_TO_OPEN,	nWriteHandle, uPriority, uOffset, uSize, pDest, NULL, NULL, NULL, NULL, 0);
						asyncQueueAdd (FILE_QUEUE_TO_WRITE, nWriteHandle, uPriority, uOffset, uSize, pDest, NULL, NULL, NULL, NULL, 0);
						asyncQueueAdd (FILE_QUEUE_TO_CLOSE, nWriteHandle, uPriority, uOffset, uSize, pDest, NULL, NULL, NULL, NULL, 0);

						filecache->WriteCacheIndexFileAsync();
					}

					return asyncQueueAdd (FILE_QUEUE_TO_MARKER, uHandle, uPriority, uOffset, uSize, pDest, NULL, NULL, callbackFunc2, pClass, nCallbackID);
				}
				else
				{
					setLastError(FILE_ERROR_MAX_ASYNC);
					return asyncQueueAdd (FILE_QUEUE_TO_READ, uHandle, uPriority, uOffset, uSize, pDest, NULL, NULL, callbackFunc2, pClass, nCallbackID);
				}
			}
			else
			{
				return asyncQueueAdd (FILE_QUEUE_TO_READ, uHandle, uPriority, uOffset, uSize, pDest, NULL, NULL, callbackFunc2, pClass, nCallbackID);
			}
		}

		//------------------------------------------------------------------------------------------

		xbool asyncQueueDelete( const u32 uHandle, const u32 uPriority, AsyncQueueCallBack callbackFunc, s32 nCallbackID )
		{
			return asyncQueueAdd (FILE_QUEUE_TO_DELETE, uHandle, uPriority, 0, 0, NULL, NULL, callbackFunc, NULL, NULL, nCallbackID);
		}

		//------------------------------------------------------------------------------------------

		xbool asyncQueueDelete( const u32 uHandle, const u32 uPriority, AsyncQueueCallBack2 callbackFunc2, void* pClass, s32 nCallbackID )
		{
			return asyncQueueAdd (FILE_QUEUE_TO_DELETE, uHandle, uPriority, 0, 0, NULL, NULL, NULL, callbackFunc2, pClass, nCallbackID);
		}

		//------------------------------------------------------------------------------------------

		xbool asyncQueueWrite (const u32 uHandle, const u32 uPriority, uintfs uOffset, uintfs uSize, const void* pSrc, AsyncQueueCallBack callbackFunc, s32 nCallbackID)
		{
			return asyncQueueAdd (FILE_QUEUE_TO_WRITE, uHandle, uPriority, uOffset, uSize, NULL, pSrc, callbackFunc, NULL, NULL, nCallbackID);
		}

		//------------------------------------------------------------------------------------------

		xbool asyncQueueWrite (const u32 uHandle, const u32 uPriority, uintfs uOffset, uintfs uSize, const void* pSrc, AsyncQueueCallBack2 callbackFunc2, void* pClass, s32 nCallbackID)
		{
			return asyncQueueAdd (FILE_QUEUE_TO_WRITE, uHandle, uPriority, uOffset, uSize, NULL, pSrc, NULL, callbackFunc2, pClass, nCallbackID);
		}

		//------------------------------------------------------------------------------------------

		void  asyncQueueCancel( const u32 uHandle )
		{
			for(u32 uPriority = 0; uPriority < (FS_PRIORITY_COUNT); uPriority++)
			{
				QueueItem* pItem		= asyncQueueRemoveHead(uPriority);
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

					asyncQueueAddToTail(uPriority, pItem);

					// Terminate the loop when we encounter our start item
					pItem = asyncQueueRemoveHead(uPriority);
					if(pItem == pStartItem)
					{
						asyncQueueAddToHead(uPriority, pItem);
						break;
					}
				}
			}
		}

		//------------------------------------------------------------------------------------------

		void asyncQueueCancel( const u32 uHandle, void* pClass )
		{
			for(u32 uPriority = 0; uPriority < (FS_PRIORITY_COUNT); uPriority++)
			{
				QueueItem* pItem		= asyncQueueRemoveHead(uPriority);
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

					asyncQueueAddToTail(uPriority, pItem);

					// Terminate the loop when we encounter our start item
					pItem = asyncQueueRemoveHead(uPriority);
					if(pItem == pStartItem)
					{
						asyncQueueAddToHead(uPriority, pItem);
						break;
					}
				}
			}
		}

		//------------------------------------------------------------------------------------------

		xbool				asyncQueueDone		( const u32 uHandle )
		{
			// Is the file handle still somewhere in the Queue?
			for(u32 uPriority = 0; uPriority < (FS_PRIORITY_COUNT); uPriority++)
			{
				QueueItem* pItem		= asyncQueueRemoveHead(uPriority);
				QueueItem* pStartItem	= pItem;

				while(pItem)
				{
					if(	(pItem->m_uStatus != FILE_QUEUE_FREE) && (pItem->m_nHandle == uHandle))
					{
						return xFALSE;
					}

					asyncQueueAddToTail(uPriority, pItem);

					// Terminate the loop when we encounter our start item
					pItem = asyncQueueRemoveHead(uPriority);
					if(pItem == pStartItem)
					{
						asyncQueueAddToHead(uPriority, pItem);
						break;
					}
				}
			}
			return xTRUE;
		}


		//------------------------------------------------------------------------------------------

		void			waitUntilIdle(void)
		{
			xbool boDone = asyncQueueUpdate();
			while(!boDone)
			{
				sync(FS_SYNC_WAIT);
				boDone = asyncQueueUpdate();
			}
		}

		//------------------------------------------------------------------------------------------

		u32				findFileHandle		( const char* szName )
		{
			for (u32 i=0; i<FS_MAX_OPENED_FILES; ++i)
			{
				xfileinfo* pInfo = getFileInfo(i);
				if (pInfo->m_nFileHandle != (u32)INVALID_FILE_HANDLE)
				{
					if (x_stricmp(pInfo->m_szFilename, szName) == 0)
						return i;
				}
			}
			return (u32)INVALID_FILE_HANDLE;
		}

	};

	namespace xfilesystem
	{
		//------------------------------------------------------------------------------------------

		static x_iallocator* sAllocator = NULL;

		//------------------------------------------------------------------------------------------

		void*				heapAlloc(s32 size, s32 alignment)
		{
			return sAllocator->allocate(size, alignment);
		}

		//------------------------------------------------------------------------------------------

		void				heapFree(void* mem)
		{
			sAllocator->deallocate(mem);
		}

		void				setAllocator	( x_iallocator* allocator )
		{
			sAllocator = allocator;
		}

		void				initialiseCommon ( u32 uAsyncQueueSize, xbool boEnableCache )
		{
			x_printf ("xfilesystem:"TARGET_PLATFORM_STR" INFO initialise()\n");

			//--------------------------------
			// Clear the error stack
			//--------------------------------
			for (u32 i=0; i<FS_MAX_ERROR_ITEMS; ++i)
				m_eLastErrorStack[i] = FILE_ERROR_OK;

			//-------------------------------------------------------
			// Allocate all the spsc queues aligned on a cache line
			//-------------------------------------------------------
			for (u32 i=0; i<FS_PRIORITY_COUNT; ++i)
			{
				void* mem = heapAlloc(sizeof(spsc_cqueue<QueueItem*, FS_MAX_ASYNC_QUEUE_ITEMS>), CACHE_LINE_SIZE);
				spsc_cqueue<QueueItem*, FS_MAX_ASYNC_QUEUE_ITEMS>* queue = new (mem) spsc_cqueue<QueueItem*, FS_MAX_ASYNC_QUEUE_ITEMS>();
				m_pAsyncQueueList[i] = queue;
			}
			if (m_pFreeQueueItemList == NULL)
			{
				void* mem = heapAlloc(sizeof(spsc_cqueue<QueueItem*, FS_MAX_ASYNC_QUEUE_ITEMS>), CACHE_LINE_SIZE);
				spsc_cqueue<QueueItem*, FS_MAX_ASYNC_QUEUE_ITEMS>* queue = new (mem) spsc_cqueue<QueueItem*, FS_MAX_ASYNC_QUEUE_ITEMS>();
				m_pFreeQueueItemList = queue;
			}
			if (m_pFreeAsyncIOList == NULL)
			{
				void* mem = heapAlloc(sizeof(spsc_cqueue<xfileasync*, FS_MAX_ASYNC_IO_OPS>), CACHE_LINE_SIZE);
				spsc_cqueue<xfileasync*, FS_MAX_ASYNC_IO_OPS>* queue = new (mem) spsc_cqueue<xfileasync*, FS_MAX_ASYNC_IO_OPS>();
				m_pFreeAsyncIOList = queue;
			}
			if (m_pAsyncIOList == NULL)
			{
				void* mem = heapAlloc(sizeof(spsc_cqueue<xfileasync*, FS_MAX_ASYNC_IO_OPS>), CACHE_LINE_SIZE);
				spsc_cqueue<xfileasync*, FS_MAX_ASYNC_IO_OPS>* queue = new (mem) spsc_cqueue<xfileasync*, FS_MAX_ASYNC_IO_OPS>();
				m_pAsyncIOList = queue;
			}

			//--------------------------------
			// Create the Async loading queue.
			//--------------------------------
			ASSERTS (uAsyncQueueSize > 0 && uAsyncQueueSize <= FS_MAX_ASYNC_QUEUE_ITEMS, "initialise() : Async Queue is 0 or to large!");

			//--------------------------------------------------
			// Fill in the status of the first element in queue.
			//--------------------------------------------------
			for (u32 uLoop = 0; uLoop < FS_MAX_ASYNC_QUEUE_ITEMS; uLoop++)
			{
				m_aAsyncQueue[uLoop].clear();
				m_pFreeQueueItemList->push(&(m_aAsyncQueue[uLoop]));
			}
			ASSERT(m_pFreeQueueItemList->full());

			//---------------------------------------
			// Current there is no Async file loaded.
			//---------------------------------------
			for (u32 uFile = 0; uFile < FS_MAX_OPENED_FILES; uFile++)
			{
				m_OpenAsyncFile[uFile].clear();

				m_OpenAsyncFile[uFile].m_szFilename = m_Filenames[uFile].mFilename;
				m_OpenAsyncFile[uFile].m_szFilename[0] = '\0';
				m_OpenAsyncFile[uFile].m_sFilenameMaxLen = sizeof(m_Filenames[uFile].mFilename);
			}

			for(u32 uSlot = 0; uSlot < FS_MAX_ASYNC_IO_OPS; uSlot++)	
			{
				m_AsyncIOData[uSlot].clear();
				m_pFreeAsyncIOList->push(&m_AsyncIOData[uSlot]);
			}
			ASSERT(m_pFreeAsyncIOList->full());

			if (boEnableCache)
				createFileCache();
		}	

		//------------------------------------------------------------------------------------------

		void				shutdownCommon		( void )
		{
			x_printf ("xfilesystem:"TARGET_PLATFORM_STR" INFO shutdown()\n");

			destroyFileCache();

			//-------------------------------------------------------
			// Free all the spsc queues
			//-------------------------------------------------------
			for (u32 i=0; i<FS_PRIORITY_COUNT; ++i)
			{
				m_pAsyncQueueList[i]->~spsc_cqueue<QueueItem*, FS_MAX_ASYNC_QUEUE_ITEMS>();
				heapFree(m_pAsyncQueueList[i]);
				m_pAsyncQueueList[i] = NULL;
			}
			if (m_pFreeQueueItemList != NULL)
			{
				m_pFreeQueueItemList->~spsc_cqueue<QueueItem*, FS_MAX_ASYNC_QUEUE_ITEMS>();
				heapFree(m_pFreeQueueItemList);
				m_pFreeQueueItemList = NULL;

			}
			if (m_pFreeAsyncIOList != NULL)
			{
				m_pFreeAsyncIOList->~spsc_cqueue<xfileasync*, FS_MAX_ASYNC_IO_OPS>();
				heapFree(m_pFreeAsyncIOList);
				m_pFreeAsyncIOList = NULL;
			}
			if (m_pAsyncIOList != NULL)
			{
				m_pAsyncIOList->~spsc_cqueue<xfileasync*, FS_MAX_ASYNC_IO_OPS>();
				heapFree(m_pAsyncIOList);
				m_pAsyncIOList = NULL;
			}
		}

		//------------------------------------------------------------------------------------------
		static x_iothread*	sIoThread = NULL;

		void				setIoThread			( x_iothread* io_thread )
		{
			sIoThread = io_thread;
		}

		//------------------------------------------------------------------------------------------

		xbool				ioThreadLoop		( void )
		{
			return (sIoThread==NULL) ? xFALSE : sIoThread->loop();
		}

		//------------------------------------------------------------------------------------------

		void				ioThreadWait		( void )
		{
			if (sIoThread==NULL) 
				return;
			sIoThread->wait();
		}

		//------------------------------------------------------------------------------------------

		void				ioThreadSignal		( void )
		{
			if (sIoThread==NULL) 
				return;
			sIoThread->signal();
		}


		//------------------------------------------------------------------------------------------

		xfiledevice*			createSystemPath( const char* inFilename, char* outFilename, s32 inFilenameMaxLen )
		{
			xfiledevice* device = NULL;

			xfilepath szFilename(outFilename, inFilenameMaxLen, inFilename);
			const xfilesystem::xalias* alias = xfilesystem::gFindAliasFromFilename(szFilename);

			if (alias != NULL)
			{
				// Remove the device part
				device = alias->device();
				if (alias->remap() != NULL)
				{
					gReplaceAliasOfFilename(szFilename, alias->remap());
				}
				else
				{
					gReplaceAliasOfFilename(szFilename, NULL);
				}
			}
			else
			{
				setLastError(FILE_ERROR_DEVICE);
			}

			//-----------------------------------------------------
			// Fix the slash direction
			//-----------------------------------------------------
			const char c = isPathUNIXStyle() ? '\\' : '/';
			const char w = isPathUNIXStyle() ? '/' : '\\';

			char* src = outFilename;
			while (*src != '\0')
			{
				if (*src == c)
					*src = w;
				++src;
			}

			return device;
		}

		//------------------------------------------------------------------------------------------

		const char*			getFileExtension( const char* szFilename )
		{
			const char* szExtension	= x_strrchr(szFilename, '.');
			if (szExtension)
			{
				return &szExtension[1];
			}
			return NULL;
		}


		//------------------------------------------------------------------------------------------

		xfileinfo*			getFileInfo			( u32 uHandle )
		{
			ASSERT(uHandle<FS_MAX_OPENED_FILES);
			return &m_OpenAsyncFile[uHandle];
		}

		//------------------------------------------------------------------------------------------

		u32					findFreeFileSlot (void)
		{
			for (s32 nSlot = 0; nSlot < FS_MAX_OPENED_FILES; nSlot++)
			{
				if (m_OpenAsyncFile[nSlot].m_nFileHandle == (u32)INVALID_FILE_HANDLE)
					return (nSlot);
			}
			return (u32)-1;
		}

		//------------------------------------------------------------------------------------------

		s32					asyncIONumFreeSlots()
		{
			return m_pFreeQueueItemList->size();
		}

		//------------------------------------------------------------------------------------------

		xfileasync*			getAsyncIOData		( u32 nSlot )
		{
			xfileasync* asyncIOInfo = &m_AsyncIOData[nSlot];
			return asyncIOInfo;
		}

		//------------------------------------------------------------------------------------------

		xfileasync*			freeAsyncIOPop		( void )
		{
			xfileasync* asyncIOInfo;
			if (m_pFreeAsyncIOList->pop(asyncIOInfo))
				return asyncIOInfo;
			else
				return NULL;
		}

		//------------------------------------------------------------------------------------------

		void				freeAsyncIOAddToTail( xfileasync* asyncIOInfo )
		{
			asyncIOInfo->setFileIndex(INVALID_FILE_HANDLE);
			m_pFreeAsyncIOList->push(asyncIOInfo);
		}

		//------------------------------------------------------------------------------------------

		xfileasync*			asyncIORemoveHead	( void )
		{
			xfileasync* item;
			if (m_pAsyncIOList->pop(item))
				return item;
			return NULL;
		}

		//------------------------------------------------------------------------------------------

		void				asyncIOAddToTail	( xfileasync* asyncIOInfo )
		{
			m_pAsyncIOList->push(asyncIOInfo);
		}

		//------------------------------------------------------------------------------------------

		QueueItem*			freeAsyncQueuePop	( )
		{
			QueueItem* item;
			if (m_pFreeQueueItemList->pop(item))
				return item;
			return NULL;
		}

		//------------------------------------------------------------------------------------------

		void				freeAsyncQueueAddToTail( QueueItem* asyncQueueItem )
		{
			m_pFreeQueueItemList->push(asyncQueueItem);
		}

		//------------------------------------------------------------------------------------------

		QueueItem*			asyncQueueRemoveHead( u32 uPriority )
		{
			QueueItem* item;
			if (m_pAsyncQueueList[uPriority]->pop(item))
				return item;
			return NULL;
		}

		//------------------------------------------------------------------------------------------

		void				asyncQueueAddToTail	( u32 uPriority, QueueItem* asyncQueueItem )
		{
			m_pAsyncQueueList[uPriority]->push(asyncQueueItem);
		}

		//------------------------------------------------------------------------------------------

		void				asyncQueueAddToHead	( u32 uPriority, QueueItem* asyncQueueItem )
		{
			m_pAsyncQueueList[uPriority]->push(asyncQueueItem);
		}

		//------------------------------------------------------------------------------------------

		QueueItem*			__asyncQueueAdd (const EFileQueueStatus uOperation, const u32 uHandle, const u32 uPriority, uintfs uOffset, uintfs uSize, void* pDest, const void* pSrc)
		{
			QueueItem*	pQueueItem			= freeAsyncQueuePop();
			u32			uClampedPriority	= uPriority;

			if(uClampedPriority > FS_PRIORITY_LOW)
			{
				setLastError(FILE_ERROR_PRIORITY);
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

		xbool				asyncQueueAdd (const EFileQueueStatus uOperation, const u32 uHandle, const u32 uPriority, uintfs uOffset, uintfs uSize, void* pDest, const void* pSrc, AsyncQueueCallBack callbackFunc, AsyncQueueCallBack2 callbackFunc2, void* pClass, s32 nCallbackID)
		{
			QueueItem* item = __asyncQueueAdd(uOperation, uHandle, uPriority, uOffset, uSize, pDest, pSrc);
			ASSERTS (item, "Async file could not be added to Queue. Not enough room!");

			// Fill in more details
			item->m_CallbackFunc	= callbackFunc;
			item->m_CallbackFunc2	= callbackFunc2;
			item->m_CallbackFunc3	= NULL;
			item->m_pCallbackClass	= pClass;
			item->m_nCallbackID		= nCallbackID;

			// Add to queue
			asyncQueueAddToTail(item->m_uPriority, item);

			return item!=NULL;
		}

		//------------------------------------------------------------------------------------------

		xbool				asyncQueueAdd (const EFileQueueStatus uOperation, const u32 uHandle, const u32 uPriority, uintfs uOffset, uintfs uSize, void* pDest, const void* pSrc, AsyncQueueCallBack callbackFunc, AsyncQueueCallBack2 callbackFunc2, AsyncQueueCallBack3 callbackFunc3, void* pClass, s32 nCallbackID)
		{
			QueueItem* item = __asyncQueueAdd(uOperation, uHandle, uPriority, uOffset, uSize, pDest, pSrc);
			ASSERTS (item, "Async file could not be added to Queue. Not enough room!");

			// Fill in more details
			item->m_CallbackFunc	= callbackFunc;
			item->m_CallbackFunc2	= callbackFunc2;
			item->m_CallbackFunc3	= callbackFunc3;
			item->m_pCallbackClass	= pClass;
			item->m_nCallbackID		= nCallbackID;

			// Add to queue
			asyncQueueAddToTail(item->m_uPriority, item);

			return item!=NULL;
		}

		//------------------------------------------------------------------------------------------

		xbool				asyncQueueUpdate (void)
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

				QueueItem* pQueue = asyncQueueRemoveHead(uPriority);
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
						if (sync (FS_SYNC_NOWAIT))
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
							freeAsyncQueueAddToTail(pQueue);
						}
						else
						{
							boDone = false;

							// Put item back at head of list
							asyncQueueAddToHead(uPriority, pQueue);
						}
					}
					else
					{
						// Put item back at head of list
						asyncQueueAddToHead(uPriority, pQueue);
					}
				}

				pQueue = asyncQueueRemoveHead(uPriority);
				if(pQueue != NULL)
				{
					//------------------------------------------
					// Should we start loading the current file.
					//------------------------------------------
					if (pQueue->m_uStatus == FILE_QUEUE_TO_OPEN)
					{
						boDone = false;

						asyncOpen( pQueue->m_nHandle );

						pQueue->m_uStatus = FILE_QUEUE_OPENING;
					}
					else if (pQueue->m_uStatus == FILE_QUEUE_TO_CLOSE)
					{
						boDone = false;

						asyncClose( pQueue->m_nHandle );

						pQueue->m_uStatus = FILE_QUEUE_CLOSING;
					}
					else if (pQueue->m_uStatus == FILE_QUEUE_TO_READ)
					{
						boDone = false;

						asyncRead( pQueue->m_nHandle, pQueue->m_uOffset, pQueue->m_uSize, pQueue->m_pDestAddr );

						pQueue->m_uStatus = FILE_QUEUE_READING;
					}
					else if (pQueue->m_uStatus == FILE_QUEUE_TO_WRITE)
					{
						boDone = false;

						asyncWrite( pQueue->m_nHandle, pQueue->m_uOffset, pQueue->m_uSize, pQueue->m_pDestAddr );

						pQueue->m_uStatus = FILE_QUEUE_WRITING;
					}
					else if (pQueue->m_uStatus == FILE_QUEUE_TO_DELETE)
					{
						boDone = false;

						asyncDelete( pQueue->m_nHandle );

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
						asyncQueueAddToHead(uPriority, pQueue);
					}
					else
					{
						// Put item back to end of free list
						freeAsyncQueueAddToTail(pQueue);
					}
				}
			}

			return boDone;
		}

		//------------------------------------------------------------------------------------------

		xbool				sync (u32 uFlag)
		{
			bool boDone = true;

			do
			{
				boDone = true;

				for(u32 nSlot = 0; nSlot < FS_MAX_ASYNC_IO_OPS; nSlot++)
				{
					xfileasync* pOperation = getAsyncIOData(nSlot);

					if(	( pOperation->getStatus() == FILE_OP_STATUS_OPEN_PENDING )	||
						( pOperation->getStatus() == FILE_OP_STATUS_OPENING )		||
						( pOperation->getStatus() == FILE_OP_STATUS_CLOSE_PENDING )	||
						( pOperation->getStatus() == FILE_OP_STATUS_CLOSING )		||
						( pOperation->getStatus() == FILE_OP_STATUS_READ_PENDING )	||
						( pOperation->getStatus() == FILE_OP_STATUS_READING )		||
						( pOperation->getStatus() == FILE_OP_STATUS_STAT_PENDING )	||
						( pOperation->getStatus() == FILE_OP_STATUS_STATING )		||
						( pOperation->getStatus() == FILE_OP_STATUS_WRITE_PENDING )	||
						( pOperation->getStatus() == FILE_OP_STATUS_WRITING ))
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
					else if( pOperation->getStatus() == FILE_OP_STATUS_DONE )
					{
						pOperation->setStatus(FILE_OP_STATUS_FREE);
						pOperation->setFileIndex(-1);

						freeAsyncIOAddToTail(pOperation);
					}
				}

			} while(!boDone && uFlag == FS_SYNC_WAIT);

			return boDone;
		}

		//------------------------------------------------------------------------------------------

		void				createFileCache		( void )
		{
			void* mem = heapAlloc(sizeof(xfilecache), 16);
			m_pCache = (xfilecache*)mem;
			xfilecache* filecache = new(m_pCache) xfilecache;
		}

		//------------------------------------------------------------------------------------------

		void				destroyFileCache	( void )
		{
			m_pCache->~xfilecache();
			heapFree(m_pCache);
			m_pCache = NULL;
		}

		//------------------------------------------------------------------------------------------

		xfilecache*			getFileCache		( )
		{
			return m_pCache;
		}

		//------------------------------------------------------------------------------------------

		void				setLastError		( EError error )
		{
			// Push everything up
			for (s32 i=1; i<FS_MAX_ERROR_ITEMS; ++i)
			{
				m_eLastErrorStack[i-1] = m_eLastErrorStack[i];
			}
			m_eLastErrorStack[FS_MAX_ERROR_ITEMS-1] = error;
		}


		//------------------------------------------------------------------------------------------

		xbool					hasLastError		( void )
		{
			return m_eLastErrorStack[FS_MAX_ERROR_ITEMS-1] != FILE_ERROR_OK;
		}

		//------------------------------------------------------------------------------------------

		void					clearLastError		( void )
		{
			setLastError(FILE_ERROR_OK);
		}

		//------------------------------------------------------------------------------------------

		EError					getLastError		( )
		{
			return m_eLastErrorStack[FS_MAX_ERROR_ITEMS-1];
		}

		//------------------------------------------------------------------------------------------

		const char*				getLastErrorStr		( )
		{
			switch (getLastError())
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

		//------------------------------------------------------------------------------------------

		void				asyncIOWorkerResume()
		{
			ioThreadSignal();
		}

		//------------------------------------------------------------------------------------------
		///< Synchronous file operations

		u32					syncOpen			( const char* szName, xbool boRead, xbool boWrite)
		{
			u32 uHandle = asyncPreOpen(szName, boRead, boWrite);
			xfileinfo* pxFileInfo = getFileInfo(uHandle);
			u32 nFileHandle;
			if (!pxFileInfo->m_pFileDevice->openOrCreateFile((u32)pxFileInfo->m_nFileIndex, pxFileInfo->m_szFilename, boRead, boWrite, nFileHandle))
			{
				x_printf ("device->openOrCreateFile failed on file %s\n", x_va_list(pxFileInfo->m_szFilename));
				pxFileInfo->clear();
				uHandle = (u32)INVALID_FILE_HANDLE;
			}
			else
			{
				pxFileInfo->m_nFileHandle = nFileHandle;
			}
			return uHandle;
		}

		//------------------------------------------------------------------------------------------

		uintfs				syncSize			( u32 uHandle )
		{
			if (uHandle==(u32)INVALID_FILE_HANDLE)
				return 0;

			xfileinfo* pxFileInfo = getFileInfo(uHandle);
			u64 length;
			if (!pxFileInfo->m_pFileDevice->lengthOfFile(pxFileInfo->m_nFileHandle, length))
			{
				x_printf ("device->lengthOfFile failed on file %s\n", x_va_list(pxFileInfo->m_szFilename));
			}
			return length;
		}

		//------------------------------------------------------------------------------------------

		void				syncRead			( u32 uHandle, uintfs uOffset, uintfs uSize, void* pBuffer )
		{
			if (uHandle==(u32)INVALID_FILE_HANDLE)
				return;

			xfileinfo* pxFileInfo = getFileInfo(uHandle);
			u64 numBytesRead;
			if (!pxFileInfo->m_pFileDevice->readFile(pxFileInfo->m_nFileHandle, uOffset, pBuffer, uSize, numBytesRead))
			{
				x_printf ("device->ReadFile failed on file %s\n", x_va_list(pxFileInfo->m_szFilename));
			}
		}

		//------------------------------------------------------------------------------------------

		void				syncWrite			( u32 uHandle, uintfs uOffset, uintfs uSize, const void* pBuffer )
		{
			if (uHandle==(u32)INVALID_FILE_HANDLE)
				return;

			xfileinfo* pxFileInfo = getFileInfo(uHandle);
			u64 numBytesWritten;
			if (!pxFileInfo->m_pFileDevice->writeFile(pxFileInfo->m_nFileHandle, uOffset, pBuffer, uSize, numBytesWritten))
			{
				x_printf ("device->WriteFile failed on file %s\n", x_va_list(pxFileInfo->m_szFilename));
			}
		}

		void				syncFlush			( u32 uHandle )
		{
			if (uHandle==(u32)INVALID_FILE_HANDLE)
				return;
		}

		//------------------------------------------------------------------------------------------

		void 				syncClose			( u32& uHandle )
		{
			if (uHandle==(u32)INVALID_FILE_HANDLE)
				return;

			xfileinfo* pxFileInfo = getFileInfo(uHandle);
			if (!pxFileInfo->m_pFileDevice->closeFile(pxFileInfo->m_nFileHandle))
			{
				x_printf ("device->CloseFile failed on file %s\n", x_va_list(pxFileInfo->m_szFilename));
			}
			pxFileInfo->clear();
			uHandle = (u32)INVALID_FILE_HANDLE;
		}

		//------------------------------------------------------------------------------------------

		void				syncDelete			( u32& uHandle )
		{
			if (uHandle==(u32)INVALID_FILE_HANDLE)
				return;

			xfileinfo* pxFileInfo = getFileInfo(uHandle);
			if (!pxFileInfo->m_pFileDevice->closeFile(pxFileInfo->m_nFileHandle))
			{
				x_printf ("device->CloseFile failed on file %s\n", x_va_list(pxFileInfo->m_szFilename));
			}
			pxFileInfo->m_pFileDevice->deleteFile(pxFileInfo->m_nFileHandle, pxFileInfo->m_szFilename);
			pxFileInfo->clear();
			uHandle = (u32)INVALID_FILE_HANDLE;
		}
	};


	//==============================================================================
	// END xcore namespace
	//==============================================================================
};
