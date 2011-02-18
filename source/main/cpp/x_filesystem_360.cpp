#include "xbase\x_target.h"
#ifdef TARGET_360

//==============================================================================
// INCLUDES
//==============================================================================
#include <Xtl.h>
#ifndef TARGET_FINAL
	#include <xbdm.h>
#endif

#include "xbase\x_debug.h"
#include "xbase\x_string_std.h"
#include "xbase\x_va_list.h"

#include "xfilesystem\x_filesystem_common.h"
#include "xfilesystem\x_filesystem_360.h"

namespace xcore
{
	//------------------------------------------------------------------------------------------
	//---------------------------------- 360 IO Functions ------------------------------------
	//------------------------------------------------------------------------------------------
	namespace xfilesystem
	{
		using namespace __private;

		static bool __OpenOrCreateFile(FileInfo* pInfo)
		{
			u32 shareType	= FILE_SHARE_READ;
			u32 fileMode	= GENERIC_WRITE|GENERIC_READ;
			u32 disposition	= 0;
			if(pInfo->m_boWriting)
			{
				disposition	= CREATE_ALWAYS;
			}
			else
			{
				fileMode	&= ~GENERIC_WRITE;
				disposition	= OPEN_EXISTING;
			}

			// FILE_FLAG_OVERLAPPED     -    This allows asynchronous I/O.
			// FILE_FLAG_NO_BUFFERING   -    No cached asynchronous I/O.
			u32 attrFlags	= FILE_ATTRIBUTE_NORMAL;

			HANDLE handle = CreateFile(pInfo->m_szFilename, fileMode, shareType, NULL, disposition, attrFlags, NULL);
			pInfo->m_nFileHandle = (u32)handle;
			return pInfo->m_nFileHandle != (u32)INVALID_HANDLE_VALUE;
		}

		static bool __LengthOfFile(FileInfo* pInfo, u64& outLength)
		{
			DWORD lowSize, highSize;
			lowSize = ::GetFileSize((HANDLE)pInfo->m_nFileHandle, &highSize);
			outLength = highSize;
			outLength = outLength << 16;
			outLength = outLength << 16;
			outLength = outLength | lowSize;
			return true;
		}

		static bool __CloseFile(FileInfo* pInfo)
		{
			if (!CloseHandle((HANDLE)pInfo->m_nFileHandle))
				return false;
			return true;
		}

		static bool __DeleteFile(FileInfo* pInfo)
		{
			if (!__CloseFile(pInfo))
				return false;
			if (!DeleteFile(pInfo->m_szFilename))
				return false;
			return true;
		}

		static bool __ReadFile(FileInfo* pInfo, void* buffer, uintfs count, u64& outNumBytesRead)
		{
			DWORD numBytesRead;
			xbool boSuccess = ReadFile((HANDLE)pInfo->m_nFileHandle, buffer, (DWORD)count, &numBytesRead, NULL); 

			if (boSuccess)
			{
				outNumBytesRead = numBytesRead;
			}

			if (!boSuccess) 
			{ 
				outNumBytesRead = -1;

				DWORD dwError = ::GetLastError();
				switch(dwError) 
				{ 
				case ERROR_HANDLE_EOF:											// We have reached the end of the FilePC during the call to ReadFile 
					return false;
				case ERROR_IO_PENDING: 
					return false; 
				default:
					return false;
				}
			}

			return true;
		}
		static bool __WriteFile(FileInfo* pInfo, const void* buffer, uintfs count, u64& outNumBytesWritten)
		{
			DWORD numBytesWritten;
			xbool boSuccess = WriteFile((HANDLE)pInfo->m_nFileHandle, buffer, (DWORD)count, &numBytesWritten, NULL); 

			if (boSuccess)
			{
				outNumBytesWritten = numBytesWritten;
			}

			if (!boSuccess) 
			{ 
				outNumBytesWritten = -1;

				DWORD dwError = ::GetLastError();
				switch(dwError) 
				{ 
				case ERROR_HANDLE_EOF:											// We have reached the end of the FilePC during the call to WriteFile 
					return false;
				case ERROR_IO_PENDING: 
					return false; 
				default:
					return false;
				}
			}

			return true;
		}
		enum EPcSeekMode
		{
			__SEEK_ORIGIN = 1,
			__SEEK_CURRENT = 2,
			__SEEK_END = 3,
		};

		static bool __Seek(FileInfo* pInfo, EPcSeekMode mode, u64 pos, u64& newPos)
		{
			s32 hardwareMode = 0;
			switch(mode)
			{
			case __SEEK_ORIGIN : hardwareMode = FILE_BEGIN; break;
			case __SEEK_CURRENT: hardwareMode = FILE_CURRENT; break;
			case __SEEK_END    : hardwareMode = FILE_END; break; 
			default: 
				ASSERT(0);
				break;
			}

			// seek!
			LARGE_INTEGER position;
			LARGE_INTEGER newFilePointer;

			newPos = pos;
			position.LowPart  = (u32)pos;
			position.HighPart = 0;
			DWORD result = SetFilePointerEx((HANDLE)pInfo->m_nFileHandle, position, &newFilePointer, hardwareMode);
			if (!result)
			{
				if (result == INVALID_SET_FILE_POINTER) 
				{
					// Failed to seek.
				}
				return false;
			}
			newPos = newFilePointer.LowPart;
			return true;
		}

		static bool __GetBlockSize(FileInfo* pInfo, u64& uSectorSize)
		{
			uSectorSize = 2048;
			return true;
		}

		static bool __Sync(FileInfo* pInfo)
		{
			return true;
		}
	};



	namespace xfilesystem
	{
		using namespace __private;

		//--------
		// Defines
		//--------
		#define FS_ASYNC_WORKER_THREAD_STACK_SIZE              (0x4000) // 16 kb
		#define FS_ASYNC_WORKER_THREAD_EXIT_CODE               (0xbee)
		#define FS_ASYNC_WORKER_THREAD_PRIO             (0)

		#define	QUEUE_SIZE				8
		
		//---------
		// Forward declares
		//---------

		static HANDLE					m_AsyncIOThreadHandle;
		static s32						m_AsyncIOThreadID;
		static HANDLE					m_Signal = NULL;

		//static FileInfo					m_OpenAsyncFile[FS_MAX_OPENED_FILES];

		//static QueueItem				m_aAsyncQueue[FS_MAX_ASYNC_QUEUE_ITEMS];
		//static xmtllist<QueueItem>		m_pAsyncQueueList[FS_PRIORITY_COUNT];
		//static xmtllist<QueueItem>		m_pFreeQueueItemList;

		//static AsyncIOInfo				m_AsyncIOData[FS_MAX_ASYNC_IO_OPS];
		//static xmtllist<AsyncIOInfo>	m_pFreeAsyncIOList;
		//static xmtllist<AsyncIOInfo>	m_pAsyncIOList;

		static u32						m_uFileListLength = 0;
		static char**					m_pszFileListData = NULL;


		//----------------
		// Private Methods
		//----------------

		void AsyncIOWorkerThread( u64 param )
		{
			while (xTRUE)
			{
				AsyncIOInfo* pAsync = AsyncIORemoveHead(); //m_pAsyncIOList.removeHead();

				if(pAsync)
				{
					if(pAsync->m_nFileIndex >=  0)
					{
						FileInfo* pxInfo = GetFileInfo(pAsync->m_nFileIndex);//&m_OpenAsyncFile[pAsync->m_nFileIndex];

						if(pAsync->m_nStatus == FILE_OP_STATUS_OPEN_PENDING)
						{
							pAsync->m_nStatus = FILE_OP_STATUS_OPENING;

							bool boError   = false;
							bool boSuccess = __OpenOrCreateFile(pxInfo);
							if (!boSuccess)
							{
								x_printf ("__OpenOrCreateFile failed on file %s\n", x_va_list(pxInfo->m_szFilename));
								boError = true;
							}
							else
							{
								u64 nPos;
								boSuccess = __Seek(pxInfo, __SEEK_END, 0, nPos);
								if ((!boSuccess) || ((pxInfo->m_boWriting == false) && (nPos == 0)) )
								{
									x_printf ("__Seek failed on file %s\n", x_va_list(pxInfo->m_szFilename));
									boError = true;
								}
								else
								{
									u64 uSize = nPos; 
									boSuccess = __Seek(pxInfo, __SEEK_ORIGIN, 0, nPos);
									if (!boSuccess)
									{
										x_printf ("__Seek failed on file %s\n", x_va_list(pxInfo->m_szFilename));
										boError = true;
									}
									else
									{
										u64	uSectorSize = 4096;
										{
											u64 uPad = uSize % uSectorSize;
											if (uPad != 0)
											{
												uPad = uSectorSize - uPad;
											}

											u32 uRoundedSize			= (u32)(uSize + uPad);
											u32 uNumSectors 			= (u32)(uRoundedSize / uSectorSize);

											pxInfo->m_uByteOffset		= 0;
											pxInfo->m_uByteSize			= uSize;
											pxInfo->m_uSectorOffset		= 0;
											pxInfo->m_uNumSectors		= uNumSectors;
											pxInfo->m_uSectorSize		= uSectorSize;
										}
									}
								}
							}
							if (boError)
							{
								if (pxInfo->m_nFileHandle != (u32)INVALID_FILE_HANDLE)
								{
									bool boClose = __CloseFile(pxInfo);
									if (!boClose)
									{
										x_printf ("__CloseFile failed on file %s\n", x_va_list(pxInfo->m_szFilename));
									}
									pxInfo->m_nFileHandle = (u32)INVALID_FILE_HANDLE;
								}
							}

							pAsync->m_nStatus	= FILE_OP_STATUS_DONE;
						}
						else if(pAsync->m_nStatus == FILE_OP_STATUS_CLOSE_PENDING)
						{
							pAsync->m_nStatus	= FILE_OP_STATUS_CLOSING;

							bool boClose = __CloseFile(pxInfo);
							if (!boClose)
							{
								x_printf ("__CloseFile failed on file %s\n", x_va_list(pxInfo->m_szFilename));
							}
							
							pxInfo->m_nFileHandle	= (u32)INVALID_FILE_HANDLE;
							pAsync->m_nStatus	= FILE_OP_STATUS_DONE;
						}
						else if(pAsync->m_nStatus == FILE_OP_STATUS_DELETE_PENDING)
						{
							pAsync->m_nStatus	= FILE_OP_STATUS_DELETING;

							bool boClose = __CloseFile(pxInfo);
							if (!boClose)
							{
								x_printf ("__CloseFile failed on file %s\n", x_va_list(pxInfo->m_szFilename));
							}
							else
							{
								bool boDelete = __DeleteFile(pxInfo);
								if (!boDelete)
								{
									x_printf ("__DeleteFile failed on file %s\n", x_va_list(pxInfo->m_szFilename));
								}
							}

							pxInfo->m_nFileHandle	= (u32)INVALID_FILE_HANDLE;
							pAsync->m_nStatus	= FILE_OP_STATUS_DONE;
						}
						else if(pAsync->m_nStatus == FILE_OP_STATUS_STAT_PENDING)
						{
							pAsync->m_nStatus	= FILE_OP_STATUS_STATING;

							//@TODO: use stats

							pAsync->m_nStatus	= FILE_OP_STATUS_DONE;
						}
						else if(pAsync->m_nStatus == FILE_OP_STATUS_READ_PENDING)
						{
							u64	nPos;
							bool boSeek = __Seek(pxInfo, __SEEK_ORIGIN, pAsync->m_uReadWriteOffset, nPos);
							if (!boSeek)
							{
								x_printf ("__Seek failed on file %s\n", x_va_list(pxInfo->m_szFilename));
							}

							pAsync->m_nStatus	= FILE_OP_STATUS_READING;

							u64 nReadSize;
							bool boRead = __ReadFile(pxInfo, pAsync->m_pReadAddress, (u32)pAsync->m_uReadWriteSize, nReadSize);
							if (!boRead)
							{
								x_printf ("__ReadFile failed on file %s\n", x_va_list(pxInfo->m_szFilename));
							}

							pAsync->m_nStatus	= FILE_OP_STATUS_DONE;
						}
						else if(pAsync->m_nStatus == FILE_OP_STATUS_WRITE_PENDING)
						{
							u64	nPos;
							bool boSeek = __Seek(pxInfo, __SEEK_ORIGIN, pAsync->m_uReadWriteOffset, nPos);
							if (!boSeek)
							{
								x_printf ("__Seek failed on file %s\n", x_va_list(pxInfo->m_szFilename));
							}

							pAsync->m_nStatus	= FILE_OP_STATUS_WRITING;

							u64 nWriteSize;
							bool boWrite = __WriteFile(pxInfo, pAsync->m_pWriteAddress, (u32)pAsync->m_uReadWriteSize, nWriteSize);
							if (!boWrite)
							{
								x_printf ("__WriteFile failed on file %s\n", x_va_list(pxInfo->m_szFilename));
							}

							if (pxInfo->m_eSource != FS_SOURCE_HOST)
							{
								bool boSyncResult = __Sync(pxInfo);
								if (!boSyncResult)
								{
									x_printf ("__Sync failed on file %s\n", x_va_list(pxInfo->m_szFilename));
								}
							}

							pAsync->m_nStatus	= FILE_OP_STATUS_DONE;
						}
					}
						FreeAsyncIOAddToTail(pAsync);
				}
				else
				{
					WaitForSingleObject(m_Signal, INFINITE);
				}
			}
		}

		//---------------
		// Public Methods
		//---------------

		void				Initialise ( u32 uAsyncQueueSize, xbool boEnableCache )
		{
			InitialiseCommon(uAsyncQueueSize, boEnableCache);

#ifndef TARGET_FINAL
			HRESULT devkitDriveResult = DmMapDevkitDrive();
			if (devkitDriveResult != S_OK)
			{
				x_printf("Stdio:"TARGET_PLATFORM_STR" ERROR devkit drive could not be mounted");
			}
#endif

			m_AsyncIOThreadHandle = CreateThread(NULL, FS_ASYNC_WORKER_THREAD_STACK_SIZE, (LPTHREAD_START_ROUTINE)AsyncIOWorkerThread, NULL, CREATE_SUSPENDED, (DWORD*)&m_AsyncIOThreadID);
			SetThreadPriority(m_AsyncIOThreadHandle, THREAD_PRIORITY_HIGHEST);

			m_Signal = CreateEvent(NULL, FALSE, FALSE, NULL);
			ResumeThread( m_AsyncIOThreadHandle );
			
		}	

		//------------------------------------------------------------------------------------------

		void				Shutdown ( void )
		{
			CloseHandle(m_AsyncIOThreadHandle);
			CloseHandle(m_Signal);

			ShutdownCommon();
		}

		//------------------------------------------------------------------------------------------

		void				GetOpenCreatedTime (u32 uHandle, xdatetime& pTimeAndDate)
		{
			ASSERTS (pTimeAndDate, "GetOpenCreatedTime() : Pointer to xsystem::TimeAndDate is NULL!");

			//CellFsStat	xStat;
			//CellFsErrno eError = cellFsFstat(m_OpenAsyncFile[uHandle].m_nFileHandle, &xStat);

			pTimeAndDate = xdatetime::sFromFileTime(0);
		}

		//------------------------------------------------------------------------------------------

		void				GetOpenModifiedTime (u32 uHandle, xdatetime& pTimeAndDate)
		{
			ASSERTS (pTimeAndDate, "GetOpenModifiedTime() : Pointer to xsystem::TimeAndDate is NULL!");

			//CellFsStat	xStat;
			//CellFsErrno eError = cellFsFstat(m_OpenAsyncFile[uHandle].m_nFileHandle, &xStat);

			pTimeAndDate = xdatetime::sFromFileTime(0);
		}

		//------------------------------------------------------------------------------------------

		void				ReSize( u32 uHandle, u64 uNewSize )
		{
			FileInfo* pInfo = GetFileInfo(uHandle);//&m_OpenAsyncFile[uHandle];

			s32 nResult=-1;
// 			nResult	= cellFsFtruncate(pInfo->m_nFileHandle, uNewSize);
			if(nResult < 0)
			{
				x_printf("Stdio:"TARGET_PLATFORM_STR" ERROR ReSize %d\n", x_va_list(nResult));
			}
		}

		//------------------------------------------------------------------------------------------

		u64					GetFreeSize( const char* szPath )
		{
			u32	uBlockSize = 0;
			u64	uBlockCount = 0;
// 			cellFsGetFreeSize(szPath, &uBlockSize, &uBlockCount);

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
				// 			s32				nFd;
				// 			CellFsDirent	xDirInfo;
				// 
				// 			XSTRING_BUFFER(szFullPath, FS_MAX_PATH);
				// 			CreateSystemPath(szDir, szFullPath);
				// 
				// 			// Start the directory read
				// 			cellFsOpendir(szFullPath.c_str(), &nFd);
				// 
				// 			while(1)
				// 			{
				// 				u64	uRead;
				// 				cellFsReaddir(nFd, &xDirInfo, &uRead);
				// 
				// 				if(uRead > 0)
				// 				{
				// 					if(xDirInfo.d_type == CELL_FS_TYPE_DIRECTORY)
				// 					{
				// 						if(	(boRecursive == true) && (x_stricmp(xDirInfo.d_name, ".") != 0) && (x_stricmp(xDirInfo.d_name, "..") != 0) )
				// 						{
				// 							// Directory (and not . or ..) - dive in and keep going...
				// 							XSTRING_BUFFER(szPath, 512);
				// 							szPath = szDir;
				// 							szPath += xDirInfo.d_name;
				// 
				// 							ParseDir(szPath.c_str(), boRecursive, ruFileList, pszFileList);
				// 						}
				// 					}
				// 					else
				// 					{
				// 						if(pszFileList)
				// 						{
				// 							// Allocate enough memory for the new entry, and copy it over
				// 							// pszFileList[ruFileList]	= (char*)HeapManager::GetHeap()->AllocFromEnd(x_strlen(szDir) + xDirInfo.d_namlen + 1);
				// 							s32 maxLen = x_strlen(szDir) + xDirInfo.d_namlen + 1;
				// 							pszFileList[ruFileList]	= (char*)x_malloc(sizeof(xbyte), maxLen, XMEM_FLAG_ALIGN_8B);
				// 							x_strcpy(pszFileList[ruFileList], maxLen, szDir);
				// 							x_strcat(pszFileList[ruFileList], maxLen, xDirInfo.d_name);
				// 						}
				// 
				// 						ruFileList++;
				// 					}
				// 				}
				// 				else
				// 				{
				// 					break;
				// 				}
				// 			}
				// 
				// 			// Done
				// 			cellFsClosedir(nFd);

			}

			//------------------------------------------------------------------------------------------

			xbool			IsPathUNIXStyle		( void )
			{
				return false;
			}

			//------------------------------------------------------------------------------------------

			void				AsyncIOWorkerResume()
			{
				SetEvent(m_Signal);
			}


			//------------------------------------------------------------------------------------------
			///< Synchronous file operations

			u32					SyncOpen			( const char* szName, xbool boWrite, xbool boRetry)
			{
				u32 uHandle = AsyncPreOpen(szName, boWrite);
				FileInfo* pxFileInfo = GetFileInfo(uHandle);
				if (!__OpenOrCreateFile(pxFileInfo))
				{
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
				__LengthOfFile(pxFileInfo, length);
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
				__Seek(pxFileInfo, __SEEK_ORIGIN, uOffset, newOffset);
				u64 numBytesRead;
				__ReadFile(pxFileInfo, pBuffer, uSize, numBytesRead);
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
				__Seek(pxFileInfo, __SEEK_ORIGIN, uOffset, newOffset);
				u64 numBytesWritten;
				__WriteFile(pxFileInfo, pBuffer, uSize, numBytesWritten);
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
				__CloseFile(pxFileInfo);
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
				__CloseFile(pxFileInfo);
				__DeleteFile(pxFileInfo);
				pxFileInfo->clear();
				uHandle = (u32)INVALID_FILE_HANDLE;
			}
		};
	};


	//==============================================================================
	// END xCore namespace
	//==============================================================================
};

#endif // TARGET_360