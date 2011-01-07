#include "../x_target.h"
#ifdef TARGET_WII

#include <revolution.h>
#include <revolution\hio2.h>
#include <revolution\nand.h>
#include <revolution\dvd.h>

//==============================================================================
// INCLUDES
//==============================================================================

#include "../x_types.h"
#include "../x_debug.h"
#include "../x_stdio.h"
#include "../x_container.h"
#include "../x_string.h"
#include "../x_time.h"
#include "../x_llist.h"
#include "../x_system.h"

#include "x_filesystem_common.h"
#include "x_filesystem_wii.h"

//==============================================================================
// xCore namespace
//==============================================================================
namespace xcore
{
	namespace xfilesystem
	{
		static DVDFileInfo		mDvdFileInfo[FS_MAX_OPENED_FILES];
		static DVDFileInfo*		GetDvdFileInfo(u32 uHandle)						{ return &mDvdFileInfo[uHandle]; }

		//------------------------------------------------------------------

		static xbool IsDvdInUse( void )
		{
			s32 dvdStatus = DVDGetDriveStatus();

			xbool bError = xFALSE;
			xbool bBusy = xFALSE;
			switch (dvdStatus)
			{
			case DVD_STATE_FATAL_ERROR	:									///< Fatal error occurred.
				bError = xTRUE;
				break;
			case DVD_STATE_END			:									///< Request complete or no request.
			case DVD_STATE_BUSY			:									///< Request is currently processing.
				bBusy = xTRUE;
				break;
			case DVD_STATE_NO_DISK		:									///< No disk in the drive.
				bError = xTRUE;
				break;
			case DVD_STATE_WRONG_DISK	:									///< Wrong disk in the drive.
				bError = xTRUE;
				break;
			case DVD_STATE_MOTOR_STOPPED:									///< Motor stopped.
			case DVD_STATE_PAUSING		:									///< The device driver is paused using the DVD Pause function.
				break;
			case DVD_STATE_RETRY		:									///< Retry error occurred.
				bError = xTRUE;
				break;
			}

			return bBusy;
		}
	};

	//------------------------------------------------------------------------------------------
	//---------------------------------- WII IO Functions ------------------------------------
	//------------------------------------------------------------------------------------------
	namespace xfilesystem
	{
		static bool __OpenOrCreateFile(FileInfo* pInfo)
		{
			DVDFileInfo* dvdFileInfo = GetDvdFileInfo(pInfo->m_nFileIndex);
			pInfo->m_nFileHandle = (u32)dvdFileInfo;
			xbool boSuccess = DVDOpen(pInfo->m_szFilename, dvdFileInfo);
			return boSuccess;
		}

		static bool __LengthOfFile(FileInfo* pInfo, u64& outLength)
		{
			DVDFileInfo* dvdFileInfo = GetDvdFileInfo(pInfo->m_nFileIndex);
			u64 l = (u64)DVDGetLength(dvdFileInfo);
			l = (l + 31) & 0xffffffe0;											///< DVDRead requires size to be a multiple of 32 so we better return a file size that fulfills this requirement
			outLength = l;
			return true;
		}

		static bool __CloseFile(FileInfo* pInfo)
		{
			DVDFileInfo* dvdFileInfo = GetDvdFileInfo(pInfo->m_nFileIndex);
			xbool boSuccess = DVDClose(dvdFileInfo);
			pInfo->m_nFileHandle = INVALID_FILE_HANDLE;
			return boSuccess;
		}

		static bool __DeleteFile(FileInfo* pInfo)
		{
			return true;
		}

		static bool __ReadFile(FileInfo* pInfo, void* buffer, s32 size, u64 offset, u64& outNumBytesRead)
		{
			DVDFileInfo* dvdFileInfo = GetDvdFileInfo(pInfo->m_nFileIndex);
			size = (size + 31) & 0xffffffe0;									///< DVDRead requires size to be a multiple of 32
			s32 numBytesRead = DVDRead(dvdFileInfo, buffer, size, offset);
			bool boSuccess = numBytesRead >= 0;
			if (boSuccess)
			{
				outNumBytesRead = numBytesRead;
			}
			return boSuccess;
		}
		static bool __WriteFile(FileInfo* pInfo, const void* buffer, s32 count, u64& outNumBytesWritten)
		{
			return true;
		}

		enum EWiiSeekMode
		{
			__SEEK_ORIGIN = 1,
			__SEEK_CURRENT = 2,
			__SEEK_END = 3,
		};

		static bool __Seek(FileInfo* pInfo, EWiiSeekMode mode, u64 pos, u64& newPos)
		{
			DVDFileInfo* dvdFileInfo = GetDvdFileInfo(pInfo->m_nFileIndex);
			if (mode == __SEEK_END)
				pos = (u64)DVDGetLength(dvdFileInfo) - pos;
			pos = (pos + 3) & 0xfffffffc;
			s32 nResult = DVDSeek(dvdFileInfo, (s32)pos);
			if (nResult==0)
				newPos = pos;
			return nResult==0;
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

	
		//---------
		// Forward declares
		//---------

		static OSThread					m_AsyncIOThread;
		static xbyte					m_pAsyncIOThreadStack[FS_ASYNC_WORKER_THREAD_STACK_SIZE];

		static FileInfo					m_OpenAsyncFile[FS_MAX_OPENED_FILES];

		static QueueItem				m_aAsyncQueue[FS_MAX_ASYNC_QUEUE_ITEMS];
		static xmtllist<QueueItem>		m_pAsyncQueueList[FS_PRIORITY_COUNT];
		static xmtllist<QueueItem>		m_pFreeQueueItemList;

		static AsyncIOInfo				m_AsyncIOData[FS_MAX_ASYNC_IO_OPS];
		static xmtllist<AsyncIOInfo>	m_pFreeAsyncIOList;
		static xmtllist<AsyncIOInfo>	m_pAsyncIOList;

		static u32						m_uFileListLength = 0;
		static char**					m_pszFileListData = NULL;


		//----------------
		// Private Methods
		//----------------

		void*	AsyncIOWorkerThread(void*)
		{
			while (xTRUE)
			{
				AsyncIOInfo* pAsync = AsyncIORemoveHead();
				if(pAsync)
				{
					if(pAsync->m_nFileIndex >=  0)
					{
						FileInfo* pxInfo = GetFileInfo(pAsync->m_nFileIndex);

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
								u64 uSize; 
								boSuccess = __LengthOfFile(pxInfo, uSize);
								if (!boSuccess)
								{
									x_printf ("__LengthOfFile failed on file %s\n", x_va_list(pxInfo->m_szFilename));
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
										pxInfo->m_uSectorSize		= 0;		///< uSectorSize
										pxInfo->m_uNumSectors		= uNumSectors;
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
							pAsync->m_nStatus	= FILE_OP_STATUS_READING;

							u64 nReadSize;
							bool boRead = __ReadFile(pxInfo, pAsync->m_pReadAddress, (u32)pAsync->m_uReadWriteSize, pAsync->m_uReadWriteOffset, nReadSize);
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

							pAsync->m_nStatus	= FILE_OP_STATUS_DONE;
						}
					}
					FreeAsyncIOAddToTail(pAsync);
				}
				else
				{
					OSSleepMilliseconds(33);
				}
			}
		}

		//---------------
		// Public Methods
		//---------------

		void				Initialise ( u32 uAsyncQueueSize, xbool boEnableCache )
		{
			InitialiseCommon(uAsyncQueueSize, boEnableCache);

			if (!OSCreateThread(&m_AsyncIOThread, AsyncIOWorkerThread, (void*)NULL, (void*)(m_pAsyncIOThreadStack + FS_ASYNC_WORKER_THREAD_STACK_SIZE), FS_ASYNC_WORKER_THREAD_STACK_SIZE, (OSPriority)6, 0))
			{
				x_printf ("Stdio:"TARGET_PLATFORM_STR" ERROR OSCreateThread failed\n");
			}
			OSSetThreadPriority (&m_AsyncIOThread, 6);
		}	

		//------------------------------------------------------------------------------------------

		void				Shutdown ( void )
		{
			OSDetachThread(&m_AsyncIOThread);
			OSCancelThread(&m_AsyncIOThread);

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
			FileInfo* pInfo = &m_OpenAsyncFile[uHandle];

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
			u32	uBlockSize  = 0;
			u64	uBlockCount = 0;
			//cellFsGetFreeSize(szPath, &uBlockSize, &uBlockCount);

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

			void __ParseDirDvd(xstring_buffer& szDir, bool boRecursive, u32& ruFileList, char** pszFileList)
			{
				//@TODO: This could easily kill the stack on the WII, we only have 64 Kilobyte

				// Start the directory read
				DVDDir dvdDir;
				DVDOpenDir(szDir.c_str(), &dvdDir);

				while (xTRUE)
				{
					DVDDirEntry dirent;
					bool uRead = DVDReadDir(&dvdDir, &dirent);

					if (uRead)
					{
						if (dirent.isDir)
						{
							if(	(boRecursive == true) && (x_stricmp(dirent.name, ".") != 0) && (x_stricmp(dirent.name, "..") != 0) )
							{
								const s32 start = szDir.getLength();
								const s32 len = x_strlen(dirent.name);
								szDir += dirent.name;
								ParseDir(szDir, boRecursive, ruFileList, pszFileList);
								szDir.remove(start, len);
							}
						}
						else
						{
							if (pszFileList)
							{
								// Allocate enough memory for the new entry, and copy it over
								// pszFileList[ruFileList]	= (char*)HeapManager::GetHeap()->AllocFromEnd(x_strlen(szDir) + xDirInfo.d_namlen + 1);
								s32 maxLen = x_strlen(szDir) + x_strlen(dirent.name) + 1;
								pszFileList[ruFileList]	= (char*)x_malloc(sizeof(xbyte), maxLen, XMEM_FLAG_ALIGN_8B);
								x_strcpy(pszFileList[ruFileList], maxLen, szDir);
								x_strcat(pszFileList[ruFileList], maxLen, dirent.name);
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
				DVDCloseDir(&dvdDir);
			}

			void ParseDir(const char* szDir, xbool boRecursive, u32& ruFileList, char** pszFileList)
			{
				XSTRING_BUFFER(szFullPath, FS_MAX_PATH);
				ESourceType eSource = CreateSystemPath(szDir, szFullPath);

				if (eSource == FS_SOURCE_DVD)
				{
					__ParseDirDvd(szFullPath, boRecursive, ruFileList, pszFileList);
				}
			}

			//------------------------------------------------------------------------------------------

			xbool			IsPathUNIXStyle		( void )
			{
				return true;
			}


			//------------------------------------------------------------------------------------------

			void				AsyncIOWorkerResume()
			{
				if (OSIsThreadSuspended(&m_AsyncIOThread))
					OSResumeThread(&m_AsyncIOThread);
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
				}
				return uHandle;
			}

			//------------------------------------------------------------------------------------------

			uintfs				SyncSize			( u32 uHandle )
			{
				if (uHandle==(u32)INVALID_FILE_HANDLE)
					return 0;

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
					return;

				FileInfo* pxFileInfo = GetFileInfo(uHandle);
				u64 numBytesRead;
				if (!__ReadFile(pxFileInfo, pBuffer, uSize, uOffset, numBytesRead))
				{
					x_printf ("__ReadFile failed on file %s\n", x_va_list(pxFileInfo->m_szFilename));
				}
			}

			//------------------------------------------------------------------------------------------

			void				SyncWrite			( u32 uHandle, uintfs uOffset, uintfs uSize, const void* pBuffer, xbool boRetry)
			{
				if (uHandle==(u32)INVALID_FILE_HANDLE)
					return;

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
					return;

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
					return;

				FileInfo* pxFileInfo = GetFileInfo(uHandle);
				if (!__CloseFile(pxFileInfo))
				{
					x_printf ("__CloseFile failed on file %s\n", x_va_list(pxFileInfo->m_szFilename));
				}
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

#endif // TARGET_WII