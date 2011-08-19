#include "xbase\x_target.h"
#ifdef TARGET_360
#include "xbase\x_types.h"

#include "xfilesystem\x_filedevice.h"
#include "xfilesystem\private\x_filesystem_common.h"
#include "xfilesystem\private\x_filesystem_360.h"
#include "xfilesystem\private\x_devicealias.h"

#include "xfilesystem\x_filesystem.h"
#include "xfilesystem\x_filepath.h"

namespace xcore
{
	//------------------------------------------------------------------------------
	namespace xfilesystem
	{
		static xfiledevice*	sSystemFileDevice = NULL;

		void init(u32 max_open_streams, xio_thread* threading, x_iallocator* allocator)
		{
			xfilesystem::setAllocator(allocator);
			xfilesystem::setIoThreadInterface(threading);
			xdevicealias::init();

			sSystemFileDevice = x_CreateFileDevice360(true);

			xfilesystem::xdevicealias dvd  ("dvd"  , sSystemFileDevice, "GAME:\\");				///< DVD Drive (read-only)
			xfilesystem::xdevicealias host ("host" , sSystemFileDevice, "DEVKIT:\\");
			xfilesystem::xdevicealias save ("hdd"  , sSystemFileDevice, "HDD:\\");

			xfilesystem::xdevicealias::sRegister(dvd);
			xfilesystem::xdevicealias::sRegister(host);
			xfilesystem::xdevicealias::sRegister(save);

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

			x_DestroyFileDevice360(sSystemFileDevice);

			xfilesystem::setIoThreadInterface(NULL);
			xfilesystem::setAllocator(NULL);
		}
	}

//==============================================================================
// END xcore namespace
//==============================================================================
};
#endif // TARGET_360