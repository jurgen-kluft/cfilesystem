#include "xbase/x_target.h"
#ifdef TARGET_PSP

//==============================================================================
// INCLUDES
//==============================================================================

#include <kernel.h>
#include <kerror.h>
#include <stdio.h>
#include <mediaman.h>
#include <psptypes.h>
#include <psperror.h>
#include <iofilemgr.h>
#include <mediaman.h>
#include <umddevctl.h>
#include <kernelutils.h>
#include <utility/utility_module.h>
#include <np/np_drm.h>

#include "xbase/x_debug.h"
#include "xbase/x_string_std.h"
#include "xbase/x_va_list.h"

#include "xfilesystem/x_filedevice.h"
#include "xfilesystem/private/x_filesystem_common.h"
#include "xfilesystem/private/x_filesystem_psp.h"
#include "xfilesystem/private/x_devicealias.h"

#include "xfilesystem/x_filesystem.h"
#include "xfilesystem/x_filepath.h"
namespace xcore
{
	namespace xfilesystem
	{
		static xfiledevice*	sSystemFileDevice = NULL;

		//------------------------------------------------------------------------------
		void init(u32 max_open_streams, xio_thread* threading, x_iallocator* allocator)
		{
			xfilesystem::setAllocator(allocator);

			xdevicealias::init();

			sSystemFileDevice = x_CreateFileDevicePSP();

			xfilesystem::sRegister(xfilesystem::xdevicealias("host", sSystemFileDevice, "host0:"));
			xfilesystem::sRegister(xfilesystem::xdevicealias("dvd" , sSystemFileDevice, "disc0:/PSP_GAME/USRDIR/" )); 

			xfilesystem::xdevicealias appdir( "appdir", "host" );
			xfilesystem::xdevicealias curdir( "curdir", "host" );
			xfilesystem::sRegister(appdir);
			xfilesystem::sRegister(curdir);

			xfilesystem::initialise(max_open_streams);
			xfilesystem::xfs_common::s_instance()->setIoThreadInterface(io_thread);
		}

		//------------------------------------------------------------------------------------------
		static char		m_pszMemoryCardPath[FS_MAX_PATH];
		void setMemoryCardPath	( const char* szMemoryCardPath )
		{
			xcstring mscardpath(m_pszMemoryCardPath, sizeof(m_pszMemoryCardPath), szMemoryCardPath);

			// Register memory stick alias
			xdevicealias ms("ms", sSystemFileDevice, mscardpath.c_str());
			xdevicealias::sRegister(ms);
		}

		//------------------------------------------------------------------------------------------
		void exit()
		{
			xfilesystem::shutdown();
			xdevicealias::exit();

			x_DestroyFileDevicePSP(sSystemFileDevice);

			xfilesystem::setAllocator(NULL);
		}

	}
	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

#endif // TARGET_PSP