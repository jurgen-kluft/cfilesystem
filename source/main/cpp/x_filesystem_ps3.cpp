#include "xbase\x_target.h"
#ifdef TARGET_PS3

//==============================================================================
// INCLUDES
//==============================================================================

#include <stdio.h>
#include <sys/paths.h>
#include <sys/process.h>
#include <sys/timer.h>
#include <cell/cell_fs.h>
#include <cell/fs/cell_fs_file_api.h>
#include <cell/sysmodule.h>
#include <sys/event.h>
#include <sys/ppu_thread.h>
#include <sys/synchronization.h>

#include "xbase\x_debug.h"
#include "xbase\x_string_std.h"
#include "xbase\x_va_list.h"

#include "xfilesystem\private\x_filesystem_common.h"
#include "xfilesystem\private\x_filesystem_ps3.h"

namespace xcore
{
	//------------------------------------------------------------------------------------------
	//---------------------------------- PS3 IO Functions ------------------------------------
	//------------------------------------------------------------------------------------------
	namespace xfilesystem
	{
		class xfiledevice_ps3system : public xifiledevice
		{
		public:
			virtual bool	OpenOrCreateFile(FileInfo* pInfo);
			virtual bool	LengthOfFile(FileInfo* pInfo, u64& outLength);
			virtual bool	CloseFile(FileInfo* pInfo);
			virtual bool	DeleteFile(FileInfo* pInfo);
			virtual bool	ReadFile(FileInfo* pInfo, void* buffer, u64 count, u64& outNumBytesRead);
			virtual bool	WriteFile(FileInfo* pInfo, const void* buffer, u64 count, u64& outNumBytesWritten);

			virtual bool	SeekOrigin(FileInfo* pInfo, u64 pos, u64& newPos);
			virtual bool	SeekCurrent(FileInfo* pInfo, u64 pos, u64& newPos);
			virtual bool	SeekEnd(FileInfo* pInfo, u64 pos, u64& newPos);

			virtual bool	Sync(FileInfo* pInfo);

			enum EPs3SeekMode
			{
				SEEK_ORIGIN = CELL_FS_SEEK_SET,
				SEEK_CURRENT = CELL_FS_SEEK_CUR,
				SEEK_END = CELL_FS_SEEK_END,
			};
			virtual bool	Seek(FileInfo* pInfo, EPs3SeekMode mode, u64 pos, u64& newPos);

			virtual bool	GetBlockSize(FileInfo* pInfo, u64& outSectorSize);
		}


		bool xfiledevice_ps3system::OpenOrCreateFile(FileInfo* pInfo)
		{
			s32	nFlags;
			if(pInfo->m_boWriting)
			{
				nFlags	= CELL_FS_O_CREAT | CELL_FS_O_RDWR;
			}
			else
			{
				nFlags	= CELL_FS_O_RDONLY;
			}

			s32	hFile	= INVALID_FILE_HANDLE;
			s32	nResult = cellFsOpen (pInfo->m_szFilename, nFlags, &hFile, NULL, 0);
			pInfo->m_nFileHandle = (u32)hFile;

			bool boSuccess = (nResult == CELL_OK);
			return boSuccess;
		}

		bool xfiledevice_ps3system::LengthOfFile(FileInfo* pInfo, u64& outLength)
		{
			CellFsStat stats;
			CellFsErrno nResult = cellFsStat(pInfo->m_szFilename, &stats);
			bool boSuccess = (nResult == CELL_OK);
			if (boSuccess)
				outLength = stats.st_size;
			else
				outLength = -1;

			return boSuccess;
		}

		bool xfiledevice_ps3system::CloseFile(FileInfo* pInfo)
		{
			s32 nResult = cellFsClose (pInfo->m_nFileHandle);
			bool boSuccess = false;
			if (nResult == CELL_OK)
			{
				pInfo->m_nFileHandle = (u32)INVALID_FILE_HANDLE;
				boSuccess = true;
			}
			return boSuccess;
		}

		bool xfiledevice_ps3system::DeleteFile(FileInfo* pInfo)
		{
			s32 nResult = cellFsUnlink(pInfo->m_szFilename);
			bool boSuccess = false;
			if (nResult == CELL_OK)
			{
				pInfo->m_nFileHandle = (u32)INVALID_FILE_HANDLE;
				boSuccess = true;
			}
			return boSuccess;
		}

		bool xfiledevice_ps3system::ReadFile(FileInfo* pInfo, void* buffer, uintfs count, u64& outNumBytesRead)
		{
			u64 numBytesRead;
			s32 nResult = cellFsRead (pInfo->m_nFileHandle, buffer, count, &numBytesRead);

			bool boSuccess = (nResult == CELL_OK);
			if (boSuccess)
			{
				outNumBytesRead = (u32)numBytesRead;
			}
			else
			{ 
				outNumBytesRead = 0;
			}
			return boSuccess;

		}
		bool xfiledevice_ps3system::WriteFile(FileInfo* pInfo, const void* buffer, uintfs count, u64& outNumBytesWritten)
		{
			u64 numBytesWritten;
			s32 nResult = cellFsWrite (pInfo->m_nFileHandle, buffer, count, &numBytesWritten);

			bool boSuccess = (nResult == CELL_OK);
			if (boSuccess)
			{
				outNumBytesWritten = (u32)numBytesWritten;
			}
			else
			{ 
				outNumBytesWritten = 0;
			}
			return boSuccess;
		}

		bool xfiledevice_ps3system::Seek(FileInfo* pInfo, EPs3SeekMode mode, u64 pos, u64& newPos)
		{
			u64	nPos;
			s32 nResult = cellFsLseek (pInfo->m_nFileHandle, pos, mode, &nPos);
			bool boSuccess = (nResult == CELL_OK);
			if (boSuccess)
			{
				newPos = nPos;
			}
			else
			{ 
				newPos = pos;
			}
			return boSuccess;
		}
		bool	xfiledevice_ps3system::SeekOrigin(FileInfo* pInfo, u64 pos, u64& newPos)
		{
			return Seek(pInfo, SEEK_ORIGIN, pos, newPos);
		}

		bool	xfiledevice_ps3system::SeekCurrent(FileInfo* pInfo, u64 pos, u64& newPos)
		{
			return Seek(pInfo, SEEK_CURRENT, pos, newPos);
		}

		bool	xfiledevice_ps3system::SeekEnd(FileInfo* pInfo, u64 pos, u64& newPos)
		{
			return Seek(pInfo, SEEK_END, pos, newPos);
		}


		static bool __Stats(FileInfo* pInfo)
		{
			CellFsStat stat;
			s32 nResult = cellFsFstat(pInfo->m_nFileHandle, &stat);

			//@TODO: use stats
			bool boSuccess = (nResult == CELL_OK);
			return boSuccess;
		}

		bool xfiledevice_ps3system::Sync(FileInfo* pInfo)
		{
			s32 nResult = cellFsFsync(pInfo->m_nFileHandle);
			return nResult == CELL_OK;
		}

		bool xfiledevice_ps3system::GetBlockSize(FileInfo* pInfo, u64& outSectorSize)
		{
			u64	uSectorSize;
			u64	uBlockSize;
			s32 nResult = cellFsFGetBlockSize(pInfo->m_nFileHandle, &uSectorSize, &uBlockSize);
			bool boSuccess = (nResult == CELL_OK);
			if (boSuccess)
			{
				outSectorSize = uSectorSize;
			}
			else
			{ 
				outSectorSize = 0;
			}
			return boSuccess;
		}
	};


	namespace xfilesystem
	{
		using namespace __private;

		//--------
		// Defines
		//--------
		#define FS_ASYNC_WORKER_THREAD_STACK_SIZE				(0x4000) // 16 kb
		#define FS_ASYNC_WORKER_THREAD_EXIT_CODE				(0xbee)
		#define FS_ASYNC_WORKER_THREAD_PRIO						(0)

		// the event queue size 1~127
		#define	FS_ASYNC_WORKER_THREAD_EVENT_QUEUE_SIZE			32
		
		//---------
		// Forward declares
		//---------

		static sys_ppu_thread_t			m_AsyncIOThread;
		static sys_event_queue_t		m_xQueue;
		static sys_event_port_t			m_xPort;

		static u32						m_uFileListLength = 0;
		static char**					m_pszFileListData = NULL;


		//----------------
		// Private Methods
		//----------------

		void AsyncIOWorkerThread( u64 param )
		{
			sys_event_queue_t	xQueue = (sys_event_queue_t)param;
			s32					nResult;

			while (xTRUE)
			{
				sys_event_t xEvent;
				nResult = sys_event_queue_receive(xQueue, &xEvent, 0);

				if (nResult == ECANCELED)
				{
					break;
				}
				else if(nResult == CELL_OK)
				{
					AsyncIOInfo* pAsync = AsyncIORemoveHead();

					if(pAsync)
					{
						if(pAsync->m_nFileIndex >=  0)
						{
							// Something like
							xifiledevice* device = GetFileDevice(pAsync->m_nFileIndex);
							device->HandleAsync(pAsync);
						}

						FreeAsyncIOAddToTail(pAsync);
					}
				}
			}
			x_printf("Stdio:"TARGET_PLATFORM_STR" INFO AsyncIOWorkerThread Exit\n");
			sys_ppu_thread_exit(0);
		}

		//---------------
		// Public Methods
		//---------------

		void				Initialise ( u32 uAsyncQueueSize, xbool boEnableCache )
		{
			InitialiseCommon(uAsyncQueueSize, boEnableCache);

			s32								nResult;

			sys_event_queue_attribute_t		xQueueAttribute;

			sys_event_queue_attribute_initialize(xQueueAttribute);

			nResult = sys_event_queue_create(&m_xQueue, &xQueueAttribute, SYS_EVENT_QUEUE_LOCAL, FS_ASYNC_WORKER_THREAD_EVENT_QUEUE_SIZE);

			if(nResult != CELL_OK)
			{
				x_printf("Stdio:"TARGET_PLATFORM_STR" ERROR sys_event_queue_create %d\n", x_va_list(nResult));
			}

			nResult = sys_ppu_thread_create(&m_AsyncIOThread, AsyncIOWorkerThread, (u64)m_xQueue, FS_ASYNC_WORKER_THREAD_PRIO, FS_ASYNC_WORKER_THREAD_STACK_SIZE, SYS_PPU_THREAD_CREATE_JOINABLE, "IO Async Read PU Thread");

			if(nResult != CELL_OK)
			{
				x_printf("Stdio:"TARGET_PLATFORM_STR" ERROR creating async IO thread!!!\n");
			}
			else
			{
				x_printf("Update thread (%d) created OK.\n", x_va_list((s32)m_AsyncIOThread));
			}

			sys_ppu_thread_t	xThreadID;

			sys_ppu_thread_get_id(&xThreadID);

			nResult = sys_event_port_create(&m_xPort, SYS_EVENT_PORT_LOCAL, (u64)xThreadID);

			if(nResult != CELL_OK)
			{
				x_printf("Stdio:"TARGET_PLATFORM_STR" ERROR sys_event_port_create %d\n", x_va_list(nResult));
			}

			nResult = sys_event_port_connect_local(m_xPort, m_xQueue);

			if(nResult != CELL_OK)
			{
				x_printf("Stdio:"TARGET_PLATFORM_STR" ERROR sys_event_port_connect_local %d\n", x_va_list(nResult));
			}
		}	

		//------------------------------------------------------------------------------------------

		void Shutdown ( void )
		{
			s32	nResult;
			nResult = sys_event_port_disconnect(m_xPort);

			if(nResult != CELL_OK)
			{
				x_printf("Stdio:"TARGET_PLATFORM_STR" ERROR sys_event_port_disconnect %d\n", x_va_list(nResult));
			}

			nResult = sys_event_port_destroy(m_xPort);

			if(nResult != CELL_OK)
			{
				x_printf("Stdio:"TARGET_PLATFORM_STR" ERROR sys_event_port_destroy %d\n", x_va_list(nResult));
			}

			nResult = sys_event_queue_destroy(m_xQueue, SYS_EVENT_QUEUE_DESTROY_FORCE);

			if(nResult != CELL_OK)
			{
				x_printf("Stdio:"TARGET_PLATFORM_STR" ERROR sys_event_queue_destroy %d\n", x_va_list(nResult));
			}

			u64 u64Status;
			printf("main: Wait for the receiver thread to exit: ");
			nResult = sys_ppu_thread_join(m_AsyncIOThread, &u64Status);

			ShutdownCommon();
		}


		//------------------------------------------------------------------------------------------

		void				GetOpenCreatedTime (u32 uHandle, xdatetime& pTimeAndDate)
		{
			ASSERTS (pTimeAndDate, "GetOpenCreatedTime() : Pointer to xsystem::TimeAndDate is NULL!");

			CellFsStat	xStat;
			FileInfo* pxFileInfo = GetFileInfo(uHandle);
			CellFsErrno eError = cellFsFstat(pxFileInfo->m_nFileHandle, &xStat);

			pTimeAndDate = xdatetime::sFromFileTime(xStat.st_ctime);
		}

		//------------------------------------------------------------------------------------------

		void				GetOpenModifiedTime (u32 uHandle, xdatetime& pTimeAndDate)
		{
			ASSERTS (pTimeAndDate, "GetOpenModifiedTime() : Pointer to xsystem::TimeAndDate is NULL!");

			CellFsStat	xStat;
			FileInfo* pxFileInfo = GetFileInfo(uHandle);
			CellFsErrno eError = cellFsFstat(pxFileInfo->m_nFileHandle, &xStat);

			pTimeAndDate = xdatetime::sFromFileTime(xStat.st_ctime);
		}

		//------------------------------------------------------------------------------------------

		void				ReSize( u32 uHandle, u64 uNewSize )
		{
			FileInfo* pxFileInfo = GetFileInfo(uHandle);

			s32 nResult	= cellFsFtruncate(pxFileInfo->m_nFileHandle, uNewSize);
			if(nResult != CELL_OK)
			{
				x_printf("Stdio:"TARGET_PLATFORM_STR" ERROR cellFsFtruncate %d\n", x_va_list(nResult));
			}
		}

		//------------------------------------------------------------------------------------------

		u64					GetFreeSize( const char* szPath )
		{
			u32	uBlockSize;
			u64	uBlockCount;
			cellFsGetFreeSize(szPath, &uBlockSize, &uBlockCount);

			return uBlockSize * uBlockCount;
		}

		//------------------------------------------------------------------------------------------

		void				CreateFileList( const char* szPath, xbool boRecursive )
		{
			ASSERTS(m_uFileListLength == 0, "CreateFileList Already exists");
			ASSERTS(m_pszFileListData == NULL, "CreateFileList Already exists - memory leak occurring!\n");

			m_uFileListLength	= 0;
			m_pszFileListData	= NULL;

			// Parse the dir twice - once to see memory usage, 2nd to get data
			ParseDir(szPath, boRecursive, m_uFileListLength, m_pszFileListData);

			if(m_uFileListLength > 0)
			{
				// Allocate and fill info
				// m_pszFileListData	= (char**)HeapManager::GetHeap()->AllocFromEnd(m_uFileListLength * sizeof(char*));
				m_pszFileListData = (char**)x_malloc(sizeof(char*), m_uFileListLength, XMEM_FLAG_ALIGN_8B);

				u32	uIndex	= 0;
				ParseDir(szPath, boRecursive, uIndex, m_pszFileListData);
			}
		}

		//------------------------------------------------------------------------------------------

		void			DestroyFileList( void )
		{
			// Done - free all buffers
			for(u32 uFile = 0; uFile < m_uFileListLength; uFile++)
			{
				x_free(m_pszFileListData[uFile]);
			}

			x_free(m_pszFileListData);

			m_uFileListLength	= 0;
			m_pszFileListData	= NULL;
		}

		//------------------------------------------------------------------------------------------

		s32				GetFileListLength	( void )
		{
			return m_uFileListLength;
		}

		//------------------------------------------------------------------------------------------

		const char*		GetFileListData		( u32 nFile )
		{
			if (nFile < m_uFileListLength && m_pszFileListData != NULL)
				return m_pszFileListData[nFile];

			return "";
		}


		//////////////////////////////////////////////////////////////////////////
		// Private xfilesystem functionality
		//////////////////////////////////////////////////////////////////////////
		namespace __private
		{
			//------------------------------------------------------------------------------------------

			void				ParseDir(const char* szDir, xbool boRecursive, u32& ruFileList, char** pszFileList)
			{
				s32				nFd;
				CellFsDirent	xDirInfo;

				XSTRING_BUFFER(szFullPath, FS_MAX_PATH);
				CreateSystemPath(szDir, szFullPath);

				// Start the directory read
				cellFsOpendir(szFullPath.c_str(), &nFd);

				while(1)
				{
					u64	uRead;
					cellFsReaddir(nFd, &xDirInfo, &uRead);

					if(uRead > 0)
					{
						if(xDirInfo.d_type == CELL_FS_TYPE_DIRECTORY)
						{
							if(	(boRecursive == true) && (x_stricmp(xDirInfo.d_name, ".") != 0) && (x_stricmp(xDirInfo.d_name, "..") != 0) )
							{
								// Directory (and not . or ..) - dive in and keep going...
								XSTRING_BUFFER(szPath, 512);
								szPath = szDir;
								szPath += xDirInfo.d_name;

								ParseDir(szPath.c_str(), boRecursive, ruFileList, pszFileList);
							}
						}
						else
						{
							if(pszFileList)
							{
								// Allocate enough memory for the new entry, and copy it over
								// pszFileList[ruFileList]	= (char*)HeapManager::GetHeap()->AllocFromEnd(x_strlen(szDir) + xDirInfo.d_namlen + 1);
								s32 maxLen = x_strlen(szDir) + xDirInfo.d_namlen + 1;
								pszFileList[ruFileList]	= (char*)x_malloc(sizeof(xbyte), maxLen, XMEM_FLAG_ALIGN_8B);
								x_strcpy(pszFileList[ruFileList], maxLen, szDir);
								x_strcat(pszFileList[ruFileList], maxLen, xDirInfo.d_name);
							}

							ruFileList++;
						}
					}
					else
					{
						break;
					}
				}

				// Done
				cellFsClosedir(nFd);
			}

			//------------------------------------------------------------------------------------------

			xbool				IsPathUNIXStyle		( void )
			{
				return true;
			}

			//------------------------------------------------------------------------------------------

			void				AsyncIOWorkerResume()
			{
				s32 nResult = sys_event_port_send(m_xPort, 0, 0, 0);
				if (nResult != CELL_OK)
				{
					x_printf("Stdio:"TARGET_PLATFORM_STR" ERROR AsyncIOWorkerResume %d\n", x_va_list(nResult));
				}		
			}

			//------------------------------------------------------------------------------------------
			///< Synchronous file operations

			u32					SyncOpen			( const char* szName, xbool boWrite, xbool boRetry)
			{
				u32 uHandle = AsyncPreOpen(szName, boWrite);
				FileInfo* pxFileInfo = GetFileInfo(uHandle);
				if (!__OpenOrCreateFile(pxFileInfo))
				{
					x_printf ("__OpenOrCreateFile failed on file %s\n", x_va_list(pxFileInfo->m_szFilename));
					pxFileInfo->clear();
					uHandle = (u32)INVALID_FILE_HANDLE;
					SetLastError(FILE_ERROR_NO_FILE);
				}
				return uHandle;
			}

			//------------------------------------------------------------------------------------------

			uintfs				SyncSize			( u32 uHandle )
			{
				if (uHandle==(u32)INVALID_FILE_HANDLE)
				{
					SetLastError(FILE_ERROR_BADF);
					return 0;
				}

				FileInfo* pxFileInfo = GetFileInfo(uHandle);
				u64 length;
				if (!__LengthOfFile(pxFileInfo, length))
				{
					x_printf ("__LengthOfFile failed on file %s\n", x_va_list(pxFileInfo->m_szFilename));
				}
				return length;
			}

			//------------------------------------------------------------------------------------------

			void				SyncRead			( u32 uHandle, uintfs uOffset, uintfs uSize, void* pBuffer, xbool boRetry)
			{
				if (uHandle==(u32)INVALID_FILE_HANDLE)
				{
					SetLastError(FILE_ERROR_BADF);
					return;
				}

				FileInfo* pxFileInfo = GetFileInfo(uHandle);
				u64 newOffset;
				if (!__Seek(pxFileInfo, __SEEK_ORIGIN, uOffset, newOffset))
				{
					x_printf ("__Seek failed on file %s\n", x_va_list(pxFileInfo->m_szFilename));
				}
				u64 numBytesRead;
				if (!__ReadFile(pxFileInfo, pBuffer, uSize, numBytesRead))
				{
					x_printf ("__ReadFile failed on file %s\n", x_va_list(pxFileInfo->m_szFilename));
				}
			}

			//------------------------------------------------------------------------------------------

			void				SyncWrite			( u32 uHandle, uintfs uOffset, uintfs uSize, const void* pBuffer, xbool boRetry)
			{
				if (uHandle==(u32)INVALID_FILE_HANDLE)
				{
					SetLastError(FILE_ERROR_BADF);
					return;
				}

				FileInfo* pxFileInfo = GetFileInfo(uHandle);
				u64 newOffset;
				if (!__Seek(pxFileInfo, __SEEK_ORIGIN, uOffset, newOffset))
				{
					x_printf ("__Seek failed on file %s\n", x_va_list(pxFileInfo->m_szFilename));
				}
				u64 numBytesWritten;
				if (!__WriteFile(pxFileInfo, pBuffer, uSize, numBytesWritten))
				{
					x_printf ("__WriteFile failed on file %s\n", x_va_list(pxFileInfo->m_szFilename));
				}
			}

			//------------------------------------------------------------------------------------------

			void 				SyncClose			( u32& uHandle )
			{
				if (uHandle==(u32)INVALID_FILE_HANDLE)
				{
					SetLastError(FILE_ERROR_BADF);
					return;
				}

				FileInfo* pxFileInfo = GetFileInfo(uHandle);
				if (!__CloseFile(pxFileInfo))
				{
					x_printf ("__CloseFile failed on file %s\n", x_va_list(pxFileInfo->m_szFilename));
				}
				pxFileInfo->clear();
				uHandle = (u32)INVALID_FILE_HANDLE;
			}

			//------------------------------------------------------------------------------------------

			void				SyncDelete			( u32& uHandle )
			{
				if (uHandle==(u32)INVALID_FILE_HANDLE)
				{
					SetLastError(FILE_ERROR_BADF);
					return;
				}

				FileInfo* pxFileInfo = GetFileInfo(uHandle);
				if (!__CloseFile(pxFileInfo))
				{
					x_printf ("__CloseFile failed on file %s\n", x_va_list(pxFileInfo->m_szFilename));
				}
				__DeleteFile(pxFileInfo);
				pxFileInfo->clear();
				uHandle = (u32)INVALID_FILE_HANDLE;
			}
		}
	};


	//==============================================================================
	// END xCore namespace
	//==============================================================================
};

#endif // TARGET_PS3