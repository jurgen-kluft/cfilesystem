#include "xbase\x_target.h"
#ifdef TARGET_3DS

//==============================================================================
// INCLUDES
//==============================================================================

#include "xbase\x_types.h"
#include "xbase\x_debug.h"
#include "xbase\x_string_std.h"
#include "xbase\x_va_list.h"

#include "xfilesystem\x_filesystem.h"
#include "xfilesystem\private\x_filesystem_common.h"
#include "xfilesystem\private\x_filesystem_3ds.h"

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

			xfilesystem::xdevicealias host ("host" , sSystemFileDevice, "/");
			xfilesystem::xdevicealias dvd  ("dvd"  , sSystemFileDevice, "/");
			xfilesystem::xdevicealias cache("cache", sSystemFileDevice, "/");

			xfilesystem::gAddAlias(host);
			xfilesystem::gAddAlias(dvd);
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

			x_DestroyFileDevice360(sSystemFileDevice);

			xfilesystem::setThreading(NULL);
			xfilesystem::setAllocator(NULL);
		}
	}
	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

#endif // TARGET_3DS