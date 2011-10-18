#include "xbase\x_target.h"
#ifdef TARGET_WII

//==============================================================================
// INCLUDES
//==============================================================================

#include "xbase\x_types.h"
#include "xbase\x_debug.h"
#include "xbase\x_string_std.h"
#include "xbase\x_va_list.h"

#include "xfilesystem\x_filedevice.h"
#include "xfilesystem\private\x_filesystem_common.h"
#include "xfilesystem\private\x_filesystem_wii.h"
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
			xdevicealias::init();

			sSystemFileDevice = x_CreateFileDeviceWII();

			xfilesystem::xdevicealias host ("host" , sSystemFileDevice, "/");
			xfilesystem::xdevicealias dvd  ("dvd"  , sSystemFileDevice, "/");
			xfilesystem::xdevicealias cache("cache", sSystemFileDevice, "/");

			xfilesystem::sRegister(host);
			xfilesystem::sRegister(dvd);
			xfilesystem::sRegister(cache);

			xfilesystem::xdevicealias appdir( "appdir", "host" );
			xfilesystem::xdevicealias curdir( "curdir", "host" );
			xfilesystem::sRegister(appdir);
			xfilesystem::sRegister(curdir);

			xfilesystem::initialise(max_open_streams);
			xfilesystem::xfs_common::s_instance()->setIoThreadInterface(io_thread);
		}

		//------------------------------------------------------------------------------
		void exit()
		{
			xfilesystem::shutdown();
			xdevicealias::exit();

			x_DestroyFileDeviceWII(sSystemFileDevice);

			xfilesystem::setAllocator(NULL);
		}
	}
	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

#endif // TARGET_WII