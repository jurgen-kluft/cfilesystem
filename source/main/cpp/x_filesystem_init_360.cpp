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

		void init(x_iothread* io_thread, x_iallocator* allocator)
		{
			xfilesystem::setAllocator(allocator);
			xfilesystem::setIoThread(io_thread);
			xfilesystem::initAlias();

			sSystemFileDevice = x_CreateFileDevice360();

			xfilesystem::xalias dvd  ("dvd"  , sSystemFileDevice, "GAME:\\");				///< DVD Drive (read-only)
			xfilesystem::xalias host ("host" , sSystemFileDevice, "DEVKIT:\\");
			xfilesystem::xalias save ("HDD"  , sSystemFileDevice, "HDD:\\");

			xfilesystem::gAddAlias(dvd);
			xfilesystem::gAddAlias(host);
			xfilesystem::gAddAlias(save);

			xfilesystem::xalias appdir( "appdir", "host" );
			xfilesystem::xalias curdir( "curdir", "host" );
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

			xfilesystem::setIoThread(NULL);
			xfilesystem::setAllocator(NULL);
		}
	}

//==============================================================================
// END xcore namespace
//==============================================================================
};
#endif // TARGET_360