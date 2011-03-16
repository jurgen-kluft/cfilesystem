#include "xbase\x_target.h"
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

#include "xbase\x_debug.h"
#include "xbase\x_string_std.h"
#include "xbase\x_va_list.h"

#include "xfilesystem\x_filesystem.h"
#include "xfilesystem\private\x_filesystem_common.h"
#include "xfilesystem\private\x_filesystem_PSP.h"

namespace xcore
{
	namespace xfilesystem
	{
		static xfiledevice*	sSystemFileDevice = NULL;

		//------------------------------------------------------------------------------
		void init(x_iothread* io_thread, x_iallocator* allocator)
		{
			xfilesystem::setAllocator(allocator);
			xfilesystem::setIoThread(io_thread);
			xfilesystem::initAlias();

			sSystemFileDevice = x_CreateFileDevicePSP();

			xfilesystem::gAddAlias(xfilesystem::xalias("host", sSystemFileDevice, "host0:"));
			xfilesystem::gAddAlias(xfilesystem::xalias("dvd" , sSystemFileDevice, "disc0:/PSP_GAME/USRDIR/" )); 

			xfilesystem::xalias appdir( "appdir", "host" );
			xfilesystem::xalias curdir( "curdir", "host" );
			xfilesystem::gAddAlias(appdir);
			xfilesystem::gAddAlias(curdir);

			xfilesystem::initialise(64, xTRUE);
		}

		void exit()
		{
			xfilesystem::shutdown();
			xfilesystem::exitAlias();

			x_DestroyFileDevicePSP(sSystemFileDevice);

			xfilesystem::setIoThread(NULL);
			xfilesystem::setAllocator(NULL);
		}

	}
	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

#endif // TARGET_PSP