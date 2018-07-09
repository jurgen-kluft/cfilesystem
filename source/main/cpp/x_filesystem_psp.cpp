#include "xbase/x_target.h"
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

#include "xbase/x_debug.h"
#include "xbase/x_system.h"
#include "xbase/x_string_std.h"

#include "xtime/x_datetime.h"

#include "xfilesystem/private/x_filesystem_common.h"
#include "xfilesystem/private/x_filesystem_psp.h"

//==============================================================================
// xcore namespace
//==============================================================================
namespace xcore
{
	namespace xfilesystem
	{
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

		//------------------------------------------------------------------------------------------

		void				initialise ( u32 uMaxOpenStreams )
		{
			xfs_common::create();
			xfs_common::s_instance()->initialiseCommon( uMaxOpenStreams );
		}	

		//------------------------------------------------------------------------------------------

		void				shutdown ( void )
		{
			xfs_common::s_instance()->shutdownCommon();
			xfs_common::destroy();
		}

		//------------------------------------------------------------------------------------------

		xbool				isPathUNIXStyle		( void )
		{
			return true;
		}

	};
};


namespace xfilesystem
{
	//------------------------------------------------------------------------------------------

	void			setDRMLicenseKey( const u8* p8Key )
	{
// 			s32 nError = sceNpDrmSetLicenseeKey ((const SceNpDrmKey*)p8Key);
// 
// 			if(nError < 0)
// 			{
// 				x_printf ("xfilesystem: " TARGET_PLATFORM_STR " ERROR sceNpDrmSetLicenseeKey Error 0x%lx\n", x_va_list(nError));
// 			}
	}

	//------------------------------------------------------------------------------------------

	s32				saveToMemoryStick(const char* szPhotosDir, const char* szDirectory, char* szFilename, void* pData, s32 nSizeBytes )
	{
		SceUID uid;

		s32 retval = sceIoMkdir( szPhotosDir, 0 );
		if ( retval != SCE_KERNEL_ERROR_OK )
		{
			//x_printf("xfilesystem: " TARGET_PLATFORM_STR " ERROR sceIoMkdir szPhotosDir fail\n");
		}

		retval = sceIoMkdir( szDirectory, 0 );
		if ( retval != SCE_KERNEL_ERROR_OK )
		{
			//x_printf("xfilesystem: " TARGET_PLATFORM_STR " ERROR sceIoMkdir szDirectory fail\n");
		}

		uid = sceIoOpen( szFilename, SCE_O_RDWR | SCE_O_TRUNC | SCE_O_CREAT, 0777);

		if(uid < 0)
		{
			x_printf("xfilesystem: " TARGET_PLATFORM_STR " ERROR sceIoOpen fail!!\n");
			return uid;
		}
		else
		{
			s32 nResult = sceIoWrite(uid, pData, nSizeBytes);
			if( 0 > nResult )
			{
				x_printf("xfilesystem: " TARGET_PLATFORM_STR " ERROR sceIoWrite fail\n");
				sceIoClose(uid);
				return nResult;
			}

			nResult = sceIoClose(uid);
			return nResult;
		}
	}

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

#endif // TARGET_PSP