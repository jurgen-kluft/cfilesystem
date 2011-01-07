#include "../x_target.h"
#ifdef TARGET_WII

//==============================================================================
// INCLUDES
//==============================================================================

#include "../x_types.h"
#include "../x_debug.h"
#include "../x_stdio.h"
#include "../x_thread.h"
#include "../x_container.h"
#include "../x_string.h"
#include "../x_va_list.h"

#include "x_filesystem.h"
#include "x_filesystem_common.h"

namespace xcore
{
	//------------------------------------------------------------------------------

	void x_StdioInit(void)
	{
		xfilesystem::xalias host ("host" , xfilesystem::FS_SOURCE_HOST,	"/");
		xfilesystem::xalias dvd  ("dvd"  , xfilesystem::FS_SOURCE_DVD,	"/");
		xfilesystem::xalias cache("cache", xfilesystem::FS_SOURCE_CACHE,"/");

		xfilesystem::AddAlias(host);
		xfilesystem::AddAlias(dvd);
		xfilesystem::AddAlias(cache);

		xfilesystem::xalias appdir( "appdir", "host" );
		xfilesystem::xalias curdir( "curdir", "host" );
		xfilesystem::AddAlias(appdir);
		xfilesystem::AddAlias(curdir);

		xfilesystem::Initialise(64, xTRUE);
	}

	//------------------------------------------------------------------------------

	void x_StdioExit()
	{
		xfilesystem::Shutdown();
		xfilesystem::ExitAlias();
	}

	//==============================================================================
	// END xCore namespace
	//==============================================================================
};

#endif // TARGET_WII