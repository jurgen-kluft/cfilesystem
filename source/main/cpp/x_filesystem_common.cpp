//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase\x_target.h"
#include "xbase\x_debug.h"
#include "xbase\x_allocator.h"
#include "xbase\x_string_std.h"

#include "xfilesystem\private\x_filesystem_common.h"
#include "xfilesystem\private\x_filesystem_cqueue.h"

#include "xfilesystem\x_iasync_result.h"
#include "xfilesystem\x_devicealias.h"
#include "xfilesystem\x_filedevice.h"
#include "xfilesystem\x_filepath.h"
#include "xfilesystem\x_filesystem.h"
#include "xfilesystem\x_threading.h"
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

		class xiasync_result_imp : public xiasync_result
		{
		public:
							xiasync_result_imp();
			virtual			~xiasync_result_imp()					{ }

			void			init(xasync_id nAsyncId, u32 nFileHandle, xevent* pEvent);

			virtual bool	checkForCompletion();
			virtual void	waitForCompletion();

			virtual u64		getResult() const;

			virtual void	clear();
			virtual s32		hold();
			virtual s32		release();
			virtual void	destroy();

			XFILESYSTEM_OBJECT_NEW_DELETE()
		private:
			s32				mRefCount;		/// This needs to be an atomic integer
			xasync_id		mAsyncId;
			u32				mFileHandle;
			u64				mResult;
			xevent*			mEvent;
		};

		//------------------------------------------------------------------------------------------
		// Init/Exit touches these
		static u32							m_uMaxOpenedFiles = 32;
		static u32							m_uMaxAsyncOperations = 32;
		static u32							m_uMaxErrorItems = 32;
		static char*						*m_Filenames = NULL;
		static xfiledata					*m_OpenAsyncFileArray = NULL;
		static xfileasync					*m_AsyncIOData = NULL;
		static xiasync_result_imp			*m_AsyncResultData = NULL;

		///< Concurrent access
		static cqueue<xfiledata*>			*m_FreeAsyncFile = NULL;
		static cqueue<xfileasync*>			*m_pFreeAsyncIOList = NULL;
		static cqueue<xfileasync*>			*m_pAsyncIOList = NULL;
		static cqueue<xiasync_result*>		*m_pAsyncResultList = NULL;
		static cqueue<u32>					*m_pLastErrorStack = NULL;

		///< Synchronous file operations
		extern u32			syncCreate			( const char* szFilename, xbool boRead = true, xbool boWrite = false );
		extern u32			syncOpen			( const char* szFilename, xbool boRead = true, xbool boWrite = false );
		extern u64			syncSetPos			( u32 uHandle, u64 filePos );
		extern u64			syncGetPos			( u32 uHandle );
		extern void			syncSetSize			( u32 uHandle, u64 fileSize );
		extern u64			syncGetSize			( u32 uHandle );
		extern u64			syncRead			( u32 uHandle, u64 uOffset, u64 uSize, void* pBuffer );	
		extern u64			syncWrite			( u32 uHandle, u64 uOffset, u64 uSize, const void* pBuffer );
		extern void 		syncClose			( u32& uHandle );
		extern void			syncDelete			( u32& uHandle );

		///< Asynchronous file operations
		static u32			asyncPreOpen		( const char* szFilename, xbool boRead = true, xbool boWrite = false );
		static void			asyncOpen			( const u32 uHandle, xiasync_result** pAsyncId );
		static u32			asyncOpen			( const char* szFilename, xbool boRead = true, xbool boWrite = false, xiasync_result** pAsyncId = NULL );
		static void			asyncRead			( const u32 uHandle, u64 uOffset, u64 uSize, void* pBuffer, xiasync_result** pAsyncId );
		static void			asyncWrite			( const u32 uHandle, u64 uOffset, u64 uSize, const void* pBuffer, xiasync_result** pAsyncId );
		static void			asyncClose			( const u32 uHandle, xiasync_result** pAsyncId );
		static void			asyncCloseAndDelete	( const u32 uHandle, xiasync_result** pAsyncId );

		//------------------------------------------------------------------------------------------

		xbool				exists ( const char* szName )
		{
			u32 uHandle = syncOpen(szName, false);
			if (uHandle == (u32)INVALID_FILE_HANDLE)
				return false;
			syncClose(uHandle);
			return true;
		}

		//------------------------------------------------------------------------------------------

		u32					open ( const char* szFilename, xbool boRead, xbool boWrite, xiasync_result** pAsyncId)
		{
			if (pAsyncId)
				return asyncOpen(szFilename, boRead, boWrite, pAsyncId);
			else
				return syncOpen(szFilename, boRead, boWrite);
		}

		//------------------------------------------------------------------------------------------

		xbool				caps ( const xfilepath& szFilename, bool& can_read, bool& can_write, bool& can_seek, bool& can_async )
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

		void				setLength ( u32 uHandle, u64 uFileSize )
		{
			syncSetSize(uHandle, uFileSize);
		}

		//------------------------------------------------------------------------------------------

		u64					getLength ( u32 uHandle )
		{
			return syncGetSize(uHandle);
		}

		//------------------------------------------------------------------------------------------

		u64					read (u32 uHandle, u64 uOffset, u64 uSize, void* pBuffer, xiasync_result** pAsyncId )
		{
			if (pAsyncId != NULL)
			{
				asyncRead(uHandle, uOffset, uSize, pBuffer, pAsyncId);
				return 0;
			}
			else
			{
				return syncRead(uHandle, uOffset, uSize, pBuffer);
			}
		}

		//------------------------------------------------------------------------------------------

		u64					write ( u32 uHandle, u64 uOffset, u64 uSize, const void* pBuffer, xiasync_result** pAsyncId )
		{
			if (pAsyncId != NULL)
			{
				asyncWrite(uHandle, uOffset, uSize, pBuffer, pAsyncId);
				return 0;
			}
			else
			{
				return syncWrite(uHandle, uOffset, uSize, pBuffer);
			}
		}

		//------------------------------------------------------------------------------------------

		u64					getpos ( u32 uHandle )
		{
			return syncGetPos(uHandle);
		}

		//------------------------------------------------------------------------------------------

		u64					setpos ( u32 uHandle, u64 uFilePos )
		{
			return syncSetPos(uHandle, uFilePos);
		}

		//------------------------------------------------------------------------------------------

		void				closeAndDelete ( u32& uHandle, xiasync_result** pAsyncId )
		{
			if (pAsyncId != NULL)
				asyncCloseAndDelete(uHandle, pAsyncId);
			else
				syncDelete(uHandle);
		}

		//------------------------------------------------------------------------------------------

		void				close ( u32 &uHandle, xiasync_result** pAsyncId )
		{
			if (pAsyncId != NULL)
				asyncClose(uHandle, pAsyncId);
			else
				syncClose(uHandle);
		}

		//------------------------------------------------------------------------------------------

		void				save (const char* szFilename, const void* pData, u64 uSize)
		{
			ASSERTS (szFilename, "Save() : Pointer to name is NULL!");
			ASSERTS (pData,	"Save() : Pointer to data is NULL!");

			u32 uHandle = open(szFilename, xTRUE);
			if (uHandle != (u32)INVALID_FILE_HANDLE)
			{
				write(uHandle, 0, uSize, pData, NULL);
				close(uHandle, NULL);
			}
		}

		//------------------------------------------------------------------------------------------

		bool				asyncCompleted ( u32 uHandle, xasync_id const& uAsync )
		{
			if (uHandle==(u32)INVALID_FILE_HANDLE)
			{
				setLastError(FILE_ERROR_BADF);
				return true;
			}

			bool _out = m_pAsyncIOList->outside(uAsync);
			return _out;
		}

		//------------------------------------------------------------------------------------------

		u32					asyncOpen ( const char* szName, xbool boRead, xbool boWrite, xiasync_result** pAsyncId )
		{
			u32 uHandle = asyncPreOpen( szName, boRead, boWrite );
			asyncOpen(uHandle, pAsyncId);
			return uHandle;
		}

		//------------------------------------------------------------------------------------------

		void				asyncOpen (const u32 uHandle, xiasync_result** pAsyncId)
		{
			if (uHandle==(u32)INVALID_FILE_HANDLE)
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

			xasync_id id = pushAsyncIO(pOpen);
			if (pAsyncId!=NULL) 
			{
				xevent* async_event = getEventFactory()->construct();
				pOpen->setEvent(async_event);

				xiasync_result_imp* async_result = reinterpret_cast<xiasync_result_imp*>(popAsyncResult());
				async_result->init(id, uHandle, async_event);
				*pAsyncId = async_result;
			}
			getIoThreadInterface()->signal();
		}

		//------------------------------------------------------------------------------------------

		void				asyncRead			( const u32 uHandle, u64 uOffset, u64 uSize, void* pBuffer, xiasync_result** pAsyncId )
		{
			if (uHandle==(u32)INVALID_FILE_HANDLE)
			{
				setLastError(FILE_ERROR_BADF);
				return;
			}

			xfileasync* pRead = popFreeAsyncIO();
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

			xasync_id id = pushAsyncIO(pRead);
			if (pAsyncId!=NULL) 
			{
				xevent* async_event = getEventFactory()->construct();
				async_event->set();
				pRead->setEvent(async_event);

				xiasync_result_imp* async_result = reinterpret_cast<xiasync_result_imp*>(popAsyncResult());
				async_result->init(id, uHandle, async_event);
				*pAsyncId = async_result;
			}

			getIoThreadInterface()->signal();
		}

		//------------------------------------------------------------------------------------------

		void				asyncWrite			( const u32 uHandle, u64 uOffset, u64 uSize, const void* pBuffer, xiasync_result** pAsyncId )
		{
			if (uHandle==(u32)INVALID_FILE_HANDLE)
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

			xasync_id id = pushAsyncIO(pWrite);
			if (pAsyncId!=NULL) 
			{
				xevent* async_event = getEventFactory()->construct();
				async_event->set();
				pWrite->setEvent(async_event);

				xiasync_result_imp* async_result = reinterpret_cast<xiasync_result_imp*>(popAsyncResult());
				async_result->init(id, uHandle, async_event);
				*pAsyncId = async_result;
			}

			getIoThreadInterface()->signal();
		}

		//------------------------------------------------------------------------------------------

		void				asyncCloseAndDelete ( const u32 uHandle, xiasync_result** pAsyncId )
		{
			if (uHandle==(u32)INVALID_FILE_HANDLE)
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

			xasync_id id = pushAsyncIO(pDelete);
			if (pAsyncId!=NULL) 
			{
				xevent* async_event = getEventFactory()->construct();
				async_event->set();
				pDelete->setEvent(async_event);

				xiasync_result_imp* async_result = reinterpret_cast<xiasync_result_imp*>(popAsyncResult());
				async_result->init(id, uHandle, async_event);
				*pAsyncId = async_result;
			}

			getIoThreadInterface()->signal();
		}

		//------------------------------------------------------------------------------------------

		void				asyncClose ( const u32 uHandle, xiasync_result** pAsyncId )
		{
			if (uHandle==(u32)INVALID_FILE_HANDLE)
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

			xasync_id id = pushAsyncIO(pClose);
			if (pAsyncId!=NULL) 
			{
				xevent* async_event = getEventFactory()->construct();
				async_event->set();
				pClose->setEvent(async_event);

				xiasync_result_imp* async_result = reinterpret_cast<xiasync_result_imp*>(popAsyncResult());
				async_result->init(id, uHandle, async_event);
				*pAsyncId = async_result;
			}

			getIoThreadInterface()->signal();
		}


		//------------------------------------------------------------------------------------------

		u32				asyncPreOpen( const char* szFilename, xbool boRead, xbool boWrite )
		{
			//-----------------------------
			// File find a free file slot.
			//-----------------------------
			u32 uHandle = popFreeFileSlot ();
			if(uHandle == (u32)INVALID_FILE_HANDLE)
			{
				ASSERTS(0, "xfilesystem:" TARGET_PLATFORM_STR " ERROR Too many files opened!");

				setLastError(FILE_ERROR_MAX_FILES);
				return (u32)INVALID_FILE_HANDLE;
			}

			char szSystemFilenameBuffer[FS_MAX_PATH + 2];
			xcstring szSystemFilename(szSystemFilenameBuffer, sizeof(szSystemFilenameBuffer));
			xfiledevice* device = createSystemPath(szFilename, szSystemFilename);

			if (device==NULL)
			{
				setLastError(FILE_ERROR_DEVICE);
				return (u32)INVALID_FILE_HANDLE;
			}

			if (boWrite && !device->canWrite())
			{
				ASSERTS(0, "xfilesystem:" TARGET_PLATFORM_STR " ERROR Device is read-only!");

				setLastError(FILE_ERROR_DEVICE_READONLY);
				return (u32)INVALID_FILE_HANDLE;
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

		void				initialiseCommon ( u32 uMaxOpenStreams )
		{
			x_printf ("xfilesystem:"TARGET_PLATFORM_STR" INFO initialise()\n");

			m_uMaxOpenedFiles = uMaxOpenStreams;
			m_uMaxAsyncOperations = uMaxOpenStreams;

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
				void* mem1 = heapAlloc(sizeof(cqueue<xfiledata*>), CACHE_LINE_SIZE);
				void* mem2 = heapAlloc(m_uMaxOpenedFiles * sizeof(xfiledata*), CACHE_LINE_SIZE);
				cqueue<xfiledata*>* queue = new (mem1) cqueue<xfiledata*>(m_uMaxOpenedFiles, (xfiledata* volatile*)mem2);
				m_FreeAsyncFile = queue;
				// Populate the unused xfiledata queue
				for (u32 uFile = 0; uFile < m_uMaxOpenedFiles; uFile++)
				{
					u32 _idx;
					m_FreeAsyncFile->push(&m_OpenAsyncFileArray[uFile], _idx);
				}
			}
			
			if (m_AsyncIOData == NULL)
			{
				m_AsyncIOData = (xfileasync*)heapAlloc(m_uMaxOpenedFiles * sizeof(xfileasync), X_ALIGNMENT_DEFAULT);
				for (u32 uFile = 0; uFile < m_uMaxOpenedFiles; uFile++)
					new (&m_AsyncIOData[uFile]) xfileasync();
			}

			if (m_AsyncResultData == NULL)
			{
				m_AsyncResultData = (xiasync_result_imp*)heapAlloc(m_uMaxAsyncOperations * sizeof(xiasync_result_imp), X_ALIGNMENT_DEFAULT);
				for (u32 uOp = 0; uOp < m_uMaxAsyncOperations; uOp++)
					new (&m_AsyncResultData[uOp]) xiasync_result_imp();
			}


			//-------------------------------------------------------
			// Allocate all the concurrent queues aligned on a cache line
			//-------------------------------------------------------
			if (m_pFreeAsyncIOList == NULL)
			{
				void* mem1 = heapAlloc(sizeof(cqueue<xfileasync*>), CACHE_LINE_SIZE);
				void* mem2 = heapAlloc(m_uMaxAsyncOperations * sizeof(xfileasync*), CACHE_LINE_SIZE);
				cqueue<xfileasync*>* queue = new (mem1) cqueue<xfileasync*>(m_uMaxAsyncOperations, (xfileasync* volatile*)mem2);
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
				void* mem1 = heapAlloc(sizeof(cqueue<xfileasync*>), CACHE_LINE_SIZE);
				void* mem2 = heapAlloc(m_uMaxAsyncOperations * sizeof(xfileasync*), CACHE_LINE_SIZE);
				cqueue<xfileasync*>* queue = new (mem1) cqueue<xfileasync*>(m_uMaxAsyncOperations, (xfileasync* volatile*)mem2);
				m_pAsyncIOList = queue;
			}
			if (m_pAsyncResultList == NULL)
			{
				void* mem1 = heapAlloc(sizeof(cqueue<xiasync_result*>), CACHE_LINE_SIZE);
				void* mem2 = heapAlloc(m_uMaxAsyncOperations * sizeof(xiasync_result*), CACHE_LINE_SIZE);
				cqueue<xiasync_result*>* queue = new (mem1) cqueue<xiasync_result*>(m_uMaxAsyncOperations, (xiasync_result* volatile*)mem2);
				m_pAsyncResultList = queue;
				// Populate the queue
				for (u32 uOp = 0; uOp < m_uMaxAsyncOperations; uOp++)
				{
					u32 _idx;
					m_pAsyncResultList->push(&m_AsyncResultData[uOp], _idx);
				}
			}			
			if (m_pLastErrorStack == NULL)
			{
				void* mem1 = heapAlloc(sizeof(cqueue<xfileasync*>), CACHE_LINE_SIZE);
				void* mem2 = heapAlloc(m_uMaxErrorItems * sizeof(u32), CACHE_LINE_SIZE);
				cqueue<u32>* queue = new (mem1) cqueue<u32>(m_uMaxErrorItems, (u32 volatile*)mem2);
				m_pLastErrorStack = queue;

				//--------------------------------
				// Clear the error stack
				//--------------------------------
				u32* lastErrorStack = (u32*)mem2;
				for (u32 i=0; i<m_uMaxErrorItems; ++i)
					lastErrorStack[i] = FILE_ERROR_OK;
			}

		}	

		//------------------------------------------------------------------------------------------

		void				shutdownCommon		( void )
		{
			x_printf ("xfilesystem:"TARGET_PLATFORM_STR" INFO shutdown()\n");

			//-------------------------------------------------------
			// Free all the concurrent queues
			//-------------------------------------------------------
			if (m_pFreeAsyncIOList != NULL)
			{
				heapFree((void*)m_pFreeAsyncIOList->getArray());
				m_pFreeAsyncIOList->~cqueue<xfileasync*>();
				heapFree(m_pFreeAsyncIOList);
				m_pFreeAsyncIOList = NULL;
			}

			if (m_pAsyncIOList != NULL)
			{
				heapFree((void*)m_pAsyncIOList->getArray());
				m_pAsyncIOList->~cqueue<xfileasync*>();
				heapFree(m_pAsyncIOList);
				m_pAsyncIOList = NULL;
				heapFree(m_AsyncIOData);
				m_AsyncIOData = NULL;
			}

			if (m_pAsyncResultList != NULL)
			{
				heapFree((void*)m_pAsyncResultList->getArray());
				m_pAsyncResultList->~cqueue<xiasync_result*>();
				heapFree(m_pAsyncResultList);
				m_pAsyncResultList = NULL;
				heapFree(m_AsyncResultData);
				m_AsyncResultData = NULL;
			}

			if (m_FreeAsyncFile != NULL)
			{
				heapFree((void*)m_FreeAsyncFile->getArray());
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
				heapFree((void*)m_pLastErrorStack->getArray());
				m_pLastErrorStack->~cqueue<u32>();
				heapFree(m_pLastErrorStack);
				m_pLastErrorStack = NULL;
			}
		}

		//------------------------------------------------------------------------------------------
		class xio_thread_st : public xio_thread
		{
		public:
			virtual void		sleep(u32 ms)						{ }
			virtual bool		loop() const						{ return false; }
			virtual void		wait()								{ }
			virtual void		signal()							{ }
		};

		static xio_thread_st	sIoThreadSt;
		static xio_thread*		sIoThreadCt = &sIoThreadSt;

		void					setIoThreadInterface( xio_thread* io_thread )
		{
			sIoThreadCt = io_thread;
			if (sIoThreadCt == NULL)
				sIoThreadCt = &sIoThreadSt;
		}

		xio_thread*				getIoThreadInterface()
		{
			return sIoThreadCt;
		}

		static xevent_factory*	sEventFactory = NULL;
		void					setEventFactory		( xevent_factory* event_factory )
		{
			sEventFactory = event_factory;
		}
		
		xevent_factory*			getEventFactory		( void )
		{
			return sEventFactory;
		}


		//------------------------------------------------------------------------------------------

		xfiledevice*			createSystemPath( const char* inFilename, xcstring& outFilename )
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
			outFilename = szFilename.c_str();

			return device;
		}

		//------------------------------------------------------------------------------------------

		xfiledata*				getFileInfo			( u32 uHandle )
		{
			ASSERT(uHandle<m_uMaxOpenedFiles);
			return &m_OpenAsyncFileArray[uHandle];
		}

		//------------------------------------------------------------------------------------------

		u32						popFreeFileSlot			(void)
		{
			xfiledata* f;
			if (m_FreeAsyncFile->pop(f))
				return f->m_nFileIndex;
			return -1;
		}

		//------------------------------------------------------------------------------------------

		bool					pushFreeFileSlot		(u32 nFileIndex)
		{
			xfiledata* f = &m_OpenAsyncFileArray[nFileIndex];
			xasync_id _id;
			if (m_FreeAsyncFile->push(f, _id))
				return true;
			return false;
		}

		//------------------------------------------------------------------------------------------

		xfileasync*				getAsyncIOData			( u32 nSlot )
		{
			xfileasync* asyncIOInfo = &m_AsyncIOData[nSlot];
			return asyncIOInfo;
		}

		//------------------------------------------------------------------------------------------

		xfileasync*				popFreeAsyncIO			( bool wait )
		{
			do
			{
				xfileasync* asyncIOInfo;
				if (m_pFreeAsyncIOList->pop(asyncIOInfo))
				{
					asyncIOInfo->clear();
					return asyncIOInfo;
				}
				
			} while (wait);
			return NULL;
		}

		//------------------------------------------------------------------------------------------

		void					pushFreeAsyncIO			( xfileasync* asyncIOInfo )
		{
			asyncIOInfo->setFileIndex(INVALID_FILE_HANDLE);
			u32 idx;
			m_pFreeAsyncIOList->push(asyncIOInfo, idx);
		}

		//------------------------------------------------------------------------------------------

		xfileasync*				popAsyncIO				( void )
		{
			xfileasync* item;
			if (m_pAsyncIOList->pop(item))
				return item;
			return NULL;
		}

		//------------------------------------------------------------------------------------------

		xasync_id				pushAsyncIO				( xfileasync* asyncIOInfo )
		{
			u32 _idx;
			m_pAsyncIOList->push(asyncIOInfo, _idx);
			return _idx;
		}

		//------------------------------------------------------------------------------------------

		u32						testAsyncId			( xasync_id id )
		{
			if (m_pAsyncIOList->inside(id))
				return 0;	// In the processing queue
			else if (m_pAsyncIOList->outside(id))
				return -1;	// Has been processed
			else
				return 1;	// Not processed yet
		}

		//------------------------------------------------------------------------------------------

		xiasync_result*			popAsyncResult		( void )
		{
			xiasync_result* item;
			if (m_pAsyncResultList->pop(item))
				return item;
			return NULL;
		}

		//------------------------------------------------------------------------------------------

		void					pushAsyncResult		( xiasync_result* asyncResult )
		{
			u32 _idx;
			m_pAsyncResultList->push(asyncResult, _idx);
		}

		//------------------------------------------------------------------------------------------

		void					setLastError		( EError error )
		{
			u32 old_error;
			u32 _idx;
			while (!m_pLastErrorStack->push((u32)error, _idx))
				m_pLastErrorStack->pop(old_error);
		}

		//------------------------------------------------------------------------------------------

		xbool					hasLastError		( void )
		{
			u32 lastError = FILE_ERROR_OK;
			m_pLastErrorStack->peek(lastError);
			return lastError != FILE_ERROR_OK;
		}

		//------------------------------------------------------------------------------------------

		void					clearLastError		( void )
		{
			setLastError(FILE_ERROR_OK);
		}

		//------------------------------------------------------------------------------------------

		EError					getLastError		( )
		{
			u32 lastError;
			if (m_pLastErrorStack->peek(lastError))
			{
				return (EError)lastError;
			}
			return FILE_ERROR_OK;
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
		///< Synchronous file operations
		u32					syncCreate			( const char* szFilename, xbool boRead, xbool boWrite )
		{
			u32 uHandle = asyncPreOpen(szFilename, boRead, boWrite);
			xfiledata* pxFileInfo = getFileInfo(uHandle);
			u32 nFileHandle;
			if (!pxFileInfo->m_pFileDevice->createFile(szFilename, boRead, boWrite, nFileHandle))
			{
				x_printf ("xfilesystem:"TARGET_PLATFORM_STR" ERROR device->createFile failed on file %s\n", x_va_list(pxFileInfo->m_szFilename));
				pxFileInfo->clear();
				uHandle = (u32)INVALID_FILE_HANDLE;
			}
			else
			{
				pxFileInfo->m_nFileHandle = nFileHandle;
			}
			return uHandle;
		}

		u32					syncOpen			( const char* szFilename, xbool boRead, xbool boWrite )
		{
			u32 uHandle = asyncPreOpen(szFilename, boRead, boWrite);
			xfiledata* pxFileInfo = getFileInfo(uHandle);
			u32 nFileHandle;
			if (!pxFileInfo->m_pFileDevice->openFile(szFilename, boRead, boWrite, nFileHandle))
			{
				x_printf ("xfilesystem:"TARGET_PLATFORM_STR" ERROR device->openFile failed on file %s\n", x_va_list(pxFileInfo->m_szFilename));
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

		u64					syncGetSize			( u32 uHandle )
		{
			if (uHandle==(u32)INVALID_FILE_HANDLE)
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

		void				syncSetSize			( u32 uHandle, u64 fileSize )
		{
			if (uHandle==(u32)INVALID_FILE_HANDLE)
				return;

			xfiledata* pxFileInfo = getFileInfo(uHandle);
			if (!pxFileInfo->m_pFileDevice->setLengthOfFile(pxFileInfo->m_nFileHandle, fileSize))
			{
				x_printf ("xfilesystem:"TARGET_PLATFORM_STR" ERROR device->setLengthOfFile failed on file %s\n", x_va_list(pxFileInfo->m_szFilename));
			}
		}

		//------------------------------------------------------------------------------------------

		u64					syncGetPos			( u32 uHandle )
		{
			if (uHandle==(u32)INVALID_FILE_HANDLE)
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

		u64					syncSetPos			( u32 uHandle, u64 filePos )
		{
			if (uHandle==(u32)INVALID_FILE_HANDLE)
				return 0;

			xfiledata* pxFileInfo = getFileInfo(uHandle);
			if (!pxFileInfo->m_pFileDevice->setFilePos(pxFileInfo->m_nFileHandle, filePos))
			{
				x_printf ("xfilesystem:"TARGET_PLATFORM_STR" ERROR device->setFilePos failed on file %s\n", x_va_list(pxFileInfo->m_szFilename));
			}
			return filePos;
		}

		//------------------------------------------------------------------------------------------

		u64					syncRead			( u32 uHandle, u64 uOffset, u64 uSize, void* pBuffer )
		{
			if (uHandle==(u32)INVALID_FILE_HANDLE)
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

		u64					syncWrite			( u32 uHandle, u64 uOffset, u64 uSize, const void* pBuffer )
		{
			if (uHandle==(u32)INVALID_FILE_HANDLE)
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

		void 				syncClose			( u32& uHandle )
		{
			if (uHandle==(u32)INVALID_FILE_HANDLE)
				return;

			xfiledata* pxFileInfo = getFileInfo(uHandle);
			if (!pxFileInfo->m_pFileDevice->closeFile(pxFileInfo->m_nFileHandle))
			{
				x_printf ("xfilesystem:"TARGET_PLATFORM_STR" ERROR device->closeFile failed on file %s\n", x_va_list(pxFileInfo->m_szFilename));
			}
			pxFileInfo->clear();
			pushFreeFileSlot(uHandle);
			uHandle = (u32)INVALID_FILE_HANDLE;
		}

		//------------------------------------------------------------------------------------------

		void				syncDelete			( u32& uHandle )
		{
			if (uHandle==(u32)INVALID_FILE_HANDLE)
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
			uHandle = (u32)INVALID_FILE_HANDLE;
		}

		//------------------------------------------------------------------------------------------

		xiasync_result_imp::xiasync_result_imp()
			: mRefCount(0)
			, mAsyncId(0)
			, mFileHandle(INVALID_FILE_HANDLE)
			, mResult(0)
			, mEvent(NULL)
		{
		}

		void			xiasync_result_imp::init(xasync_id nAsyncId, u32 nFileHandle, xevent* pEvent)
		{
			mRefCount = 0;
			mAsyncId = nAsyncId;
			mFileHandle = nFileHandle;
			mResult = 0;
			mEvent = pEvent;
		}

		bool			xiasync_result_imp::checkForCompletion()
		{
			return mFileHandle == INVALID_FILE_HANDLE || testAsyncId(mAsyncId) == -1;
		}

		void			xiasync_result_imp::waitForCompletion()
		{
			if (mFileHandle != INVALID_FILE_HANDLE)
			{
				xfiledata* fileInfo = getFileInfo(mFileHandle);
				if (checkForCompletion() == false)
				{
					mEvent->wait();
				}
			}
		}

		u64				xiasync_result_imp::getResult() const
		{
			return mResult;
		}

		void			xiasync_result_imp::clear()
		{
			mFileHandle = INVALID_FILE_HANDLE;
		}

		s32				xiasync_result_imp::hold()
		{
			return ++mRefCount;
		}

		s32				xiasync_result_imp::release()
		{
			return --mRefCount;
		}

		void			xiasync_result_imp::destroy()
		{
			// Push us back in the free list
			u32 _idx;
			m_pAsyncResultList->push(this, _idx);
		}

	};

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};
