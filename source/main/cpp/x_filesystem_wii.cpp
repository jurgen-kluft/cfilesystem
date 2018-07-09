#include "xbase/x_target.h"
#ifdef TARGET_WII

#include <revolution.h>
#include <revolution\dvd.h>

//==============================================================================
// INCLUDES
//==============================================================================

#include "xbase/x_types.h"
#include "xbase/x_debug.h"
#include "xbase/x_string_std.h"

#include "xtime/x_datetime.h"

#include "xfilesystem/private/x_filesystem_common.h"
#include "xfilesystem/private/x_filesystem_wii.h"

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
			u32	uBlockSize  = 0;
			u64	uBlockCount = 0;
			//cellFsGetFreeSize(szPath, &uBlockSize, &uBlockCount);

			return uBlockSize * uBlockCount;
		}

		//------------------------------------------------------------------------------------------

		void				initialise ( u32 uMaxOpenStreams )
		{
			xfs_common::s_create();
			xfs_common::s_instance()->initialiseCommon( uMaxOpenStreams );
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
			return true;
		}

	};


	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

#endif // TARGET_WII