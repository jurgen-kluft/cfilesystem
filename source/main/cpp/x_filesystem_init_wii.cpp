#include "xbase\x_target.h"
#ifdef TARGET_WII

//==============================================================================
// INCLUDES
//==============================================================================

#include "xbase\x_types.h"
#include "xbase\x_debug.h"
#include "xbase\x_string_std.h"
#include "xbase\x_va_list.h"

#include "xfilesystem\x_filesystem.h"
#include "xfilesystem\private\x_filesystem_common.h"

namespace xcore
{
	//------------------------------------------------------------------------------
	namespace xfilesystem
	{
		void init(void)
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

#endif // TARGET_WII