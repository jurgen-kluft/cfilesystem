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
#include "xfilesystem\private\x_filesystem_WII.h"

namespace xcore
{
	//------------------------------------------------------------------------------
	namespace xfilesystem
	{
		static xfiledevice*	sSystemFileDevice = NULL;

		void init(x_iothread* io_thread, x_iallocator* allocator)
		{
			xfilesystem::setAllocator(allocator);
			xfilesystem::setIoThread(io_thread);
			xfilesystem::initAlias();

			sSystemFileDevice = x_CreateFileDeviceWII();

			xfilesystem::xalias host ("host" , sSystemFileDevice, "/");
			xfilesystem::xalias dvd  ("dvd"  , sSystemFileDevice, "/");
			xfilesystem::xalias cache("cache", sSystemFileDevice, "/");

			xfilesystem::addAlias(host);
			xfilesystem::addAlias(dvd);
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

			x_DestroyFileDeviceWII(sSystemFileDevice);

			xfilesystem::setIoThread(NULL);
			xfilesystem::setAllocator(NULL);
		}
	}
	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

#endif // TARGET_WII