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
	namespace xfilesystem
	{
		//---------
		// Forward declares
		//---------

		static u32						m_uFileListLength = 0;
		static char**					m_pszFileListData = NULL;

		//---------------
		// Public Methods
		//---------------

		//------------------------------------------------------------------------------------------

		void				GetOpenCreatedTime (u32 uHandle, xdatetime& pTimeAndDate)
		{
			ASSERTS (pTimeAndDate, "GetOpenCreatedTime() : Pointer to xsystem::TimeAndDate is NULL!");

			CellFsStat	xStat;
			xfileinfo* pxFileInfo = getFileInfo(uHandle);
			CellFsErrno eError = cellFsFstat(pxFileInfo->m_nFileHandle, &xStat);

			pTimeAndDate = xdatetime::sFromFileTime(xStat.st_ctime);
		}

		//------------------------------------------------------------------------------------------

		void				GetOpenModifiedTime (u32 uHandle, xdatetime& pTimeAndDate)
		{
			ASSERTS (pTimeAndDate, "GetOpenModifiedTime() : Pointer to xsystem::TimeAndDate is NULL!");

			CellFsStat	xStat;
			xfileinfo* pxFileInfo = getFileInfo(uHandle);
			CellFsErrno eError = cellFsFstat(pxFileInfo->m_nFileHandle, &xStat);

			pTimeAndDate = xdatetime::sFromFileTime(xStat.st_ctime);
		}

		//------------------------------------------------------------------------------------------

		void				ReSize( u32 uHandle, u64 uNewSize )
		{
			xfileinfo* pxFileInfo = getFileInfo(uHandle);

			s32 nResult	= cellFsFtruncate(pxFileInfo->m_nFileHandle, uNewSize);
			if(nResult != CELL_OK)
			{
				x_printf("xfilesystem:"TARGET_PLATFORM_STR" ERROR cellFsFtruncate %d\n", x_va_list(nResult));
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

		void				createFileList( const char* szPath, xbool boRecursive )
		{
			ASSERTS(m_uFileListLength == 0, "createFileList Already exists");
			ASSERTS(m_pszFileListData == NULL, "createFileList Already exists - memory leak occurring!\n");

			m_uFileListLength	= 0;
			m_pszFileListData	= NULL;

			// Parse the dir twice - once to see memory usage, 2nd to get data
			parseDir(szPath, boRecursive, m_uFileListLength, m_pszFileListData);

			if(m_uFileListLength > 0)
			{
				// Allocate and fill info
				// m_pszFileListData	= (char**)HeapManager::GetHeap()->AllocFromEnd(m_uFileListLength * sizeof(char*));
				m_pszFileListData = (char**)x_malloc(sizeof(char*), m_uFileListLength, XMEM_FLAG_ALIGN_8B);

				u32	uIndex	= 0;
				parseDir(szPath, boRecursive, uIndex, m_pszFileListData);
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

		s32				getFileListLength	( void )
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

		//------------------------------------------------------------------------------------------

		void				initialise ( u32 uAsyncQueueSize, xbool boEnableCache )
		{
			initialiseCommon(uAsyncQueueSize, boEnableCache);
		}	

		//------------------------------------------------------------------------------------------

		void				shutdown ( void )
		{
			shutdownCommon();
		}

		//------------------------------------------------------------------------------------------

		void				parseDir(const char* szDir, xbool boRecursive, u32& ruFileList, char** pszFileList)
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

							parseDir(szPath.c_str(), boRecursive, ruFileList, pszFileList);
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

		xbool				isPathUNIXStyle		( void )
		{
			return true;
		}


	};


	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

#endif // TARGET_PS3