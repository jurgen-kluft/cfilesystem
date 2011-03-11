//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase\x_target.h"
#include "xbase\x_debug.h"
#include "xbase\x_memory_std.h"
#include "xbase\x_string_std.h"
#include "xbase\x_va_list.h"

#include "xfilesystem\private\x_filesystem_common.h"
#include "xfilesystem\private\x_filesystem_spsc_queue.h"
#include "xfilesystem\private\x_filecache.h"
#include "xfilesystem\private\x_fileinfo.h"

#include "xfilesystem\x_filesystem.h"
#include "xfilesystem\x_alias.h"

//==============================================================================
// xcore namespace
//==============================================================================
namespace xcore
{
	//------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------
	//----------------------- xfilecache Implementation ----------------------------------------
	//------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------
	namespace xfilesystem
	{
		//---------------------
		// statics
		//---------------------
		xfilecache::CallbackData	xfilecache::m_xCallbacks[MAX_CALLBACKS];


		void xfilecache::FileIOCallback( u32 nHandle, s32 nID )
		{
			if(m_xCallbacks[nID].m_Callback == NULL)
			{
				m_xCallbacks[nID].m_Callback(m_xCallbacks[nID].m_pClass, m_xCallbacks[nID].m_nID);
				m_xCallbacks[nID].m_Callback = NULL;
			}
		}

		xfilecache::xfilecache() 
			: m_nCacheHandle((u32)INVALID_FILE_HANDLE)
			, m_uCacheSize(0)
		{
			x_memset(&m_xHeader, 0, sizeof(CacheHeader));
			x_memset(m_xCallbacks, 0, sizeof(CallbackData) * MAX_CALLBACKS);
		}

		void	xfilecache::initialise() 
		{
			if (xfilesystem::doesFileExist(CACHE_FILENAME))
			{
				m_nCacheHandle	= xfilesystem::open(CACHE_FILENAME, true);
				xfilesystem::read(m_nCacheHandle, 0, sizeof(CacheHeader), &m_xHeader, false);

				for(s32 nFile = 0; nFile < MAX_CACHED_FILES; nFile++)
				{
					if(m_xHeader.m_xCacheList[nFile].m_uFlags & CACHE_FILE_FLAGS_BUSY)
					{
						if(xfilesystem::doesFileExist(m_xHeader.m_xCacheList[nFile].m_szName))
						{
							u32 h = xfilesystem::open(m_xHeader.m_xCacheList[nFile].m_szName, false);
							xfilesystem::closeAndDelete(h);
						}
					}
					else if(m_xHeader.m_xCacheList[nFile].m_uFlags & CACHE_FILE_FLAGS_VALID)
					{
						if(m_xHeader.m_xCacheList[nFile].m_uFlags & CACHE_FILE_FLAGS_PERMANENT)
						{
							FileEntry* pEntry = m_xPermanentList.getHead();

							while(pEntry)
							{
								if(m_xHeader.m_xCacheList[nFile].m_uOffset < pEntry->m_uOffset)
								{
									m_xPermanentList.addBefore(&m_xHeader.m_xCacheList[nFile], pEntry);
									break;
								}

								pEntry	= pEntry->getNext();
							}

							if(pEntry == NULL)
							{
								m_xPermanentList.addToTail(&m_xHeader.m_xCacheList[nFile]);
							}
						}
						else
						{
							FileEntry* pEntry = m_xTransientList.getTail();

							while(pEntry)
							{
								if(m_xHeader.m_xCacheList[nFile].m_uOffset > pEntry->m_uOffset)
								{
									m_xTransientList.addAfter(&m_xHeader.m_xCacheList[nFile], pEntry);
									break;
								}

								pEntry	= pEntry->getNext();
							}

							if(pEntry == NULL)
							{
								m_xTransientList.addToHead(&m_xHeader.m_xCacheList[nFile]);
							}
						}
					}
				}
			}
			else
			{
				m_nCacheHandle = xfilesystem::open(CACHE_FILENAME, true);
				xfilesystem::write(m_nCacheHandle, 0, sizeof(CacheHeader), &m_xHeader, false);
			}

			m_uCacheSize = xfilesystem::getFreeSize(CACHE_PATH);
		}

		//------------------------------------------------------------------------------------------

		xfilecache::~xfilecache()
		{
			if (m_nCacheHandle != (u32)INVALID_FILE_HANDLE)
				xfilesystem::close(m_nCacheHandle);
		}

		//------------------------------------------------------------------------------------------

		void xfilecache::PurgeCache()
		{
			for(s32 nFile = 0; nFile < MAX_CACHED_FILES; nFile++)
			{
				m_xHeader.m_xCacheList[nFile].m_uFlags = CACHE_FILE_FLAGS_INVALID;
			}

			xfilesystem::createFileList("cache:\\", true);
			for(s32 nFile = 0; nFile < xfilesystem::getFileListLength(); nFile++)
			{
				const char* szFile = xfilesystem::getFileListData(nFile);
				u32 h = xfilesystem::open(szFile, false);
				xfilesystem::closeAndDelete(h);
			}
			xfilesystem::destroyFileList();
		}

		//------------------------------------------------------------------------------------------

		void xfilecache::InvalidateCacheIndexFile()
		{
			xfilesystem::xfileinfo* xInfo = getFileInfo(m_nCacheHandle);
			u32 h = xfilesystem::open(xInfo->m_szFilename, false);
			xfilesystem::closeAndDelete(h);
		}

		//------------------------------------------------------------------------------------------

		void xfilecache::InvalidateCacheIndexFileAsync()
		{
			xfilesystem::asyncQueueDelete(m_nCacheHandle, 0, NULL, 0);
		}

		//------------------------------------------------------------------------------------------

		void xfilecache::WriteCacheIndexFile()
		{
			for(s32 nFile = 0; nFile < MAX_CACHED_FILES; nFile++)
			{
				if(m_xHeader.m_xCacheList[nFile].m_uFlags & CACHE_FILE_FLAGS_BUSY)
				{
					m_xHeader.m_xCacheList[nFile].m_uFlags &= ~CACHE_FILE_FLAGS_BUSY;
					m_xHeader.m_xCacheList[nFile].m_uFlags |= CACHE_FILE_FLAGS_VALID;
				}
			}

			xfilesystem::write(m_nCacheHandle, 0, sizeof(CacheHeader), &m_xHeader, false);
		}

		//------------------------------------------------------------------------------------------

		void xfilecache::WriteCacheIndexFileAsync()
		{
			for(s32 nFile = 0; nFile < MAX_CACHED_FILES; nFile++)
			{
				if(m_xHeader.m_xCacheList[nFile].m_uFlags & CACHE_FILE_FLAGS_BUSY)
				{
					m_xHeader.m_xCacheList[nFile].m_uFlags &= ~CACHE_FILE_FLAGS_BUSY;
					m_xHeader.m_xCacheList[nFile].m_uFlags |= CACHE_FILE_FLAGS_VALID;
				}
			}

			xfilesystem::asyncQueueWrite(m_nCacheHandle, xfilesystem::FS_PRIORITY_HIGH, 0, sizeof(CacheHeader), &m_xHeader, NULL, 0);
		}

		//------------------------------------------------------------------------------------------

		s32 xfilecache::GetCachedIndex( const char* szFilename, u64 uOffset )
		{
			for(s32 nFile = 0; nFile < MAX_CACHED_FILES; nFile++)
			{
				if(m_xHeader.m_xCacheList[nFile].m_uFlags & CACHE_FILE_FLAGS_VALID)
				{
					if(x_strnicmp(m_xHeader.m_xCacheList[nFile].m_szName, szFilename, x_strlen(szFilename)) == 0)
					{
						if(m_xHeader.m_xCacheList[nFile].m_uOffset == uOffset)
						{
							return nFile;
						}
					}
				}
			}

			return -1;
		}

		//------------------------------------------------------------------------------------------

		void xfilecache::GetCacheData( s32 nIndex, FileEntry& rxFileEntry )
		{
			if(nIndex >= 0 && nIndex < MAX_CACHED_FILES)
			{
				rxFileEntry	= m_xHeader.m_xCacheList[nIndex];
			}
			else
			{
				ASSERTS(0, "Index out of range\n");
			}
		}

		//------------------------------------------------------------------------------------------

		bool xfilecache::AddToCache( const char* szFilename, void* pData, const u64 uOffset, const u64 uSize, const bool boPermanent )
		{
			for(s32 nFile = 0; nFile < MAX_CACHED_FILES; nFile++)
			{
				if(m_xHeader.m_xCacheList[nFile].m_uFlags == CACHE_FILE_FLAGS_INVALID)
				{
					char szPath[FS_MAX_PATH];
					szPath[0] = '\0';
					x_strcpy(szPath, FS_MAX_PATH, szFilename);

					if (uOffset != 0)
					{
						char szTempOffset[xfilesystem::FS_MAX_PATH];
						x_sprintf(szTempOffset, FS_MAX_PATH, "@%x", x_va((s32)uOffset));
						x_strcat(szPath, FS_MAX_PATH, szTempOffset);
					}

					char szCachePath[FS_MAX_PATH];

					xfilesystem::replaceAliasOfFilename(szPath, FS_MAX_PATH, xfilesystem::findAliasFromFilename(szPath)->alias());

					u32	nWriteHandle = xfilesystem::open(szCachePath, true);
					xfilesystem::write(nWriteHandle, 0, uSize, pData, false);
					xfilesystem::close(nWriteHandle);

					x_strcpy(m_xHeader.m_xCacheList[nFile].m_szName, CACHED_FILE_NAME_LENGTH, szPath);
					m_xHeader.m_xCacheList[nFile].m_uOffset	= uOffset;
					m_xHeader.m_xCacheList[nFile].m_uSize	= uSize;
					m_xHeader.m_xCacheList[nFile].m_uFlags	= boPermanent ? CACHE_FILE_FLAGS_PERMANENT | CACHE_FILE_FLAGS_BUSY : CACHE_FILE_FLAGS_BUSY;
					m_xHeader.m_xCacheList[nFile].m_uCRC	= 0;

					if(boPermanent)
					{
						m_xPermanentList.addToTail(&m_xHeader.m_xCacheList[nFile]);
					}
					else
					{
						m_xTransientList.addToHead(&m_xHeader.m_xCacheList[nFile]);
					}

					return true;
				}
			}

			return false;
		}

		//------------------------------------------------------------------------------------------

		bool xfilecache::AddToCacheAsync( const char* szFilename, void* pData, const u64 uOffset, const u64 uSize, const bool boPermanent, CacheCallBack Callback, void* pClass, s32 nID )
		{
			for(s32 nFile = 0; nFile < MAX_CACHED_FILES; nFile++)
			{
				if(m_xHeader.m_xCacheList[nFile].m_uFlags == CACHE_FILE_FLAGS_INVALID)
				{
					s32	nCallback	= -1;

					for(s32 nCB = 0; nCB < MAX_CALLBACKS; nCB++)
					{
						if(m_xCallbacks[nCB].m_Callback == NULL)
						{
							m_xCallbacks[nCB].m_Callback	= Callback;
							m_xCallbacks[nCB].m_pClass		= pClass;
							m_xCallbacks[nCB].m_nID			= nID;

							nCallback	= nCB;
							break;
						}
					}

					if(nCallback == -1)
					{
						return false;
					}

					char szPath[FS_MAX_PATH];
					szPath[0] = '\0';
					if(uOffset != 0)
					{
						x_sprintf(szPath, FS_MAX_PATH-1, "%s@%x", x_va(szFilename), x_va((s32)uOffset));
					}
					else
					{
						x_strcpy(szPath, FS_MAX_PATH, szFilename);
					}
					const xalias* alias = findAlias("cache");
					replaceAliasOfFilename(szPath, FS_MAX_PATH, alias!=NULL ? alias->alias() : NULL);

					u32 nWriteHandle = xfilesystem::asyncPreOpen(szPath, xTRUE);
					xfilesystem::asyncQueueOpen(nWriteHandle, xfilesystem::FS_PRIORITY_HIGH, NULL, 0);
					xfilesystem::asyncQueueWrite(nWriteHandle, xfilesystem::FS_PRIORITY_HIGH, 0, uSize, pData, FileIOCallback, nCallback);
					xfilesystem::asyncQueueClose(nWriteHandle, xfilesystem::FS_PRIORITY_HIGH, NULL, 0);

					x_strcpy(m_xHeader.m_xCacheList[nFile].m_szName, CACHED_FILE_NAME_LENGTH, szFilename);
					m_xHeader.m_xCacheList[nFile].m_uOffset	= uOffset;
					m_xHeader.m_xCacheList[nFile].m_uSize	= uSize;
					m_xHeader.m_xCacheList[nFile].m_uFlags	= boPermanent ? CACHE_FILE_FLAGS_PERMANENT | CACHE_FILE_FLAGS_BUSY : CACHE_FILE_FLAGS_BUSY;
					m_xHeader.m_xCacheList[nFile].m_uCRC	= 0;

					if(boPermanent)
					{
						m_xPermanentList.addToTail(&m_xHeader.m_xCacheList[nFile]);
					}
					else
					{
						m_xTransientList.addToHead(&m_xHeader.m_xCacheList[nFile]);
					}

					return true;
				}
			}

			return false;
		}

		//------------------------------------------------------------------------------------------

		bool xfilecache::SetCacheData( const char* szFilename, void* pData, const u64 uOffset, const u64 uSize, const bool boPermanent )
		{
			for(s32 nFile = 0; nFile < MAX_CACHED_FILES; nFile++)
			{
				if(m_xHeader.m_xCacheList[nFile].m_uFlags == CACHE_FILE_FLAGS_INVALID)
				{
					x_strcpy(m_xHeader.m_xCacheList[nFile].m_szName, CACHED_FILE_NAME_LENGTH, szFilename);
					m_xHeader.m_xCacheList[nFile].m_uOffset	= uOffset;
					m_xHeader.m_xCacheList[nFile].m_uSize	= uSize;
					m_xHeader.m_xCacheList[nFile].m_uFlags	= boPermanent ? CACHE_FILE_FLAGS_PERMANENT | CACHE_FILE_FLAGS_BUSY : CACHE_FILE_FLAGS_BUSY;
					m_xHeader.m_xCacheList[nFile].m_uCRC	= 0;

					if(boPermanent)
					{
						m_xPermanentList.addToTail(&m_xHeader.m_xCacheList[nFile]);
					}
					else
					{
						m_xTransientList.addToHead(&m_xHeader.m_xCacheList[nFile]);
					}

					return true;
				}
			}

			return false;
		}

		//------------------------------------------------------------------------------------------

		void xfilecache::RemoveFromCache( const char* szFilename )
		{
			s32	nIndex	= GetCachedIndex(szFilename);

			if(nIndex >= 0)
			{
				if(m_xHeader.m_xCacheList[nIndex].m_uFlags & CACHE_FILE_FLAGS_PERMANENT)
				{
				}

				char cacheFilename[FS_MAX_PATH];
				cacheFilename[0] = '\0';
				x_strcpy(cacheFilename, FS_MAX_PATH, m_xHeader.m_xCacheList[nIndex].m_szName);
				const xalias* alias = xfilesystem::findAlias("cache");
				xfilesystem::replaceAliasOfFilename(cacheFilename, FS_MAX_PATH, alias!=NULL ? alias->alias() : NULL);
				u32 h = xfilesystem::open(cacheFilename);
				xfilesystem::closeAndDelete(h);
				m_xHeader.m_xCacheList[nIndex].m_uFlags = CACHE_FILE_FLAGS_INVALID;
			}
		}

	};

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};
