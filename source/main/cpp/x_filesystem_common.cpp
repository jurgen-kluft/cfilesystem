//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase\x_target.h"
#include "xbase\x_debug.h"
#include "xbase\x_allocator.h"
#include "xbase\x_string_std.h"

#include "xfilesystem\private\x_filesystem_common.h"
#include "xfilesystem\private\x_filesystem_spsc_queue.h"

#include "xfilesystem\x_filesystem.h"
#include "xfilesystem\private\x_filedata.h"
#include "xfilesystem\private\x_fileasync.h"
#include "xfilesystem\x_iasync_result.h"
#include "xfilesystem\x_filedevice.h"
#include "xfilesystem\x_filepath.h"
#include "xfilesystem\x_devicealias.h"
#include "xfilesystem\x_threading.h"

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

		//------------------------------------------------------------------------------------------

		class xiasync_result_imp : public xiasync_result
		{
		public:
									xiasync_result_imp()
										: mFileHandle(INVALID_FILE_HANDLE)	{ }
			virtual					~xiasync_result_imp()					{ }

			XFILESYSTEM_OBJECT_NEW_DELETE()

			void					init(u32 nFileHandle)
			{
				mFileHandle = nFileHandle;
			}

			virtual bool			isCompleted()
			{
				return mFileHandle == INVALID_FILE_HANDLE || asyncDone(mFileHandle) == xTRUE;
			}

			virtual void			waitUntilCompleted()
			{
				if (mFileHandle != INVALID_FILE_HANDLE)
				{
					xfiledata* fileInfo = getFileInfo(mFileHandle);
					getThreading()->wait(fileInfo->m_nFileIndex);
				}
			}

			virtual void			clear()
			{
				mFileHandle = INVALID_FILE_HANDLE;
			}

			virtual void			hold()
			{
			}

			virtual s32				release()
			{
				return 1;
			}

			virtual void			destroy()
			{
				
			}

		private:
			u32				mFileHandle;
		};

		//------------------------------------------------------------------------------------------
		static u32							m_uMaxOpenedFiles = 32;
		static u32							m_uMaxAsyncOperations = 32;
		static u32							m_uMaxErrorItems = 32;

		static char*						*m_Filenames;

		static xiasync_result_imp			*m_AsyncResults;
		static xfiledata					*m_OpenAsyncFile;
		static xfileasync					*m_AsyncIOData;

		static EError						*m_eLastErrorStack;

		static spsc_cqueue<xfileasync*>*	m_pFreeAsyncIOList = NULL;
		static spsc_cqueue<xfileasync*>*	m_pAsyncIOList = NULL;

		//------------------------------------------------------------------------------------------

		void				update( void )
		{
			sync(FS_SYNC_NOWAIT);
		}

		//------------------------------------------------------------------------------------------

		xbool				exists( const char* szName )
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

		xbool				caps				( const xfilepath& szFilename, bool& can_read, bool& can_write, bool& can_seek, bool& can_async )
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

		u64					getLength			( u32 uHandle )
		{
			return syncSize(uHandle);
		}

		//------------------------------------------------------------------------------------------

		void				read (u32 uHandle, u64 uOffset, u64 uSize, void* pBuffer )
		{
			syncRead(uHandle, uOffset, uSize, pBuffer);
		}

		//------------------------------------------------------------------------------------------

		void				write( u32 uHandle, u64 uOffset, u64 uSize, const void* pBuffer )
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

		void				save (const char* szFilename, const void* pData, u64 uSize)
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

			xfiledata*  pInfo  = getFileInfo(uHandle);
			xfileasync* pAsync = getAsyncIOData(pInfo->m_nFileIndex);
			return pAsync->getStatus() == FILE_OP_STATUS_DONE;
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

			pOpen->setFileIndex			(uHandle);
			pOpen->setStatus			(FILE_OP_STATUS_OPEN_PENDING);
			pOpen->setReadAddress		(NULL);
			pOpen->setWriteAddress		(NULL);
			pOpen->setReadWriteOffset	(0);
			pOpen->setReadWriteSize		(0);

			asyncIOAddToTail(pOpen);
			getThreading()->signal();
		}

		//------------------------------------------------------------------------------------------

		void				asyncRead			( const u32 uHandle, u64 uOffset, u64 uSize, void* pBuffer, xiasync_result*& pAsyncResult )
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

			xfiledata* pInfo = getFileInfo(uHandle);

			xiasync_result_imp* pAsyncResultImp = &m_AsyncResults[pInfo->m_nFileIndex];
			pAsyncResultImp->init(pInfo->m_nFileHandle);
			pAsyncResult = pAsyncResultImp;

			pRead->setFileIndex			(pInfo->m_nFileIndex);
			pRead->setStatus 			(FILE_OP_STATUS_READ_PENDING);
			pRead->setReadAddress		(pBuffer);
			pRead->setWriteAddress		(NULL);
			pRead->setReadWriteOffset	(uOffset);
			pRead->setReadWriteSize		(uSize);

			asyncIOAddToTail(pRead);
			getThreading()->signal();
		}

		//------------------------------------------------------------------------------------------

		void				asyncWrite			( const u32 uHandle, u64 uOffset, u64 uSize, const void* pBuffer, xiasync_result*& pAsyncResult )
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

			xfiledata* pInfo = getFileInfo(uHandle);

			xiasync_result_imp* pAsyncResultImp = &m_AsyncResults[pInfo->m_nFileIndex];
			pAsyncResultImp->init(pInfo->m_nFileHandle);
			pAsyncResult = pAsyncResultImp;

			pWrite->setFileIndex		(uHandle);
			pWrite->setStatus 			(FILE_OP_STATUS_WRITE_PENDING);
			pWrite->setReadAddress		(NULL);
			pWrite->setWriteAddress		(pBuffer);
			pWrite->setReadWriteOffset	(uOffset);
			pWrite->setReadWriteSize	(uSize);

			asyncIOAddToTail(pWrite);
			getThreading()->signal();
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

			pDelete->setFileIndex		(uHandle);
			pDelete->setStatus 			(FILE_OP_STATUS_DELETE_PENDING);
			pDelete->setReadAddress		(NULL);
			pDelete->setWriteAddress	(NULL);
			pDelete->setReadWriteOffset	(0);
			pDelete->setReadWriteSize	(0);

			asyncIOAddToTail(pDelete);
			getThreading()->signal();
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

			pClose->setFileIndex		(uHandle);
			pClose->setStatus 			(FILE_OP_STATUS_CLOSE_PENDING);
			pClose->setReadAddress		(NULL);
			pClose->setWriteAddress		(NULL);
			pClose->setReadWriteOffset	(0);
			pClose->setReadWriteSize	(0);

			asyncIOAddToTail(pClose);
			getThreading()->signal();
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

			char szSystemFilenameBuffer[FS_MAX_PATH + 2];
			xcstring szSystemFilename(szSystemFilenameBuffer, sizeof(szSystemFilenameBuffer));
			xfiledevice* device = createSystemPath(szFilename, szSystemFilename);

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

			xfiledata* fileInfo = getFileInfo(uHandle);
			fileInfo->m_uByteSize		= 0;

			fileInfo->m_nFileHandle		= (u32)PENDING_FILE_HANDLE;
			fileInfo->m_nFileIndex		= uHandle;

			xcstring fileInfoFilename(fileInfo->m_szFilename, fileInfo->m_sFilenameMaxLen, szSystemFilename.c_str());

			fileInfo->m_boReading		= boRead;
			fileInfo->m_boWriting		= boWrite;
			fileInfo->m_pFileDevice		= device;

			return uHandle;
		}

		//------------------------------------------------------------------------------------------

		void			waitUntilIdle(void)
		{
			sync(FS_SYNC_WAIT);
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

			m_Filenames = (char**)heapAlloc(m_uMaxOpenedFiles * sizeof(char*), X_ALIGNMENT_DEFAULT);
			for (u32 uFile = 0; uFile < m_uMaxOpenedFiles; uFile++)
				m_Filenames[uFile] = (char*)heapAlloc(FS_MAX_PATH + 2, X_ALIGNMENT_DEFAULT);

			m_AsyncResults = (xiasync_result_imp*)heapAlloc(m_uMaxOpenedFiles * sizeof(xiasync_result_imp), X_ALIGNMENT_DEFAULT);
			for (u32 uFile = 0; uFile < m_uMaxOpenedFiles; uFile++)
				new (&m_AsyncResults[uFile]) xiasync_result_imp();
			m_OpenAsyncFile = (xfiledata*)heapAlloc(m_uMaxOpenedFiles * sizeof(xfiledata), X_ALIGNMENT_DEFAULT);
			for (u32 uFile = 0; uFile < m_uMaxOpenedFiles; uFile++)
				new (&m_OpenAsyncFile[uFile]) xfiledata();
			m_AsyncIOData = (xfileasync*)heapAlloc(m_uMaxOpenedFiles * sizeof(xfileasync), X_ALIGNMENT_DEFAULT);
			for (u32 uFile = 0; uFile < m_uMaxOpenedFiles; uFile++)
				new (&m_AsyncIOData[uFile]) xfileasync();
			m_eLastErrorStack = (EError*)heapAlloc(m_uMaxErrorItems * sizeof(EError), X_ALIGNMENT_DEFAULT);

			//--------------------------------
			// Clear the error stack
			//--------------------------------
			for (u32 i=0; i<m_uMaxErrorItems; ++i)
				m_eLastErrorStack[i] = FILE_ERROR_OK;

			//-------------------------------------------------------
			// Allocate all the spsc queues aligned on a cache line
			//-------------------------------------------------------
			if (m_pFreeAsyncIOList == NULL)
			{
				void* mem1 = heapAlloc(sizeof(spsc_cqueue<xfileasync*>), CACHE_LINE_SIZE);
				void* mem2 = heapAlloc(m_uMaxAsyncOperations * sizeof(xfileasync*), CACHE_LINE_SIZE);
				spsc_cqueue<xfileasync*>* queue = new (mem1) spsc_cqueue<xfileasync*>(m_uMaxAsyncOperations, (xfileasync* volatile*)mem2);
				m_pFreeAsyncIOList = queue;
			}
			if (m_pAsyncIOList == NULL)
			{
				void* mem1 = heapAlloc(sizeof(spsc_cqueue<xfileasync*>), CACHE_LINE_SIZE);
				void* mem2 = heapAlloc(m_uMaxAsyncOperations * sizeof(xfileasync*), CACHE_LINE_SIZE);
				spsc_cqueue<xfileasync*>* queue = new (mem1) spsc_cqueue<xfileasync*>(m_uMaxAsyncOperations, (xfileasync* volatile*)mem2);
				m_pAsyncIOList = queue;
			}

			//---------------------------------------
			// Current there is no Async file loaded.
			//---------------------------------------
			for (u32 uFile = 0; uFile < m_uMaxOpenedFiles; uFile++)
			{
				m_OpenAsyncFile[uFile].clear();
				m_OpenAsyncFile[uFile].m_szFilename = m_Filenames[uFile];
				m_OpenAsyncFile[uFile].m_sFilenameMaxLen = FS_MAX_PATH;

				x_memset(m_OpenAsyncFile[uFile].m_szFilename, 0, FS_MAX_PATH);
				m_OpenAsyncFile[uFile].m_szFilename[0] = '\0';
				m_OpenAsyncFile[uFile].m_szFilename[1] = '\0';
				m_OpenAsyncFile[uFile].m_szFilename[FS_MAX_PATH+0] = '\0';
				m_OpenAsyncFile[uFile].m_szFilename[FS_MAX_PATH+1] = '\0';
			}

			for(u32 uSlot = 0; uSlot < m_uMaxAsyncOperations; uSlot++)	
			{
				m_AsyncIOData[uSlot].clear();
				m_pFreeAsyncIOList->push(&m_AsyncIOData[uSlot]);
			}
			ASSERT(m_pFreeAsyncIOList->full());
		}	

		//------------------------------------------------------------------------------------------

		void				shutdownCommon		( void )
		{
			x_printf ("xfilesystem:"TARGET_PLATFORM_STR" INFO shutdown()\n");

			//-------------------------------------------------------
			// Free all the spsc queues
			//-------------------------------------------------------
			if (m_pFreeAsyncIOList != NULL)
			{
				heapFree((void*)m_pFreeAsyncIOList->getArray());
				m_pFreeAsyncIOList->~spsc_cqueue<xfileasync*>();
				heapFree(m_pFreeAsyncIOList);
				m_pFreeAsyncIOList = NULL;
			}
			if (m_pAsyncIOList != NULL)
			{
				heapFree((void*)m_pAsyncIOList->getArray());
				m_pAsyncIOList->~spsc_cqueue<xfileasync*>();
				heapFree(m_pAsyncIOList);
				m_pAsyncIOList = NULL;
			}

			for (u32 uFile = 0; uFile < m_uMaxOpenedFiles; uFile++)
				heapFree(m_Filenames[uFile]);
			heapFree(m_Filenames);

			heapFree(m_AsyncResults);
			heapFree(m_OpenAsyncFile);
			heapFree(m_AsyncIOData);
			heapFree(m_eLastErrorStack);

			m_Filenames = NULL;
			m_AsyncResults = NULL;
			m_OpenAsyncFile = NULL;
			m_AsyncIOData = NULL;
			m_eLastErrorStack = NULL;
		}

		//------------------------------------------------------------------------------------------
		class xsinglethreading : public xthreading
		{
		public:
			virtual void		sleep(u32 ms)						{ }
			virtual bool		loop() const						{ return false; }
			virtual void		wait()								{ }
			virtual void		signal()							{ }

			virtual void		lock(u32 streamIndex)				{ }
			virtual void		unlock(u32 streamIndex)				{ }
			virtual void		wait(u32 streamIndex)				{ }
			virtual void		signal(u32 streamIndex)				{ }
		};

		static xsinglethreading	sIoThreadingSt;
		static xthreading*		sIoThreading = &sIoThreadingSt;

		void				setThreading( xthreading* io_thread )
		{
			sIoThreading = io_thread;
			if (sIoThreading == NULL)
				sIoThreading = &sIoThreadingSt;
		}

		xthreading*			getThreading()
		{
			return sIoThreading;
		}

		//------------------------------------------------------------------------------------------

		xfiledevice*			createSystemPath( const char* inFilename, xcstring& outFilename )
		{
			xfilepath szFilename(inFilename);
			const xfilesystem::xdevicealias* alias = xdevicealias::sFind(szFilename);
			if (alias == NULL)
			{
				// Take the workdir
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

		xfiledata*			getFileInfo			( u32 uHandle )
		{
			ASSERT(uHandle<m_uMaxOpenedFiles);
			return &m_OpenAsyncFile[uHandle];
		}

		//------------------------------------------------------------------------------------------

		u32					findFreeFileSlot (void)
		{
			for (u32 nSlot = 0; nSlot < m_uMaxOpenedFiles; nSlot++)
			{
				if (m_OpenAsyncFile[nSlot].m_nFileHandle == (u32)INVALID_FILE_HANDLE)
					return (nSlot);
			}
			return (u32)-1;
		}

		//------------------------------------------------------------------------------------------

		s32					asyncIONumFreeSlots()
		{
			return m_pFreeAsyncIOList->size();
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

		xbool				sync (ESyncMode mode)
		{
			bool boDone = true;

			do
			{
				boDone = true;

				for(u32 nSlot = 0; nSlot < m_uMaxAsyncOperations; nSlot++)
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

						if (mode != FS_SYNC_WAIT)
						{
							return boDone;
						}
						else
						{
							getThreading()->sleep(1);
						}
					}
					else if( pOperation->getStatus() == FILE_OP_STATUS_DONE )
					{
						pOperation->setStatus(FILE_OP_STATUS_FREE);
						pOperation->setFileIndex(-1);

						freeAsyncIOAddToTail(pOperation);
					}
				}

			} while (!boDone && mode == FS_SYNC_WAIT);

			return boDone;
		}


		//------------------------------------------------------------------------------------------

		void				setLastError		( EError error )
		{
			// Push everything up
			for (u32 i=1; i<m_uMaxErrorItems; ++i)
			{
				m_eLastErrorStack[i-1] = m_eLastErrorStack[i];
			}
			m_eLastErrorStack[m_uMaxErrorItems-1] = error;
		}


		//------------------------------------------------------------------------------------------

		xbool					hasLastError		( void )
		{
			return m_eLastErrorStack[m_uMaxErrorItems-1] != FILE_ERROR_OK;
		}

		//------------------------------------------------------------------------------------------

		void					clearLastError		( void )
		{
			setLastError(FILE_ERROR_OK);
		}

		//------------------------------------------------------------------------------------------

		EError					getLastError		( )
		{
			return m_eLastErrorStack[m_uMaxErrorItems-1];
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
		u32					syncCreate			( const char* szFilename, xbool boRead, xbool boWrite)
		{
			u32 uHandle = asyncPreOpen(szFilename, boRead, boWrite);
			xfiledata* pxFileInfo = getFileInfo(uHandle);
			u32 nFileHandle;
			if (!pxFileInfo->m_pFileDevice->createFile(szFilename, boRead, boWrite, nFileHandle))
			{
				x_printf ("device->createFile failed on file %s\n", x_va_list(pxFileInfo->m_szFilename));
				pxFileInfo->clear();
				uHandle = (u32)INVALID_FILE_HANDLE;
			}
			else
			{
				pxFileInfo->m_nFileHandle = nFileHandle;
			}
			return uHandle;
		}

		u32					syncOpen			( const char* szFilename, xbool boRead, xbool boWrite)
		{
			u32 uHandle = asyncPreOpen(szFilename, boRead, boWrite);
			xfiledata* pxFileInfo = getFileInfo(uHandle);
			u32 nFileHandle;
			if (!pxFileInfo->m_pFileDevice->openFile(szFilename, boRead, boWrite, nFileHandle))
			{
				x_printf ("device->openFile failed on file %s\n", x_va_list(pxFileInfo->m_szFilename));
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

		u64					syncSize			( u32 uHandle )
		{
			if (uHandle==(u32)INVALID_FILE_HANDLE)
				return 0;

			xfiledata* pxFileInfo = getFileInfo(uHandle);
			u64 length;
			if (!pxFileInfo->m_pFileDevice->getLengthOfFile(pxFileInfo->m_nFileHandle, length))
			{
				x_printf ("device->lengthOfFile failed on file %s\n", x_va_list(pxFileInfo->m_szFilename));
			}
			return length;
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
				x_printf ("device->ReadFile failed on file %s\n", x_va_list(pxFileInfo->m_szFilename));
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
				x_printf ("device->WriteFile failed on file %s\n", x_va_list(pxFileInfo->m_szFilename));
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

			xfiledata* pxFileInfo = getFileInfo(uHandle);
			if (!pxFileInfo->m_pFileDevice->closeFile(pxFileInfo->m_nFileHandle))
			{
				x_printf ("device->CloseFile failed on file %s\n", x_va_list(pxFileInfo->m_szFilename));
			}
			pxFileInfo->m_pFileDevice->deleteFile(pxFileInfo->m_szFilename);
			pxFileInfo->clear();
			uHandle = (u32)INVALID_FILE_HANDLE;
		}
	};


	//==============================================================================
	// END xcore namespace
	//==============================================================================
};
