#include "xbase\x_target.h"
#ifdef TARGET_PC

//==============================================================================
// INCLUDES
//==============================================================================

#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOKANJI
#include <windows.h>
#include <stdio.h>

#include "xbase\x_debug.h"
#include "xbase\x_string_std.h"
#include "xbase\x_va_list.h"

#include "xtime\x_time.h"

#include "xfilesystem\private\x_filesystem_common.h"
#include "xfilesystem\private\x_filesystem_win32.h"

#include "xfilesystem\x_filesystem.h"
#include "xfilesystem\x_filedevice.h"
#include "xfilesystem\private\x_fileinfo.h"
#include "xfilesystem\private\x_fileasync.h"

namespace xcore
{
	namespace xfilesystem
	{
		//---------
		// Forward declares
		//---------


		//---------------
		// Public Methods
		//---------------

		//------------------------------------------------------------------------------------------

		void				getOpenCreatedTime (u32 uHandle, xdatetime& pTimeAndDate)
		{
			//ASSERTS (pTimeAndDate, "GetOpenCreatedTime() : Pointer to xsystem::TimeAndDate is NULL!");

			//CellFsStat	xStat;
			//CellFsErrno eError = cellFsFstat(m_OpenAsyncFile[uHandle].m_nFileHandle, &xStat);

			pTimeAndDate = xdatetime::sFromFileTime(0);
		}

		//------------------------------------------------------------------------------------------

		void				getOpenModifiedTime (u32 uHandle, xdatetime& pTimeAndDate)
		{
			//ASSERTS (pTimeAndDate, "GetOpenModifiedTime() : Pointer to xsystem::TimeAndDate is NULL!");

			//CellFsStat	xStat;
			//CellFsErrno eError = cellFsFstat(m_OpenAsyncFile[uHandle].m_nFileHandle, &xStat);

			pTimeAndDate = xdatetime::sFromFileTime(0);
		}

		//------------------------------------------------------------------------------------------

		void				setLength( u32 uHandle, u64 uNewSize )
		{
			xfiledata* pInfo = getFileInfo(uHandle);

			s32 nResult=-1;
			nResult	= pInfo->m_pFileDevice->setLengthOfFile(pInfo->m_nFileHandle, uNewSize);
			if(nResult < 0)
			{
				x_printf("xfilesystem:"TARGET_PLATFORM_STR" ERROR ReSize %d\n", x_va_list(nResult));
			}
		}

		//------------------------------------------------------------------------------------------

		u64					getFreeSize( const char* szPath )
		{
			u32	uBlockSize = 0;
			u64	uBlockCount = 0;
// 			cellFsGetFreeSize(szPath, &uBlockSize, &uBlockCount);

			return uBlockSize * uBlockCount;
		}

		//------------------------------------------------------------------------------------------

		void				initialise ( xbool boEnableCache )
		{
			initialiseCommon( boEnableCache );
		}	

		//------------------------------------------------------------------------------------------

		void				shutdown ( void )
		{
			shutdownCommon();
		}

		//------------------------------------------------------------------------------------------

		xbool				isPathUNIXStyle		( void )
		{
			return false;
		}

	};

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

#endif // TARGET_PC
