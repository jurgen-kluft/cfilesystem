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

		void init(u32 max_open_streams, xthreading* threading, x_iallocator* allocator)
		{
			xfilesystem::setAllocator(allocator);
			xfilesystem::setThreading(threading);
			xfilesystem::initAlias();

			sSystemFileDevice = x_CreateFileDevicePS3();

			xfilesystem::xdevicealias host ("host" , sSystemFileDevice, SYS_APP_HOME"/");
			xfilesystem::xdevicealias dvd  ("dvd"  , sSystemFileDevice, SYS_DEV_BDVD"/PS3_GAME/USRDIR/");
			xfilesystem::xdevicealias hdd0 ("hdd"  , sSystemFileDevice, SYS_DEV_HDD0"/game/TEST12345/USRDIR/");		//[hj]: hack directory

			xfilesystem::sRegister(host);
			xfilesystem::sRegister(dvd);
			xfilesystem::sRegister(hdd0);

			xfilesystem::xdevicealias appdir( "appdir", "host" );
			xfilesystem::xdevicealias curdir( "curdir", "host" );

			xfilesystem::sRegister(appdir);
			xfilesystem::sRegister(curdir);
			
			xfilesystem::initialise(max_open_streams);
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
