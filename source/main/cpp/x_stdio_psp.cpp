#include "../x_target.h"
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

#include "../x_debug.h"
#include "../x_system.h"
#include "../x_thread.h"
#include "../x_container.h"
#include "../x_string.h"
#include "../x_va_list.h"
#include "../x_llist.h"
#include "../x_stdio.h"

#include "x_filesystem.h"
#include "x_filesystem_common.h"


namespace xcore
{
	//------------------------------------------------------------------------------
	void x_StdioInit(void)
	{
		xfilesystem::AddAlias(xfilesystem::xalias("host", xfilesystem::FS_SOURCE_HOST, "host0:"));
		xfilesystem::AddAlias(xfilesystem::xalias("dvd" , xfilesystem::FS_SOURCE_DVD , "disc0:/PSP_GAME/USRDIR/" )); 

		xfilesystem::xalias appdir( "appdir", "host" );
		xfilesystem::xalias curdir( "curdir", "host" );
		xfilesystem::AddAlias(appdir);
		xfilesystem::AddAlias(curdir);

		xfilesystem::Initialise(64, xTRUE);
	}

	void x_StdioExit()
	{
		xfilesystem::Shutdown();
		xfilesystem::ExitAlias();
	}

	//==============================================================================
	// END xCore namespace
	//==============================================================================
};

#endif // TARGET_PSP