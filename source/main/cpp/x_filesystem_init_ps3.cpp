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

		void init(x_iothread* io_thread, x_iallocator* allocator)
		{
			xfilesystem::setAllocator(allocator);
			xfilesystem::setIoThread(io_thread);
			xfilesystem::initAlias();

			sSystemFileDevice = x_CreateFileDevicePS3();

			xfilesystem::xalias host ("host" , sSystemFileDevice, SYS_APP_HOME"/");
			xfilesystem::xalias dvd  ("dvd"  , sSystemFileDevice, SYS_DEV_BDVD"/PS3_GAME/USRDIR/");
			xfilesystem::xalias hdd0 ("hdd0" , sSystemFileDevice, SYS_DEV_HDD0"/game/TEST12345/USRDIR/");		//[hj]: hack directory
			xfilesystem::xalias ms   ("ms"   , sSystemFileDevice, SYS_DEV_MS"/");
			xfilesystem::xalias cache("cache", sSystemFileDevice, SYS_DEV_MS"/");

			xfilesystem::addAlias(host);
			xfilesystem::addAlias(dvd);
			xfilesystem::addAlias(hdd0);
			xfilesystem::addAlias(ms);
			xfilesystem::addAlias(cache);

			xfilesystem::xalias appdir( "appdir", "host" );
			xfilesystem::xalias curdir( "curdir", "host" );

			xfilesystem::addAlias(appdir);
			xfilesystem::addAlias(curdir);
			
			xfilesystem::initialise(64, xTRUE);
		}

		//------------------------------------------------------------------------------
		void exit()
		{
			xfilesystem::shutdown();
			xfilesystem::exitAlias();

			x_DestroyFileDevicePS3(sSystemFileDevice);

			xfilesystem::setIoThread(NULL);
			xfilesystem::setAllocator(NULL);
		}
	}

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

#endif // TARGET_PS3
