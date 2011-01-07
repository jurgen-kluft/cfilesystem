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