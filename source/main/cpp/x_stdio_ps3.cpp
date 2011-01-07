#include "../x_target.h"
#ifdef TARGET_PS3

//==============================================================================
// INCLUDES
//==============================================================================
#include <sys/paths.h>

#include "../x_system.h"
#include "x_filesystem_common.h"

//==============================================================================
// xCore namespace
//==============================================================================
namespace xcore
{
	//------------------------------------------------------------------------------

	void x_StdioInit(void)
	{
		xfilesystem::xalias host ("host" , xfilesystem::FS_SOURCE_HOST, SYS_APP_HOME"/");
		xfilesystem::xalias dvd  ("dvd"  , xfilesystem::FS_SOURCE_DVD, SYS_DEV_BDVD"/PS3_GAME/USRDIR/");
		xfilesystem::xalias hdd0 ("hdd0" , xfilesystem::FS_SOURCE_HDD, SYS_DEV_HDD0"/game/TEST12345/USRDIR/");		//[hj]: hack directory
		xfilesystem::xalias ms   ("ms"   , xfilesystem::FS_SOURCE_MS, SYS_DEV_MS"/");
		xfilesystem::xalias cache("cache", xfilesystem::FS_SOURCE_CACHE, SYS_DEV_MS"/");

		xfilesystem::AddAlias(host);
		xfilesystem::AddAlias(dvd);
		xfilesystem::AddAlias(hdd0);
		xfilesystem::AddAlias(ms);
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

#endif // TARGET_PS3