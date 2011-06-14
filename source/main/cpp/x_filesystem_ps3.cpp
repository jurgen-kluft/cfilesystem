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

		//---------------
		// Public Methods
		//---------------

		//------------------------------------------------------------------------------------------

		void				getOpenCreatedTime (u32 uHandle, xdatetime& pTimeAndDate)
		{
			ASSERTS (pTimeAndDate, "GetOpenCreatedTime() : Pointer to xsystem::TimeAndDate is NULL!");

			CellFsStat	xStat;
			xfiledata* pxFileInfo = getFileInfo(uHandle);
			CellFsErrno eError = cellFsFstat(pxFileInfo->m_nFileHandle, &xStat);

			pTimeAndDate = xdatetime::sFromFileTime(xStat.st_ctime);
		}

		//------------------------------------------------------------------------------------------

		void				getOpenModifiedTime (u32 uHandle, xdatetime& pTimeAndDate)
		{
			ASSERTS (pTimeAndDate, "GetOpenModifiedTime() : Pointer to xsystem::TimeAndDate is NULL!");

			CellFsStat	xStat;
			xfiledata* pxFileInfo = getFileInfo(uHandle);
			CellFsErrno eError = cellFsFstat(pxFileInfo->m_nFileHandle, &xStat);

			pTimeAndDate = xdatetime::sFromFileTime(xStat.st_ctime);
		}

		//------------------------------------------------------------------------------------------

		u64					getFreeSize( const char* szPath )
		{
			u32	uBlockSize;
			u64	uBlockCount;
			cellFsGetFreeSize(szPath, &uBlockSize, &uBlockCount);

			return uBlockSize * uBlockCount;
		}

		//------------------------------------------------------------------------------------------

		void				initialise ( u32 uMaxOpenStreams )
		{
			initialiseCommon(uMaxOpenStreams);
		}	

		//------------------------------------------------------------------------------------------

		void				shutdown ( void )
		{
			shutdownCommon();
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