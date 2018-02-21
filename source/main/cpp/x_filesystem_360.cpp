#include "xbase/x_target.h"
#ifdef TARGET_360

//==============================================================================
// INCLUDES
//==============================================================================
#include <Xtl.h>
#ifndef TARGET_FINAL
#include <xbdm.h>
#endif

#include "xbase/x_debug.h"
#include "xbase/x_string_std.h"
#include "xbase/x_va_list.h"

#include "xtime/x_datetime.h"

#include "xfilesystem/private/x_filesystem_common.h"
#include "xfilesystem/private/x_filesystem_360.h"

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

			//CellFsStat	xStat;
			//CellFsErrno eError = cellFsFstat(m_OpenAsyncFile[uHandle].m_nFileHandle, &xStat);

			pTimeAndDate = xdatetime::sFromFileTime(0);
		}

		//------------------------------------------------------------------------------------------

		void				getOpenModifiedTime (u32 uHandle, xdatetime& pTimeAndDate)
		{
			ASSERTS (pTimeAndDate, "GetOpenModifiedTime() : Pointer to xsystem::TimeAndDate is NULL!");

			//CellFsStat	xStat;
			//CellFsErrno eError = cellFsFstat(m_OpenAsyncFile[uHandle].m_nFileHandle, &xStat);

			pTimeAndDate = xdatetime::sFromFileTime(0);
		}

		//------------------------------------------------------------------------------------------

		u64					getFreeSize( const char* szPath )
		{
			u32	uBlockSize = 0;
			u64	uBlockCount = 0;
			// 			cellFsGetFreeSize(szPath, &uBlockSize, &uBlockCount);

			return uBlockSize * uBlockCount;
		}


		//////////////////////////////////////////////////////////////////////////
		// Private xfilesystem functionality
		//////////////////////////////////////////////////////////////////////////

		//------------------------------------------------------------------------------------------

		void				initialise ( u32 uMaxOpenStreams )
		{
			xfs_common::s_create();
			xfs_common::s_instance()->initialiseCommon( uMaxOpenStreams );

#ifndef TARGET_FINAL
			HRESULT devkitDriveResult = DmMapDevkitDrive();
			if (devkitDriveResult != S_OK)
			{
				x_printf("xfilesystem:"TARGET_PLATFORM_STR" ERROR devkit drive could not be mounted");
			}
#endif
		}	

		//------------------------------------------------------------------------------------------

		void				shutdown ( void )
		{
			xfs_common::s_instance()->shutdownCommon();
			xfs_common::s_destroy();
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

#endif // TARGET_360