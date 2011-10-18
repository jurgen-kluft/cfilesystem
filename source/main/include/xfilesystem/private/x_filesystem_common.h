#ifndef __X_FILESYSTEM_COMMON_H__
#define __X_FILESYSTEM_COMMON_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase\x_types.h"
#include "xbase\x_allocator.h"
#include "xbase\x_singleton.h"

#include "xfilesystem\x_filesystem.h"
#include "xfilesystem\private\x_filesystem_constants.h"
#include "xfilesystem\x_stream.h"
#include "xfilesystem\private\x_filesystem_cqueue.h"



//==============================================================================
// xcore namespace
//==============================================================================
namespace xcore
{
	class xcstring;
	class xdatetime;

	//==============================================================================
	// xfilesystem common, private
	//==============================================================================
	namespace xfilesystem
	{
		class xfiledevice;
		class xfilepath;
		struct xfiledata;
		struct xfileasync;
		class xio_thread;


		extern void				setAllocator		( x_iallocator* allocator );
		extern void*			heapAlloc			( s32 size, s32 alignment );
		extern void				heapFree			( void* mem );

		
		extern void 			initialise			( u32 uMaxOpenStreams );
		extern void				shutdown			( void );

		xbool					isPathUNIXStyle		( void );

		// xfilesystem common singleton class
		class xfs_common: public xcore::xsingleton<xfs_common, xcore::xheap_instantiation>
		{

			//==============================================================================
			// FORWARD DECLARES
			//==============================================================================

					// Init/Exit touches these


			u32							m_uMaxOpenedFiles;
			u32							m_uMaxAsyncOperations;
			u32							m_uMaxErrorItems;
			char*							*m_Filenames;
			xfiledata						*m_OpenAsyncFileArray;
			xfileasync						*m_AsyncIOData;
			//	 xiasync_result_imp			*m_AsyncResultData;

			///< Concurrent access
			cqueue<xfiledata*>				*m_FreeAsyncFile;
			cqueue<xfileasync*>			*m_pFreeAsyncIOList;
			cqueue<xfileasync*>			*m_pAsyncIOList;
			cqueue<u32>					*m_pLastErrorStack;


			s32							mInitCommonComplete;
			xio_thread*					mIoThread;

						///< Synchronous file operations
			 u32			syncCreate			( const char* szFilename, xbool boRead = true, xbool boWrite = false );
			 u32			syncOpen			( const char* szFilename, xbool boRead = true, xbool boWrite = false );
			 u64			syncSetPos			( u32 uHandle, u64 filePos );
			 u64			syncGetPos			( u32 uHandle );
			 void			syncSetSize			( u32 uHandle, u64 fileSize );
			 u64			syncGetSize			( u32 uHandle );
			 u64			syncRead			( u32 uHandle, u64 uOffset, u64 uSize, void* pBuffer );	
			 u64			syncWrite			( u32 uHandle, u64 uOffset, u64 uSize, const void* pBuffer );
			 void 			syncClose			( u32& uHandle );
			 void			syncDelete			( u32& uHandle );

			///< Asynchronous file operations
			 u32			asyncPreOpen		( const char* szFilename, xbool boRead = true, xbool boWrite = false );
			 void			asyncOpen			( const u32 uHandle, x_asyncio_callback_struct callback = x_asyncio_callback_struct() );
			 u32			asyncOpen			( const char* szFilename, xbool boRead = true, xbool boWrite = false, x_asyncio_callback_struct callback = x_asyncio_callback_struct() );
			 void			asyncRead			( const u32 uHandle, u64 uOffset, u64 uSize, void* pBuffer, x_asyncio_callback_struct callback = x_asyncio_callback_struct() );
			 void			asyncWrite			( const u32 uHandle, u64 uOffset, u64 uSize, const void* pBuffer, x_asyncio_callback_struct callback = x_asyncio_callback_struct() );
			 void			asyncClose			( const u32 uHandle, x_asyncio_callback_struct callback = x_asyncio_callback_struct() );
			 void			asyncCloseAndDelete	( const u32 uHandle, x_asyncio_callback_struct callback = x_asyncio_callback_struct() );




		public:
			//////////////////////////////////////////////////////////////////////////
			// Common xfilesystem functionality
			//////////////////////////////////////////////////////////////////////////

			static x_iallocator* singleton_allocator();
			static u32           singleton_alignment();
			XCORE_CLASS_NEW_DELETE(singleton_allocator, 4)


			xfs_common()
			{
				//sAllocator = NULL;

				m_uMaxOpenedFiles = 32;
				m_uMaxAsyncOperations = 32;
				m_uMaxErrorItems = 32;
				m_Filenames = NULL;
				m_OpenAsyncFileArray = NULL;
				m_AsyncIOData = NULL;

				
				m_FreeAsyncFile = NULL;
				m_pFreeAsyncIOList = NULL;
				m_pAsyncIOList = NULL;
				m_pLastErrorStack = NULL;

				mInitCommonComplete = 0;
				mIoThread = NULL;
			}


			 xcore::s32			getMaxAsyncOp		( void )
			 {
				 return m_uMaxAsyncOperations;
			 }


			///< Synchronous file operations
			 xbool				exists				( const char* szFilename );
			 u64				getLength			( u32 uHandle );
			 void				setLength			( u32 uHandle, u64 uFileSize );
			 xbool				caps				( const xfilepath& szFilename, bool& can_read, bool& can_write, bool& can_seek, bool& can_async );

			 u32				open				( const char* szFilename, xbool boRead = true, xbool boWrite = false, x_asyncio_callback_struct callback = x_asyncio_callback_struct() );
			 u64				read				( u32 uHandle, u64 uOffset, u64 uSize, void* pBuffer, x_asyncio_callback_struct callback = x_asyncio_callback_struct() );	
			 u64				write				( u32 uHandle, u64 uOffset, u64 uSize, const void* pBuffer, x_asyncio_callback_struct callback = x_asyncio_callback_struct() );
			 u64				getpos				( u32 uHandle );
			 u64				setpos				( u32 uHandle, u64 uPos );
			 void 				close				( u32& uHandle, x_asyncio_callback_struct callback = x_asyncio_callback_struct() );
			 void				closeAndDelete		( u32& uHandle, x_asyncio_callback_struct callback = x_asyncio_callback_struct() );
			 void				save				( const char* szFilename, const void* pData, u64 uSize );
			 u32				createFile			( const char* szFilename, xbool boRead = true, xbool boWrite = false);

			 xbool				hasLastError		( void );
			 void				clearLastError		( void );
			 EError				getLastError		( void );
			 const char*		getLastErrorStr		( void );

			//////////////////////////////////////////////////////////////////////////
			// Private xfilesystem functionality
			//////////////////////////////////////////////////////////////////////////
		



			///< Threading
			 void				setIoThreadInterface( xio_thread* io_thread );
			 xio_thread*		getIoThreadInterface( void );

			///< Common
			 void				initialiseCommon	( u32 uMaxOpenStreams );
			 void				shutdownCommon		( void );

			 xfiledevice*		createSystemPath	( const char* szFilename, xcstring& outFilename );

			 xfiledata*			getFileInfo			( u32 uHandle );
			 u32				popFreeFileSlot		( void );
			 bool				pushFreeFileSlot	( u32 uHandle );

			 xfileasync*		getAsyncIOData		( u32 nSlot );
			 xfileasync*		popFreeAsyncIO		( bool wait = false );
			 void				pushFreeAsyncIO		( xfileasync* asyncIOInfo );

			 xfileasync*		popAsyncIO			( void );
			 xasync_id			pushAsyncIO			( xfileasync* asyncIOInfo );
			 u32				testAsyncId			( xasync_id id );


			 xbool				isPathUNIXStyle		( void );					///< UNIX = '/', Win32 = '\'

			///< Error
			 void				setLastError		( EError error );


		};
	};

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

//==============================================================================
// END __X_FILESYSTEM_COMMON_H__
//==============================================================================
#endif
