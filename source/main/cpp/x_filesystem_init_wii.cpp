#include "xbase/x_target.h"
#ifdef TARGET_WII

//==============================================================================
// INCLUDES
//==============================================================================

#include "xbase/x_types.h"
#include "xbase/x_debug.h"
#include "xbase/x_string_std.h"
#include "xbase/x_va_list.h"

#include "xfilesystem/x_filedevice.h"
#include "xfilesystem/private/x_filesystem_common.h"
#include "xfilesystem/private/x_filesystem_wii.h"
#include "xfilesystem/private/x_devicealias.h"

#include "xfilesystem/x_filesystem.h"
#include "xfilesystem/x_filepath.h"

namespace xcore
{
	//------------------------------------------------------------------------------
	namespace xfilesystem
	{
		static xfiledevice*	sSystemFileDevice = NULL;
		static xfiledevice* sNandFileDevice = NULL;

		void init(u32 max_open_streams, xio_thread* io_thread, x_iallocator* allocator)
		{
			xfilesystem::setAllocator(allocator);
			xdevicealias::init();

			xfilesystem::initialise(max_open_streams);
			xfilesystem::xfs_common::s_instance()->setIoThreadInterface(io_thread);

			sSystemFileDevice = x_CreateFileDeviceWII();
			sNandFileDevice = x_CreateNandDeviceWII();

			xfilesystem::xdevicealias host ("host" , sSystemFileDevice, "/");
			xfilesystem::xdevicealias dvd  ("dvd"  , sSystemFileDevice, "/");
			xfilesystem::xdevicealias cache("cache", sSystemFileDevice, "/");

			xfilesystem::xdevicealias::sRegister(host);
			xfilesystem::xdevicealias::sRegister(dvd);
			xfilesystem::xdevicealias::sRegister(cache);

			xfilesystem::xdevicealias appdir( "appdir", "host" );
			xfilesystem::xdevicealias curdir( "curdir", "host" );
			xfilesystem::xdevicealias::sRegister(appdir);
			xfilesystem::xdevicealias::sRegister(curdir);

			xdevicealias nand("nand", sNandFileDevice, "/");
			xdevicealias::sRegister(nand);


		}

		//------------------------------------------------------------------------------
		void exit()
		{
			xfilesystem::shutdown();
			xdevicealias::exit();

			x_DestroyFileDeviceWII(sSystemFileDevice);
			xfilesystem::x_DestroyNandDeviceWII(sNandFileDevice);

			xfilesystem::setAllocator(NULL);
		}
	}
	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

#endif // TARGET_WII