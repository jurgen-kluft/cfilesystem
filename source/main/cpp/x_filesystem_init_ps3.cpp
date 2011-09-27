#include "xbase\x_target.h"
#ifdef TARGET_PS3

#include <sys/paths.h>

#include "xfilesystem\x_filedevice.h"
#include "xfilesystem\private\x_filesystem_common.h"
#include "xfilesystem\private\x_filesystem_ps3.h"
#include "xfilesystem\private\x_devicealias.h"

#include "xfilesystem\x_filesystem.h"
#include "xfilesystem\x_filepath.h"

namespace xcore
{
	namespace xfilesystem
	{
		static xfiledevice*	sSystemFileDevice = NULL;

		void init(u32 max_open_streams, xio_thread* threading, x_iallocator* allocator)
		{
			xfilesystem::setAllocator(allocator);
			xfilesystem::setIoThreadInterface(threading);
			xdevicealias::init();

//			sSystemFileDevice = x_CreateFileDevicePS3();
			sSystemFileDevice = x_CreateFileDevicePS3(true);
			xfilesystem::xdevicealias host ("host" , sSystemFileDevice, SYS_APP_HOME"/");
			xfilesystem::xdevicealias dvd  ("dvd"  , sSystemFileDevice, SYS_DEV_BDVD"/PS3_GAME/USRDIR/");
			xfilesystem::xdevicealias hdd0 ("hdd"  , sSystemFileDevice, SYS_DEV_HDD0"/game/TEST12345/USRDIR/");		//[hj]: hack directory

			xfilesystem::xdevicealias::sRegister(host);
			xfilesystem::xdevicealias::sRegister(dvd);
			xfilesystem::xdevicealias::sRegister(hdd0);

			xfilesystem::xdevicealias appdir( "appdir", "host" );
			xfilesystem::xdevicealias curdir( "curdir", "host" );

			xfilesystem::xdevicealias::sRegister(appdir);
			xfilesystem::xdevicealias::sRegister(curdir);
			
			xfilesystem::initialise(max_open_streams);
		}

		//------------------------------------------------------------------------------
		void exit()
		{
			xfilesystem::shutdown();
			xdevicealias::exit();

			x_DestroyFileDevicePS3(sSystemFileDevice);

			xfilesystem::setIoThreadInterface(NULL);
			xfilesystem::setAllocator(NULL);
		}
	}

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

#endif // TARGET_PS3
