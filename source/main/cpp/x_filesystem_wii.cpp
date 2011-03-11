#include "xbase\x_target.h"
#ifdef TARGET_WII

#include <revolution.h>
#include <revolution\hio2.h>
#include <revolution\nand.h>
#include <revolution\dvd.h>

//==============================================================================
// INCLUDES
//==============================================================================

#include "xbase\x_types.h"
#include "xbase\x_debug.h"
#include "xbase\x_string_std.h"

#include "xfilesystem\private\x_filesystem_common.h"
#include "xfilesystem\private\x_filesystem_wii.h"

//==============================================================================
// xcore namespace
//==============================================================================
namespace xcore
{
	namespace xfilesystem
	{
		//------------------------------------------------------------------------------------------

		//---------------
		// Public Methods
		//---------------

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
			xfileinfo* pInfo = &m_OpenAsyncFile[uHandle];

			s32 nResult=-1;
// 			nResult	= cellFsFtruncate(pInfo->m_nFileHandle, uNewSize);
			if(nResult < 0)
			{
				x_printf("xfilesystem:"TARGET_PLATFORM_STR" ERROR ReSize %d\n", x_va_list(nResult));
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
		void				__ParseDirDvd(xstring_buffer& szDir, bool boRecursive, u32& ruFileList, char** pszFileList)
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
							parseDir(szDir, boRecursive, ruFileList, pszFileList);
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
							parseDir(szDir, boRecursive, ruFileList, pszFileList);
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

		void parseDir(const char* szDir, xbool boRecursive, u32& ruFileList, char** pszFileList)
		{
			XSTRING_BUFFER(szFullPath, FS_MAX_PATH);
			ESourceType eSource = CreateSystemPath(szDir, szFullPath);

			if (eSource == FS_SOURCE_DVD)
			{
				__ParseDirDvd(szFullPath, boRecursive, ruFileList, pszFileList);
			}
		}

		//------------------------------------------------------------------------------------------

		xbool			isPathUNIXStyle		( void )
		{
			return true;
		}

	};


	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

#endif // TARGET_WII