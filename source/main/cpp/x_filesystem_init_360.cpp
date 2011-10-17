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
		enum Xbox360_DriveTypes
		{
			DRIVE_TYPE_HOST							= 0,
			DRIVE_TYPE_DVD					        = 1,
			DRIVE_TYPE_HDD							= 2,
			DRIVE_TYPE_NUM						    = 3,
		};

		static xfiledevice*	sSystemFileDevice[DRIVE_TYPE_NUM] = 
		{
				NULL,
				NULL,
				NULL
		};

		void init(u32 max_open_streams, xio_thread* threading, x_iallocator* allocator)
		{
			xfilesystem::setAllocator(allocator);
			xdevicealias::init();

			sSystemFileDevice[DRIVE_TYPE_DVD] = x_CreateFileDevice360("GAME:\\",true);
			sSystemFileDevice[DRIVE_TYPE_HOST] = x_CreateFileDevice360("DEVKIT:\\",true);
			sSystemFileDevice[DRIVE_TYPE_HDD] = x_CreateFileDevice360("HDD:\\",true);


			xfilesystem::xdevicealias dvd  ("dvd"  , sSystemFileDevice[DRIVE_TYPE_DVD], "GAME:\\");				///< DVD Drive (read-only)
			xfilesystem::xdevicealias host ("host" , sSystemFileDevice[DRIVE_TYPE_HOST], "DEVKIT:\\");
			xfilesystem::xdevicealias save ("hdd"  , sSystemFileDevice[DRIVE_TYPE_HDD], "HDD:\\");

			xfilesystem::xdevicealias::sRegister(dvd);
			xfilesystem::xdevicealias::sRegister(host);
			xfilesystem::xdevicealias::sRegister(save);

			xfilesystem::xdevicealias appdir( "appdir", "host" );
			xfilesystem::xdevicealias curdir( "curdir", "host" );
			xfilesystem::xdevicealias::sRegister(appdir);
			xfilesystem::xdevicealias::sRegister(curdir);

			xfilesystem::initialise(max_open_streams);
			xfilesystem::xfs_common::instance()->setIoThreadInterface(io_thread);
		}

		//------------------------------------------------------------------------------
		void exit()
		{
			xfilesystem::shutdown();
			xdevicealias::exit();

			for (s32 i=0; i<DRIVE_TYPE_NUM; ++i)
			{
				xfiledevice* device = sSystemFileDevice[i];
				if (device != NULL)
				{
					x_DestroyFileDevice360(device);
					sSystemFileDevice[i] = NULL;
				} 
			}

			xfilesystem::setAllocator(NULL);
		}
	}

//==============================================================================
// END xcore namespace
//==============================================================================
};
#endif // TARGET_360