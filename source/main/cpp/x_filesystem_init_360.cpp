#include "xbase\x_target.h"
#ifdef TARGET_360

//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase\x_system.h"

#include "xfilesystem\private\x_filesystem_common.h"

//==============================================================================
// xCore namespace
//==============================================================================
namespace xcore
{
	//------------------------------------------------------------------------------
	namespace xfilesystem
	{

		void init(void)
		{
			xfilesystem::xalias dvd  ("dvd"  , xfilesystem::FS_SOURCE_DVD,  "GAME:\\");				///< DVD Drive (read-only)
			xfilesystem::xalias host ("host" , xfilesystem::FS_SOURCE_HOST, "DEVKIT:\\");
			xfilesystem::xalias save ("HDD" , xfilesystem::FS_SOURCE_MS, "HDD:\\");

			xfilesystem::AddAlias(dvd);
			xfilesystem::AddAlias(host);
			xfilesystem::AddAlias(save);

			xfilesystem::xalias appdir( "appdir", "host" );
			xfilesystem::xalias curdir( "curdir", "host" );
			xfilesystem::AddAlias(appdir);
			xfilesystem::AddAlias(curdir);

			xfilesystem::Initialise(64, xTRUE);
		}

		//------------------------------------------------------------------------------

		void exit()
		{
			xfilesystem::Shutdown();
			xfilesystem::ExitAlias();
		}

	}

	//==============================================================================
	// END xCore namespace
	//==============================================================================
};

#endif // TARGET_360