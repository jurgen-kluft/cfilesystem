#include "xbase\x_target.h"
#ifdef TARGET_360
#include "xbase\x_system.h"

#include "xfilesystem\x_filedevice.h"
#include "xfilesystem\private\x_filesystem_common.h"
#include "xfilesystem\private\x_filesystem_360.h"

namespace xcore
{
	//------------------------------------------------------------------------------
	namespace xfilesystem
	{
		static xfiledevice*	sSystemFileDevice = NULL;

		void init(xthreading* threading, x_iallocator* allocator)
		{
			xfilesystem::setAllocator(allocator);
			xfilesystem::setThreading(threading);
			xfilesystem::initAlias();

			sSystemFileDevice = x_CreateFileDevice360();

			xfilesystem::xdevicealias dvd  ("dvd"  , sSystemFileDevice, "GAME:\\");				///< DVD Drive (read-only)
			xfilesystem::xdevicealias host ("host" , sSystemFileDevice, "DEVKIT:\\");
			xfilesystem::xdevicealias save ("HDD"  , sSystemFileDevice, "HDD:\\");

			xfilesystem::gAddAlias(dvd);
			xfilesystem::gAddAlias(host);
			xfilesystem::gAddAlias(save);

			xfilesystem::xdevicealias appdir( "appdir", "host" );
			xfilesystem::xdevicealias curdir( "curdir", "host" );
			xfilesystem::gAddAlias(appdir);
			xfilesystem::gAddAlias(curdir);

			xfilesystem::initialise(64, xTRUE);
		}

		//------------------------------------------------------------------------------
		void exit()
		{
			xfilesystem::shutdown();
			xfilesystem::exitAlias();

			x_DestroyFileDevice360(sSystemFileDevice);

			xfilesystem::setThreading(NULL);
			xfilesystem::setAllocator(NULL);
		}
	}

//==============================================================================
// END xcore namespace
//==============================================================================
};
#endif // TARGET_360