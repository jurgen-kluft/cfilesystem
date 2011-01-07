#include "../x_target.h"
#ifdef TARGET_PSP

//==============================================================================
// INCLUDES
//==============================================================================

#include <kernel.h>
#include <kerror.h>
#include <stdio.h>
#include <psptypes.h>
#include <psperror.h>
#include <iofilemgr.h>
#include <mediaman.h>
#include <umddevctl.h>
#include <kernelutils.h>
#include <utility/utility_module.h>
#include <np/np_drm.h>

#include "../x_debug.h"
#include "../x_system.h"
#include "../x_thread.h"
#include "../x_container.h"
#include "../x_string.h"
#include "../x_time.h"
#include "../x_llist.h"

#include "x_filesystem_common.h"
#include "x_filesystem_psp.h"

//==============================================================================
// xCore namespace
//==============================================================================
namespace xcore
{
	//------------------------------------------------------------------------------------------
	//---------------------------------- PSP IO Functions ------------------------------------
	//------------------------------------------------------------------------------------------
	namespace xfilesystem
	{

		static bool __OpenOrCreateFile(FileInfo* pInfo)
		{
			s32 flag;
			if (pInfo->m_boWriting)
			{
				flag = SCE_O_RDWR | SCE_O_TRUNC | SCE_O_CREAT;
			}
			else
			{
				flag = SCE_O_RDONLY;
			}
			if (x_stricmp(__private::GetFileExtension(pInfo->m_szFilename), "EDAT") == 0)
			{
				flag = SCE_FGAMEDATA | SCE_O_RDONLY;
			}
			
			SceMode mode = 0;
			SceUID nResult = sceIoOpen(pInfo->m_szFilename, flag, mode);
			bool boError = nResult < 0;
			if (!boError)
			{
				pInfo->m_nFileHandle = (u32)nResult;
			}
			return !boError;
		}

		static bool __LengthOfFile(FileInfo* pInfo, u64& outLength)
		{
			SceIoStat stats;
			s32 nResult = sceIoGetstat(pInfo->m_szFilename, &stats);
			bool boSuccess = (nResult == SCE_KERNEL_ERROR_OK);
			if (boSuccess)
				outLength = stats.st_size;
			else
				outLength = -1;

			return boSuccess;
		}

		static bool __CloseFile(FileInfo* pInfo)
		{
			s32 nResult = sceIoClose(pInfo->m_nFileHandle);
			return nResult==SCE_KERNEL_ERROR_OK;
		}

		static bool __DeleteFile(FileInfo* pInfo)
		{
			s32 nResult = sceIoRemove(pInfo->m_szFilename);
			return nResult==SCE_KERNEL_ERROR_OK;
		}

		static bool __ReadFile(FileInfo* pInfo, void* buffer, s32 count, u64& outNumBytesRead)
		{
			SceSSize numBytesWritten = sceIoRead(pInfo->m_nFileHandle, buffer, count);
			bool boSuccess = numBytesWritten>=0;
			if (boSuccess)
			{
				outNumBytesRead = numBytesWritten;
			}
			return boSuccess;
		}
		static bool __WriteFile(FileInfo* pInfo, const void* buffer, s32 count, u64& outNumBytesWritten)
		{
			SceSSize numBytesWritten = sceIoWrite(pInfo->m_nFileHandle, buffer, count);
			bool boSuccess = numBytesWritten>=0;
			if (boSuccess)
			{
				outNumBytesWritten = numBytesWritten;
			}
			return boSuccess;
		}

		enum EPspSeekMode
		{
			__SEEK_ORIGIN = SCE_SEEK_SET,
			__SEEK_CURRENT = SCE_SEEK_CUR,
			__SEEK_END = SCE_SEEK_END,
		};

		static bool __Seek(FileInfo* pInfo, EPspSeekMode mode, u64 pos, u64& newPos)
		{
			SceOff offset = pos;
			SceOff newOffset = sceIoLseek(pInfo->m_nFileHandle, offset, mode);
			bool boSuccess = newOffset>=0;
			if (boSuccess)
			{
				newPos = newOffset;
			}
			return boSuccess;
		}
	};

	namespace xfilesystem
	{
		static char	m_pszMemoryCardPath[FS_MAX_PATH];
	};



	namespace xfilesystem
	{
		using namespace __private;

		//--------
		// Defines
		//--------
		#define FS_ASYNC_WORKER_THREAD_STACK_SIZE	(0x4000)					// 16KB
		#define FS_ASYNC_WORKER_THREAD_EXIT_CODE	(0xbee)

		#define FS_ASYNC_WORKER_THREAD_PRIO_LOWEST	(SCE_KERNEL_USER_LOWEST_PRIORITY)
		#define FS_ASYNC_WORKER_THREAD_PRIO_HIGHEST	(SCE_KERNEL_USER_HIGHEST_PRIORITY)

		#define FS_ASYNC_WORKER_THREAD_PRIO			(FS_ASYNC_WORKER_THREAD_PRIO_HIGHEST + 2)

		//---------
		// Forward declares
		//---------

		static SceUID					m_AsyncIOThreadHandle;
		static s32						m_AsyncIOThreadID;

		enum EWorkerThreadState { FS_WORKER_THREAD_RUN, FS_WORKER_THREAD_EXITING, FS_WORKER_THREAD_ENDED };
		static volatile s32				m_AsyncIOThreadState;

		static u32						m_uFileListLength = 0;
		static char**					m_pszFileListData = NULL;

		//----------------
		// Private Methods
		//----------------

		s32 AsyncIOWorkerThread(SceSize argSize, void *argBlock)
		{
			while (m_AsyncIOThreadState == FS_WORKER_THREAD_RUN)
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

							pAsync->m_nStatus	= FILE_OP_STATUS_DONE;
						}
					}
					FreeAsyncIOAddToTail(pAsync);
				}
				else
				{
					sceKernelDelayThread((SceUInt)33);
				}
			}

			m_AsyncIOThreadState = FS_WORKER_THREAD_ENDED;
			return 0;
		}

		//---------------
		// Public Methods
		//---------------

		void				Initialise ( u32 uAsyncQueueSize, xbool boEnableCache )
		{
			InitialiseCommon(uAsyncQueueSize, boEnableCache);

			m_AsyncIOThreadState = FS_WORKER_THREAD_RUN;
			s32 ret = m_AsyncIOThreadHandle = sceKernelCreateThread("AsyncIOWorkerThread", AsyncIOWorkerThread, FS_ASYNC_WORKER_THREAD_PRIO, (SceSize)FS_ASYNC_WORKER_THREAD_STACK_SIZE, SCE_KERNEL_TH_CLEAR_STACK, NULL );
			if (ret < 0)
			{
				x_printf ("Stdio:"TARGET_PLATFORM_STR" ERROR sceKernelCreateThread failed\n");
			}

			ret = sceKernelStartThread(m_AsyncIOThreadHandle, 0, NULL);
			if (ret < 0)
			{
				x_printf ("Stdio:"TARGET_PLATFORM_STR" ERROR sceKernelStartThread failed\n");
			}

			ret = sceKernelChangeThreadPriority(m_AsyncIOThreadHandle, FS_ASYNC_WORKER_THREAD_PRIO);
			if (ret < 0)
			{
				x_printf ("Stdio:"TARGET_PLATFORM_STR" ERROR sceKernelChangeThreadPriority failed\n");
			}
		}	

		//------------------------------------------------------------------------------------------

		void				Shutdown ( void )
		{
			m_AsyncIOThreadState = FS_WORKER_THREAD_EXITING;
			AsyncIOWorkerResume();
			while (m_AsyncIOThreadState != FS_WORKER_THREAD_ENDED){}
			s32 ret = sceKernelDeleteThread(m_AsyncIOThreadHandle);
			if (ret != SCE_KERNEL_ERROR_OK)
			{
				x_printf("Stdio:"TARGET_PLATFORM_STR" ERROR sceKernelDeleteThread %d \n", x_va_list(ret));
			}

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
			FileInfo* pInfo = GetFileInfo(uHandle);

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
				return true;
			}

			//------------------------------------------------------------------------------------------

			void				AsyncIOWorkerResume()
			{
				s32 ret = sceKernelWakeupThread(m_AsyncIOThreadHandle);
				if (ret != SCE_KERNEL_ERROR_OK)
				{
					x_printf("Stdio:"TARGET_PLATFORM_STR" ERROR resume thread %d\n", x_va_list(ret));
					ASSERT(xFALSE);
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
		};
	};


	namespace xfilesystem
	{
		//------------------------------------------------------------------------------------------

		void			SetDRMLicenseKey( const u8* p8Key )
		{
// 			s32 nError = sceNpDrmSetLicenseeKey ((const SceNpDrmKey*)p8Key);
// 
// 			if(nError < 0)
// 			{
// 				x_printf ("sceNpDrmSetLicenseeKey Error 0x%lx\n", x_va_list(nError));
// 			}
		}

		//------------------------------------------------------------------------------------------

		void			SetMemoryCardPath	( const char* szMemoryCardPath )
		{
			x_strcpy(m_pszMemoryCardPath, FS_MAX_PATH, szMemoryCardPath);

			// Register memory stick alias
			xalias ms("ms", FS_SOURCE_MS, m_pszMemoryCardPath);
			AddAlias(ms);
		}

		//------------------------------------------------------------------------------------------

		s32				SaveToMemoryStick(const char* szPhotosDir, const char* szDirectory, char* szFilename, void* pData, s32 nSizeBytes )
		{
			SceUID uid;

			s32 retval = sceIoMkdir( szPhotosDir, 0 );
			if ( retval != SCE_KERNEL_ERROR_OK )
			{
				//x_printf("sceIoMkdir szPhotosDir fail\n");
			}

			retval = sceIoMkdir( szDirectory, 0 );
			if ( retval != SCE_KERNEL_ERROR_OK )
			{
				//x_printf("sceIoMkdir szDirectory fail\n");
			}

			uid = sceIoOpen( szFilename, SCE_O_RDWR | SCE_O_TRUNC | SCE_O_CREAT, 0777);

			if(uid < 0)
			{
				x_printf("--sample,Open fail!!\n");
				return uid;
			}
			else
			{
				x_printf("Opened FD: %d\n", x_va_list(uid));

				s32 nResult = sceIoWrite(uid, pData, nSizeBytes);

				if( 0 > nResult )
				{
					x_printf("Write fail\n");
					sceIoClose(uid);
					return nResult;
				}

				nResult = sceIoClose(uid);
				return nResult;
			}
		}

	};

	//==============================================================================
	// END xCore namespace
	//==============================================================================
};

#endif // TARGET_PSP