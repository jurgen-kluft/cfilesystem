#include "xbase\x_target.h"
#ifdef TARGET_PS3

#include <sys/paths.h>
#include "xfilesystem\private\x_filesystem_common.h"
#include "xfilesystem\private\x_filesystem_PS3.h"

namespace xcore
{
	namespace xfilesystem
	{
		static xfiledevice*	sSystemFileDevice = NULL;

		void init(xthreading* threading, x_iallocator* allocator)
		{
			xfilesystem::setAllocator(allocator);
			xfilesystem::setThreading(threading);
			xfilesystem::initAlias();

			sSystemFileDevice = x_CreateFileDevicePS3();

			xfilesystem::xdevicealias host ("host" , sSystemFileDevice, SYS_APP_HOME"/");
			xfilesystem::xdevicealias dvd  ("dvd"  , sSystemFileDevice, SYS_DEV_BDVD"/PS3_GAME/USRDIR/");
			xfilesystem::xdevicealias hdd0 ("hdd0" , sSystemFileDevice, SYS_DEV_HDD0"/game/TEST12345/USRDIR/");		//[hj]: hack directory
			xfilesystem::xdevicealias ms   ("ms"   , sSystemFileDevice, SYS_DEV_MS"/");
			xfilesystem::xdevicealias cache("cache", sSystemFileDevice, SYS_DEV_MS"/");

			xfilesystem::gAddAlias(host);
			xfilesystem::gAddAlias(dvd);
			xfilesystem::gAddAlias(hdd0);
			xfilesystem::gAddAlias(ms);
			xfilesystem::gAddAlias(cache);

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

			x_DestroyFileDevicePS3(sSystemFileDevice);

			xfilesystem::setThreading(NULL);
			xfilesystem::setAllocator(NULL);
		}
	}

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

#endif // TARGET_PS3
