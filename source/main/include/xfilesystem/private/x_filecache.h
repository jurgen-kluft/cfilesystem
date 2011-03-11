#ifndef __X_FILESYSTEM_CACHE_H__
#define __X_FILESYSTEM_CACHE_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase\x_types.h"
#include "xfilesystem\private\x_filesystem_spsc_queue.h"
#include "xfilesystem\private\x_filesystem_llist.h"

//==============================================================================
// xcore namespace
//==============================================================================
namespace xcore
{
	//==============================================================================
	// xfilecache
	//==============================================================================
	namespace xfilesystem
	{
		//--------
		// Defines
		//--------
		enum ECacheSettings
		{
			CACHED_FILE_NAME_LENGTH		= 256,
			MAX_CALLBACKS				= 16,
			MAX_CACHED_FILES			= 1024,
			CACHE_FILE_ALIGNMENT		= 2048
		};

		#define	CACHE_PATH					"cache:\\"
		#define	CACHE_FILENAME				CACHE_PATH "DataCache.dat"

		//---------
		// Typedefs
		//---------
		typedef void (*CacheCallBack) (void* pClass, s32 nID);

		//------
		// Enums
		//------
		enum ECacheFileFlags
		{
			CACHE_FILE_FLAGS_INVALID	= 0x00,		// Data not set
			CACHE_FILE_FLAGS_VALID		= 0x01,		// Valid cached data
			CACHE_FILE_FLAGS_PERMANENT	= 0x02,		// Must not be automatically de-cached
			CACHE_FILE_FLAGS_BUSY		= 0x04,		// Operation in progress
		};

		//-----------
		// Data Types
		//-----------

		class xfilecache
		{
			class FileEntry
			{
			public:
				FileEntry*		m_pPrev;
				FileEntry*		m_pNext;

				char			m_szName[CACHED_FILE_NAME_LENGTH];
				u64				m_uOffset;
				u64				m_uSize;

				u32				m_uFlags;
				u32				m_uCRC;

				FileEntry*		getPrev	()											{ return m_pPrev; }
				FileEntry*		getNext	()											{ return m_pNext; }

				void			setPrev	( FileEntry* pPrev )						{ m_pPrev = pPrev; }
				void			setNext	( FileEntry* pNext )						{ m_pNext = pNext; }
			};

			class CallbackData
			{
			public:
				CacheCallBack	m_Callback;
				void*			m_pClass;
				s32				m_nID;
			};

			class CacheHeader
			{
			public:
				u32				m_uFlags;
				FileEntry		m_xCacheList[MAX_CACHED_FILES];
			};

			u32								m_nCacheHandle;
			u64								m_uCacheSize;
			CacheHeader						m_xHeader;
			xfilesystem::llist<FileEntry>	m_xPermanentList;				///< Single thread access
			xfilesystem::llist<FileEntry>	m_xTransientList;				///< Single thread access

			static	CallbackData			m_xCallbacks[MAX_CALLBACKS];

			static void	FileIOCallback		( u32 nHandle, s32 nID );

		public:
						xfilecache						();
						~xfilecache						();

			void*		operator new (size_t size, void *p)					{ return p; }
			void		operator delete(void* mem, void* )					{ }	

			void		initialise						();
			void		PurgeCache						();

			s32			GetCachedIndex					( const char* szFilename, u64 uOffset = 0 );
			void		GetCacheData					( s32 nIndex, FileEntry& rxFileEntry );

			bool		AddToCache						( const char* szFilename, void* pData, const u64 uOffset, const u64 uSize, const bool boPermanent );
			bool		AddToCacheAsync					( const char* szFilename, void* pData, const u64 uOffset, const u64 uSize, const bool boPermanent, CacheCallBack Callback, void* pClass, s32 nID );
			bool		SetCacheData					( const char* szFilename, void* pData, const u64 uOffset, const u64 uSize, const bool boPermanent );

			void		RemoveFromCache					( const char* szFilename );

			void		InvalidateCacheIndexFile		();
			void		InvalidateCacheIndexFileAsync	();

			void		WriteCacheIndexFile				();
			void		WriteCacheIndexFileAsync		();
		};
	};

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

//==============================================================================
// END __X_FILESYSTEM_CACHE_H__
//==============================================================================
#endif