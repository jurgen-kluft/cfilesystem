//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase\x_target.h"
#include "xbase\x_debug.h"
#include "xbase\x_allocator.h"
#include "xbase\x_string_std.h"

#include "xfilesystem\private\x_filesystem_common.h"
#include "xfilesystem\private\x_filesystem_cqueue.h"
#include "xfilesystem\private\x_devicealias.h"

#include "xfilesystem\x_filedevice.h"
#include "xfilesystem\x_filepath.h"
#include "xfilesystem\x_filesystem.h"
#include "xfilesystem\x_threading.h"
#include "xfilesystem\x_stream.h"
#include "xfilesystem\private\x_filedata.h"
#include "xfilesystem\private\x_fileasync.h"

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

	/*
		The user can create streams on any thread and this means that the file system will be
		accessed in a concurrent way. So this is why all the objects are managed by a concurrent
		queue.

		The last error feature should be for every xfiledata and for the global file system.
	*/

	namespace xfilesystem
	{
		//------------------------------------------------------------------------------------------

		// allocator stuff
		static x_iallocator*				sAllocator = NULL;


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

		x_iallocator*			getAllocator		( void )
		{
			return sAllocator;
		}


		//------------------------------------------------------------------------------------------

		x_iallocator* xfs_common::singleton_allocator()
		{
			return sAllocator;
		}
		u32    xfs_common::singleton_alignment()
		{
			return 4;
		}


		//------------------------------------------------------------------------------------------

		xbool				xfs_common::exists ( const char* szName )
		{
			s32 uHandle = syncOpen(szName, false);
			if (uHandle == INVALID_FILE_HANDLE)
				return false;
			syncClose(uHandle);
			return true;
		}

		//------------------------------------------------------------------------------------------

		s32					xfs_common::open ( const char* szFilename, xbool boRead, xbool boWrite, x_asyncio_callback_struct callback)
		{
			if (callback.callback != NULL)
				return asyncOpen(szFilename, boRead, boWrite, callback);
			else
				return syncOpen(szFilename, boRead, boWrite);
		}

		//------------------------------------------------------------------------------------------

		xbool				xfs_common::caps ( const xfilepath& szFilename, bool& can_read, bool& can_write, bool& can_seek, bool& can_async )
		{
			const xdevicealias* alias = xdevicealias::sFind(szFilename);
			if (alias == NULL)
				return xFALSE;

			const xfiledevice* file_device = alias->device();

			can_read  = true;
			can_write = file_device->canWrite();
			can_seek  = file_device->canSeek();
			can_async = true;
			return xTRUE;
		}

		//------------------------------------------------------------------------------------------

		void				xfs_common::setLength ( s32 uHandle, u64 uFileSize )
		{
			syncSetSize(uHandle, uFileSize);
		}

		//------------------------------------------------------------------------------------------

		u64					xfs_common::getLength ( s32 uHandle )
		{
			return syncGetSize(uHandle);
		}

		//------------------------------------------------------------------------------------------

		u64					xfs_common::read (s32 uHandle, u64 uOffset, u64 uSize, void* pBuffer, x_asyncio_callback_struct callback )
		{
			if (callback.callback != NULL)
			{
				asyncRead(uHandle, uOffset, uSize, pBuffer, callback);
				return 0;
			}
			else
			{
				return syncRead(uHandle, uOffset, uSize, pBuffer);
			}
		}

		//------------------------------------------------------------------------------------------

		u64					xfs_common::write ( s32 uHandle, u64 uOffset, u64 uSize, const void* pBuffer, x_asyncio_callback_struct callback )
		{
			if (callback.callback != NULL)
			{
				asyncWrite(uHandle, uOffset, uSize, pBuffer, callback);
				return 0;
			}
			else
			{
				return syncWrite(uHandle, uOffset, uSize, pBuffer);
			}
		}

		//------------------------------------------------------------------------------------------

		u64					xfs_common::getpos ( s32 uHandle )
		{
			return syncGetPos(uHandle);
		}

		//------------------------------------------------------------------------------------------

		u64					xfs_common::setpos ( s32 uHandle, u64 uFilePos )
		{
			return syncSetPos(uHandle, uFilePos);
		}

		//------------------------------------------------------------------------------------------

		void				xfs_common::closeAndDelete ( s32& uHandle, x_asyncio_callback_struct callback )
		{
			if (callback.callback != NULL)
				asyncCloseAndDelete(uHandle, callback);
			else
				syncDelete(uHandle);
		}

		//------------------------------------------------------------------------------------------

		void				xfs_common::close ( s32 &uHandle, x_asyncio_callback_struct callback)
		{
			if (callback.callback != NULL)
				asyncClose(uHandle, callback);
			else
				syncClose(uHandle);
		}

		//------------------------------------------------------------------------------------------

		void				xfs_common::save (const char* szFilename, const void* pData, u64 uSize)
		{
			ASSERTS (szFilename, "Save() : Pointer to name is NULL!");
			ASSERTS (pData,	"Save() : Pointer to data is NULL!");

			s32 uHandle = open(szFilename, xTRUE);
			if (uHandle != INVALID_FILE_HANDLE)
			{
				write(uHandle, 0, uSize, pData);
				close(uHandle);
			}
		}

		s32				xfs_common::createFile( const char* szFilename, xbool boRead, xbool boWrite)
		{
			ASSERTS (szFilename, "Create() : Pointer to name is NULL!");

			return syncCreate(szFilename, boRead, boWrite);
		}


		//------------------------------------------------------------------------------------------
		/*
		bool				xfs_common::asyncCompleted ( s32 uHandle, xasync_id const& uAsync )
		{
			if (uHandle==INVALID_FILE_HANDLE)
			{
				setLastError(FILE_ERROR_BADF);
				return true;
			}

			bool is_inside = m_pAsyncIOList->inside(uAsync);
			return !is_inside;
		}
		*/

		//------------------------------------------------------------------------------------------

		s32					xfs_common::asyncOpen ( const char* szName, xbool boRead, xbool boWrite, x_asyncio_callback_struct callback )
		{
			s32 uHandle = asyncPreOpen( szName, boRead, boWrite );
			asyncOpen(uHandle, callback);
			return uHandle;
		}

		//------------------------------------------------------------------------------------------

		void				xfs_common::asyncOpen (const s32 uHandle, x_asyncio_callback_struct callback)
		{
			if (uHandle==INVALID_FILE_HANDLE)
				return;

			xfileasync* pOpen = popFreeAsyncIO(false);
			if (pOpen == 0)
			{
				ASSERTS(0, "xfilesystem: " TARGET_PLATFORM_STR " ERROR Out of AsyncIO slots");

				setLastError(FILE_ERROR_MAX_ASYNC);
				pOpen = popFreeAsyncIO(true);
			}

			pOpen->setFileIndex			(uHandle);
			pOpen->setStatus			(FILE_OP_STATUS_OPEN_PENDING);
			pOpen->setReadAddress		(NULL);
			pOpen->setWriteAddress		(NULL);
			pOpen->setReadWriteOffset	(0);
			pOpen->setReadWriteSize		(0);
			pOpen->setCallback			(callback);

			/*
			xevent* async_event = getEventFactory()->construct();
			async_event->set();
			pOpen->setEvent(async_event);
			*/

			xasync_id id = pushAsyncIO(pOpen);
			
			/*
			if (pAsyncId!=NULL) 
			{

				xiasync_result_imp* async_result = reinterpret_cast<xiasync_result_imp*>(popAsyncResult());
				async_result->init(id, uHandle, async_event);
				*pAsyncId = async_result;
				(*pAsyncId)->destroy();
			}
			*/


			getIoThreadInterface()->signal();
		}

		//------------------------------------------------------------------------------------------

		void				xfs_common::asyncRead			( const s32 uHandle, u64 uOffset, u64 uSize, void* pBuffer, x_asyncio_callback_struct callback )
		{
			if (uHandle==INVALID_FILE_HANDLE)
			{
				setLastError(FILE_ERROR_BADF);
				return;
			}

			xfileasync* pRead = popFreeAsyncIO(false);
			if(pRead == 0)
			{
				ASSERTS(0, "xfilesystem:" TARGET_PLATFORM_STR " ERROR Out of AsyncIO slots");

				setLastError(FILE_ERROR_MAX_ASYNC);
				pRead = popFreeAsyncIO(true);
			}

			xfiledata* pInfo = getFileInfo(uHandle);

			pRead->setFileIndex			(pInfo->m_nFileIndex);
			pRead->setStatus 			(FILE_OP_STATUS_READ_PENDING);
			pRead->setReadAddress		(pBuffer);
			pRead->setWriteAddress		(NULL);
			pRead->setReadWriteOffset	(uOffset);
			pRead->setReadWriteSize		(uSize);
			pRead->setCallback			(callback);

			/*
			xevent* async_event = getEventFactory()->construct();
			async_event->set();
			pRead->setEvent(async_event);
			*/

			xasync_id id = pushAsyncIO(pRead);
			/*
			if (pAsyncId!=NULL) 
			{


				xiasync_result_imp* async_result = reinterpret_cast<xiasync_result_imp*>(popAsyncResult());
				async_result->init(id, uHandle, async_event);
				*pAsyncId = async_result;
				(*pAsyncId)->destroy();
			}
			*/

			getIoThreadInterface()->signal();
		}

		//------------------------------------------------------------------------------------------

		void				xfs_common::asyncWrite			( const s32 uHandle, u64 uOffset, u64 uSize, const void* pBuffer, x_asyncio_callback_struct callback )
		{
			if (uHandle==INVALID_FILE_HANDLE)
			{
				setLastError(FILE_ERROR_BADF);
				return;
			}

			xfileasync* pWrite = popFreeAsyncIO(false);
			if(pWrite == 0)
			{
				ASSERTS(0, "xfilesystem:" TARGET_PLATFORM_STR " ERROR Out of AsyncIO slots");

				setLastError(FILE_ERROR_MAX_ASYNC);
				pWrite = popFreeAsyncIO(true);
			}

			xfiledata* pInfo = getFileInfo(uHandle);

			pWrite->setFileIndex		(uHandle);
			pWrite->setStatus 			(FILE_OP_STATUS_WRITE_PENDING);
			pWrite->setReadAddress		(NULL);
			pWrite->setWriteAddress		(pBuffer);
			pWrite->setReadWriteOffset	(uOffset);
			pWrite->setReadWriteSize	(uSize);
			pWrite->setCallback			(callback);

			/*
			xevent* async_event = getEventFactory()->construct();
			async_event->set();
			pWrite->setEvent(async_event);
			*/

			xasync_id id = pushAsyncIO(pWrite);
			
			/*
			if (pAsyncId!=NULL) 
			{


				xiasync_result_imp* async_result = reinterpret_cast<xiasync_result_imp*>(popAsyncResult());
				async_result->init(id, uHandle, async_event);
				*pAsyncId = async_result;
				(*pAsyncId)->destroy();
			}
			*/

			getIoThreadInterface()->signal();
		}

		//------------------------------------------------------------------------------------------

		void				xfs_common::asyncCloseAndDelete ( const s32 uHandle, x_asyncio_callback_struct callback )
		{
			if (uHandle==INVALID_FILE_HANDLE)
			{
				setLastError(FILE_ERROR_BADF);
				return;
			}

			xfileasync* pDelete = popFreeAsyncIO(false);
			if (NULL == pDelete)
			{
				ASSERTS(0, "xfilesystem:" TARGET_PLATFORM_STR " ERROR Out of AsyncIO slots");

				setLastError(FILE_ERROR_MAX_ASYNC);
				pDelete = popFreeAsyncIO(true);
			}

			pDelete->setFileIndex		(uHandle);
			pDelete->setStatus 			(FILE_OP_STATUS_DELETE_PENDING);
			pDelete->setReadAddress		(NULL);
			pDelete->setWriteAddress	(NULL);
			pDelete->setReadWriteOffset	(0);
			pDelete->setReadWriteSize	(0);
			pDelete->setCallback		(callback);



			xasync_id id = pushAsyncIO(pDelete);


			getIoThreadInterface()->signal();
		}

		//------------------------------------------------------------------------------------------

		void				xfs_common::asyncClose ( const s32 uHandle, x_asyncio_callback_struct callback )
		{
			if (uHandle==INVALID_FILE_HANDLE)
			{
				setLastError(FILE_ERROR_BADF);
				return;
			}

			xfileasync* pClose = popFreeAsyncIO(false);
			if (pClose == 0)
			{
				ASSERTS(0, "xfilesystem:" TARGET_PLATFORM_STR " ERROR Out of AsyncIO slots");

				setLastError(FILE_ERROR_MAX_ASYNC);

				pClose = popFreeAsyncIO(true);
			}

			pClose->setFileIndex		(uHandle);
			pClose->setStatus 			(FILE_OP_STATUS_CLOSE_PENDING);
			pClose->setReadAddress		(NULL);
			pClose->setWriteAddress		(NULL);
			pClose->setReadWriteOffset	(0);
			pClose->setReadWriteSize	(0);
			pClose->setCallback			(callback);


			xasync_id id = pushAsyncIO(pClose);

			getIoThreadInterface()->signal();
		}


		//------------------------------------------------------------------------------------------

		s32				xfs_common::asyncPreOpen( const char* szFilename, xbool boRead, xbool boWrite )
		{
			//-----------------------------
			// File find a free file slot.
			//-----------------------------
			s32 uHandle = popFreeFileSlot ();
			if(uHandle == INVALID_FILE_HANDLE)
			{
				ASSERTS(0, "xfilesystem:" TARGET_PLATFORM_STR " ERROR Too many files opened!");

				setLastError(FILE_ERROR_MAX_FILES);
				return INVALID_FILE_HANDLE;
			}

			char szSystemFilenameBuffer[FS_MAX_PATH + 2];
			xcstring szSystemFilename(szSystemFilenameBuffer, sizeof(szSystemFilenameBuffer));
			xfiledevice* device = createSystemPath(szFilename, szSystemFilename);

			if (device==NULL)
			{
				setLastError(FILE_ERROR_DEVICE);
				return INVALID_FILE_HANDLE;
			}

			if (boWrite && !device->canWrite())
			{
				ASSERTS(0, "xfilesystem:" TARGET_PLATFORM_STR " ERROR Device is read-only!");

				setLastError(FILE_ERROR_DEVICE_READONLY);
				return INVALID_FILE_HANDLE;
			}

			xfiledata* fileInfo = getFileInfo(uHandle);
			fileInfo->m_uByteSize		= 0;
			fileInfo->m_nFileHandle		= (u32)PENDING_FILE_HANDLE;

			xcstring fileInfoFilename(fileInfo->m_szFilename, fileInfo->m_sFilenameMaxLen, szSystemFilename.c_str());

			fileInfo->m_boReading		= boRead;
			fileInfo->m_boWriting		= boWrite;
			fileInfo->m_pFileDevice		= device;

			return uHandle;
		}

	}

	namespace xfilesystem
	{
		//------------------------------------------------------------------------------------------


		void				xfs_common::initialiseCommon ( u32 uMaxOpenStreams )
		{

			ASSERT(mInitCommonComplete != 1);
			ASSERT(this != NULL);

			x_printf ("xfilesystem: %s INFO initialise()\n", TARGET_PLATFORM_STR);

			m_uMaxOpenedFiles = uMaxOpenStreams;

			if (m_Filenames == NULL)
			{
				m_Filenames = (char**)heapAlloc(m_uMaxOpenedFiles * sizeof(char*), X_ALIGNMENT_DEFAULT);
				for (u32 uFile = 0; uFile < m_uMaxOpenedFiles; uFile++)
				{
					char* filename = (char*)heapAlloc(FS_MAX_PATH + 2, X_ALIGNMENT_DEFAULT);
					x_memset(filename, '\0', FS_MAX_PATH + 2);
					m_Filenames[uFile] = filename;
				}
			}

			if (m_OpenAsyncFileArray == NULL)
			{
				m_OpenAsyncFileArray = (xfiledata*)heapAlloc(m_uMaxOpenedFiles * sizeof(xfiledata), X_ALIGNMENT_DEFAULT);
				for (u32 uFile = 0; uFile < m_uMaxOpenedFiles; uFile++)
				{
					new (&m_OpenAsyncFileArray[uFile]) xfiledata();
					m_OpenAsyncFileArray[uFile].clear();
					m_OpenAsyncFileArray[uFile].m_nFileIndex = uFile;
					m_OpenAsyncFileArray[uFile].m_szFilename = m_Filenames[uFile];
					m_OpenAsyncFileArray[uFile].m_sFilenameMaxLen = FS_MAX_PATH;
				}
			}

			if (m_FreeAsyncFile == NULL)
			{
				void* mem1 = heapAlloc(sizeof(cqueue<xfiledata*>), X_CACHE_LINE_SIZE);
				cqueue<xfiledata*>* queue = new (mem1) cqueue<xfiledata*>();
				queue->init(sAllocator, m_uMaxOpenedFiles);
				
				m_FreeAsyncFile = queue;
				// Populate the unused xfiledata queue
				for (u32 uFile = 0; uFile < m_uMaxOpenedFiles; uFile++)
				{
					u32 _idx;
					m_FreeAsyncFile->push(&m_OpenAsyncFileArray[uFile], _idx);
				}
				ASSERT(m_FreeAsyncFile->full());
			}
			
			if (m_AsyncIOData == NULL)
			{
				m_AsyncIOData = (xfileasync*)heapAlloc(m_uMaxAsyncOperations * sizeof(xfileasync), X_ALIGNMENT_DEFAULT);
				for (u32 uFile = 0; uFile < m_uMaxAsyncOperations; uFile++)
					new (&m_AsyncIOData[uFile]) xfileasync();
			}



			//-------------------------------------------------------
			// Allocate all the concurrent queues aligned on a cache line
			//-------------------------------------------------------
			if (m_pFreeAsyncIOList == NULL)
			{
				void* mem1 = heapAlloc(sizeof(cqueue<xfileasync*>), X_CACHE_LINE_SIZE);
				cqueue<xfileasync*>* queue = new (mem1) cqueue<xfileasync*>();
				queue->init(sAllocator, m_uMaxAsyncOperations);
				
				m_pFreeAsyncIOList = queue;

				for(u32 uSlot = 0; uSlot < m_uMaxAsyncOperations; uSlot++)	
				{
					m_AsyncIOData[uSlot].clear();
					u32 _idx;
					m_pFreeAsyncIOList->push(&m_AsyncIOData[uSlot], _idx);
				}
				ASSERT(m_pFreeAsyncIOList->full());
			}
			if (m_pAsyncIOList == NULL)
			{
				void* mem1 = heapAlloc(sizeof(cqueue<xfileasync*>), X_CACHE_LINE_SIZE);
				cqueue<xfileasync*>* queue = new (mem1) cqueue<xfileasync*>();
				queue->init(sAllocator, m_uMaxAsyncOperations);
				m_pAsyncIOList = queue;
			}
	
			if (m_pLastErrorStack == NULL)
			{
				void* mem1 = heapAlloc(sizeof(cqueue<xfileasync*>), X_CACHE_LINE_SIZE);
				cqueue<u32>* queue = new (mem1) cqueue<u32>();
				queue->init(sAllocator, m_uMaxErrorItems);

				m_pLastErrorStack = queue;
			}

			mInitCommonComplete = 1;

		}	

		//------------------------------------------------------------------------------------------

		void				xfs_common::shutdownCommon		( void )
		{

			ASSERT(mInitCommonComplete == 1);

			x_printf ("xfilesystem:%s INFO shutdown()\n", TARGET_PLATFORM_STR);

			//-------------------------------------------------------
			// Free all the concurrent queues
			//-------------------------------------------------------
			if (m_pFreeAsyncIOList != NULL)
			{
				m_pFreeAsyncIOList->clear(sAllocator);
				m_pFreeAsyncIOList->~cqueue<xfileasync*>();
				heapFree(m_pFreeAsyncIOList);
				m_pFreeAsyncIOList = NULL;
			}

			if (m_pAsyncIOList != NULL)
			{
				m_pAsyncIOList->clear(sAllocator);
				m_pAsyncIOList->~cqueue<xfileasync*>();
				heapFree(m_pAsyncIOList);
				m_pAsyncIOList = NULL;
				heapFree(m_AsyncIOData);
				m_AsyncIOData = NULL;
			}


			if (m_FreeAsyncFile != NULL)
			{
				m_FreeAsyncFile->clear(sAllocator);
				m_FreeAsyncFile->~cqueue<xfiledata*>();
				heapFree(m_FreeAsyncFile);
				m_FreeAsyncFile = NULL;
				heapFree(m_OpenAsyncFileArray);
				m_OpenAsyncFileArray = NULL;
			}

			if (m_Filenames != NULL)
			{
				for (u32 uFile = 0; uFile < m_uMaxOpenedFiles; uFile++)
					heapFree(m_Filenames[uFile]);
				heapFree(m_Filenames);
				m_Filenames = NULL;
			}

			if (m_pLastErrorStack!= NULL)
			{
				m_pLastErrorStack->clear(sAllocator);
				m_pLastErrorStack->~cqueue<u32>();
				heapFree(m_pLastErrorStack);
				m_pLastErrorStack = NULL;
			}

			mInitCommonComplete = 0;
		}

		

		void					xfs_common::setIoThreadInterface( xio_thread* io_thread )
		{
			mIoThread = io_thread;
		}

		xio_thread*				xfs_common::getIoThreadInterface()
		{
			return mIoThread;
		}



		//------------------------------------------------------------------------------------------

		xbool					xfs_common::isPathUNIXStyle(void)
		{
			return xfilesystem::isPathUNIXStyle();
		}

		xfiledevice*			xfs_common::createSystemPath( const char* inFilename, xcstring& outFilename )
		{
			xfilepath szFilename(inFilename);
			const xfilesystem::xdevicealias* alias = xdevicealias::sFind(szFilename);
			if (alias == NULL)
			{
				// Take the working directory
				alias = xdevicealias::sFind("curdir");
			}

			// Remove the device part and replace it with the remap part of the file device that matches the
			// device part of the filename.
			xfiledevice* device = alias->device();
			szFilename.setDevicePart(alias->remap());
			outFilename = szFilename.c_str_device();
			// app_home:/dirname/filename.txt  (remove char ':' )
			// right format is app_home/dirname/filename.txt
			//if(xfs_common::s_instance()->isPathUNIXStyle())
			//	outFilename.remove(":");

			return device;
		}

		//------------------------------------------------------------------------------------------

		xfiledata*				xfs_common::getFileInfo			( s32 uHandle )
		{
			ASSERT(uHandle<m_uMaxOpenedFiles);
			return &m_OpenAsyncFileArray[uHandle];
		}

		//------------------------------------------------------------------------------------------

		s32						xfs_common::popFreeFileSlot			(void)
		{
			xfiledata* f;
			if (m_FreeAsyncFile->pop(f))
				return f->m_nFileIndex;
			return -1;
		}

		//------------------------------------------------------------------------------------------

		bool					xfs_common::pushFreeFileSlot		(s32 nFileIndex)
		{

			ASSERT(nFileIndex < m_uMaxOpenedFiles);
			xfiledata* f = &m_OpenAsyncFileArray[nFileIndex];
			xasync_id _id;
			if (m_FreeAsyncFile->push(f, _id))
				return true;
			return false;
		}

		//------------------------------------------------------------------------------------------

		xfileasync*				xfs_common::getAsyncIOData			( u32 nSlot )
		{
			xfileasync* asyncIOInfo = &m_AsyncIOData[nSlot];
			return asyncIOInfo;
		}

		//------------------------------------------------------------------------------------------

		xfileasync*				xfs_common::popFreeAsyncIO			( bool wait )
		{
			do
			{
				xfileasync* asyncIOInfo;
				if (m_pFreeAsyncIOList->pop(asyncIOInfo))
				{
					ASSERT(asyncIOInfo != NULL);
					asyncIOInfo->clear();
					return asyncIOInfo;
				}
				
			} while (wait);

			return NULL;
		}

		//------------------------------------------------------------------------------------------

		void					xfs_common::pushFreeAsyncIO			( xfileasync* asyncIOInfo )
		{
			ASSERT(asyncIOInfo != NULL);
			asyncIOInfo->setFileIndex(INVALID_FILE_HANDLE);
			u32 idx;
			bool queueSuccess = m_pFreeAsyncIOList->push(asyncIOInfo, idx);
			
			ASSERT(queueSuccess); // make sure we can actually push on the queue
		
		}

		//------------------------------------------------------------------------------------------

		xfileasync*				xfs_common::popAsyncIO				( void )
		{
			xfileasync* item;
			if (m_pAsyncIOList->pop(item))
				return item;
			return NULL;
		}

		//------------------------------------------------------------------------------------------

		xasync_id				xfs_common::pushAsyncIO				( xfileasync* asyncIOInfo )
		{
			u32 _idx;

			ASSERT(asyncIOInfo != NULL);

			bool queueSuccess = m_pAsyncIOList->push(asyncIOInfo, _idx);

			ASSERT(queueSuccess); // make sure we can actually push on the queue

			return _idx;
		}

		//------------------------------------------------------------------------------------------

		s32						xfs_common::testAsyncId			( xasync_id id )
		{
			if (m_pAsyncIOList->inside(id))
				return 0;	// In the processing queue
			else 
				return -1;	// Has been processed
		}

		//------------------------------------------------------------------------------------------

		void					xfs_common::setLastError		( EError error )
		{
			u32 old_error;
			u32 _idx;
			while (!m_pLastErrorStack->push((u32)error, _idx))
				m_pLastErrorStack->pop(old_error);
		}

		//------------------------------------------------------------------------------------------

		xbool					xfs_common::hasLastError		( void )
		{
			u32 lastError = FILE_ERROR_OK;
			if (m_pLastErrorStack->pop(lastError))
				return lastError != FILE_ERROR_OK;
			return false;
		}

		//------------------------------------------------------------------------------------------

		void					xfs_common::clearLastError		( void )
		{
			u32 lastError;
			while (!m_pLastErrorStack->empty())
				m_pLastErrorStack->pop(lastError);
		}

		//------------------------------------------------------------------------------------------

		EError					xfs_common::getLastError		( )
		{
			u32 lastError;
			if (m_pLastErrorStack->pop(lastError))
				return (EError)lastError;
			return FILE_ERROR_OK;
		}

		//------------------------------------------------------------------------------------------

		const char*				xfs_common::getLastErrorStr		( )
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
		///< Synchronous file operations
		s32					xfs_common::syncCreate			( const char* szFilename, xbool boRead, xbool boWrite )
		{
			s32 uHandle = asyncPreOpen(szFilename, boRead, boWrite);
			xfiledata* pxFileInfo = getFileInfo(uHandle);
			u32 nFileHandle;

			char szSystemFilenameBuffer[FS_MAX_PATH + 2];
			xcstring szSystemFilename(szSystemFilenameBuffer, sizeof(szSystemFilenameBuffer));
			xfiledevice* device = createSystemPath(szFilename, szSystemFilename);

			if (!pxFileInfo->m_pFileDevice->createFile(szSystemFilename.c_str(), boRead, boWrite, nFileHandle))
			{
				x_printf ("xfilesystem:"TARGET_PLATFORM_STR" ERROR device->createFile failed on file %s\n", x_va_list(pxFileInfo->m_szFilename));
				pxFileInfo->clear();
				uHandle = INVALID_FILE_HANDLE;
			}
			else
			{
				pxFileInfo->m_nFileHandle = nFileHandle;
			}
			return uHandle;
		}

		s32					xfs_common::syncOpen			( const char* szFilename, xbool boRead, xbool boWrite )
		{
			s32 uHandle = asyncPreOpen(szFilename, boRead, boWrite);
			xfiledata* pxFileInfo = getFileInfo(uHandle);
			u32 nFileHandle;

			char szSystemFilenameBuffer[FS_MAX_PATH + 2];
			xcstring szSystemFilename(szSystemFilenameBuffer, sizeof(szSystemFilenameBuffer));
			xfiledevice* device = createSystemPath(szFilename, szSystemFilename);

			if (!pxFileInfo->m_pFileDevice->openFile(szSystemFilename.c_str(), boRead, boWrite, nFileHandle))
			{
				x_printf ("xfilesystem:"TARGET_PLATFORM_STR" ERROR device->openFile failed on file %s\n", x_va_list(pxFileInfo->m_szFilename));
				pxFileInfo->clear();
				uHandle = INVALID_FILE_HANDLE;
			}
			else
			{
				pxFileInfo->m_nFileHandle = nFileHandle;
			}
			return uHandle;
		}

		//------------------------------------------------------------------------------------------

		u64					xfs_common::syncGetSize			( s32 uHandle )
		{
			if (uHandle==INVALID_FILE_HANDLE)
				return 0;

			xfiledata* pxFileInfo = getFileInfo(uHandle);
			u64 fileSize = 0;
			if (!pxFileInfo->m_pFileDevice->getLengthOfFile(pxFileInfo->m_nFileHandle, fileSize))
			{
				x_printf ("xfilesystem:"TARGET_PLATFORM_STR" ERROR device->getLengthOfFile failed on file %s\n", x_va_list(pxFileInfo->m_szFilename));
			}
			return fileSize;
		}

		//------------------------------------------------------------------------------------------

		void				xfs_common::syncSetSize			( s32 uHandle, u64 fileSize )
		{
			if (uHandle==INVALID_FILE_HANDLE)
				return;

			xfiledata* pxFileInfo = getFileInfo(uHandle);
			if (!pxFileInfo->m_pFileDevice->setLengthOfFile(pxFileInfo->m_nFileHandle, fileSize))
			{
				x_printf ("xfilesystem:"TARGET_PLATFORM_STR" ERROR device->setLengthOfFile failed on file %s\n", x_va_list(pxFileInfo->m_szFilename));
			}
		}

		//------------------------------------------------------------------------------------------

		u64					xfs_common::syncGetPos			( s32 uHandle )
		{
			if (uHandle==INVALID_FILE_HANDLE)
				return 0;

			xfiledata* pxFileInfo = getFileInfo(uHandle);
			u64 filePos = 0;
			if (!pxFileInfo->m_pFileDevice->getFilePos(pxFileInfo->m_nFileHandle, filePos))
			{
				x_printf ("xfilesystem:"TARGET_PLATFORM_STR" ERROR device->getFilePos failed on file %s\n", x_va_list(pxFileInfo->m_szFilename));
			}
			return filePos;
		}

		//------------------------------------------------------------------------------------------

		u64					xfs_common::syncSetPos			( s32 uHandle, u64 filePos )
		{
			if (uHandle==INVALID_FILE_HANDLE)
				return 0;

			xfiledata* pxFileInfo = getFileInfo(uHandle);
			if (!pxFileInfo->m_pFileDevice->setFilePos(pxFileInfo->m_nFileHandle, filePos))
			{
				x_printf ("xfilesystem:"TARGET_PLATFORM_STR" ERROR device->setFilePos failed on file %s\n", x_va_list(pxFileInfo->m_szFilename));
			}
			return filePos;
		}

		//------------------------------------------------------------------------------------------

		u64					xfs_common::syncRead			( s32 uHandle, u64 uOffset, u64 uSize, void* pBuffer )
		{
			if (uHandle==INVALID_FILE_HANDLE)
				return 0;

			xfiledata* pxFileInfo = getFileInfo(uHandle);
			u64 numBytesRead;
			if (!pxFileInfo->m_pFileDevice->readFile(pxFileInfo->m_nFileHandle, uOffset, pBuffer, uSize, numBytesRead))
			{
				x_printf ("xfilesystem:"TARGET_PLATFORM_STR" ERROR device->readFile failed on file %s\n", x_va_list(pxFileInfo->m_szFilename));
			}
			return numBytesRead;
		}

		//------------------------------------------------------------------------------------------

		u64					xfs_common::syncWrite			( s32 uHandle, u64 uOffset, u64 uSize, const void* pBuffer )
		{
			if (uHandle==INVALID_FILE_HANDLE)
				return 0;

			xfiledata* pxFileInfo = getFileInfo(uHandle);
			u64 numBytesWritten;
			if (!pxFileInfo->m_pFileDevice->writeFile(pxFileInfo->m_nFileHandle, uOffset, pBuffer, uSize, numBytesWritten))
			{
				x_printf ("xfilesystem:"TARGET_PLATFORM_STR" ERROR device->writeFile failed on file %s\n", x_va_list(pxFileInfo->m_szFilename));
			}
			return numBytesWritten;
		}


		//------------------------------------------------------------------------------------------

		void 				xfs_common::syncClose			( s32& uHandle )
		{
			if (uHandle==INVALID_FILE_HANDLE)
				return;

			xfiledata* pxFileInfo = getFileInfo(uHandle);
			if (!pxFileInfo->m_pFileDevice->closeFile(pxFileInfo->m_nFileHandle))
			{
				x_printf ("xfilesystem:"TARGET_PLATFORM_STR" ERROR device->closeFile failed on file %s\n", x_va_list(pxFileInfo->m_szFilename));
			}
			pxFileInfo->clear();
			pushFreeFileSlot(uHandle);
			uHandle = INVALID_FILE_HANDLE;
		}

		//------------------------------------------------------------------------------------------

		void				xfs_common::syncDelete			( s32& uHandle )
		{
			if (uHandle==INVALID_FILE_HANDLE)
				return;

			xfiledata* pxFileInfo = getFileInfo(uHandle);
			if (!pxFileInfo->m_pFileDevice->closeFile(pxFileInfo->m_nFileHandle))
			{
				x_printf ("xfilesystem:"TARGET_PLATFORM_STR" ERROR device->closeFile failed on file %s\n", x_va_list(pxFileInfo->m_szFilename));
			}
			if (pxFileInfo->m_pFileDevice->deleteFile(pxFileInfo->m_szFilename))
			{
				x_printf ("xfilesystem:"TARGET_PLATFORM_STR" ERROR device->deleteFile failed on file %s\n", x_va_list(pxFileInfo->m_szFilename));
			}
			pxFileInfo->clear();
			pushFreeFileSlot(uHandle);
			uHandle = INVALID_FILE_HANDLE;
		}



	//==============================================================================
	// END xcore namespace
	//==============================================================================
	};
};