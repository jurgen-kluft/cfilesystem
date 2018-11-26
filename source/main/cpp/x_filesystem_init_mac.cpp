#include "xbase/x_target.h"
#ifdef TARGET_MAC

#include <sys/paths.h>

#include "xfilesystem/x_filedevice.h"
#include "xfilesystem/private/x_filesystem_common.h"
#include "xfilesystem/private/x_filesystem_mac.h"
#include "xfilesystem/private/x_devicealias.h"

#include "xfilesystem/x_filesystem.h"
#include "xfilesystem/x_filepath.h"

namespace xcore
{
	enum MAC_DriveTypes
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

	void init(u32 max_open_streams, xio_thread* io_thread, x_iallocator* allocator)
	{
		xfilesystem::setAllocator(allocator);

		xdevicealias::init();

//			sSystemFileDevice = x_CreateFileDevicePS3();
		sSystemFileDevice[DRIVE_TYPE_HOST] = x_CreateFileDevicePS3(SYS_APP_HOME,true);
		sSystemFileDevice[DRIVE_TYPE_DVD] = x_CreateFileDevicePS3(SYS_DEV_BDVD,true);
		sSystemFileDevice[DRIVE_TYPE_HDD] = x_CreateFileDevicePS3(SYS_DEV_HDD0,true);

		xfilesystem::xdevicealias host ("host" , sSystemFileDevice[DRIVE_TYPE_HOST], SYS_APP_HOME"/");
		xfilesystem::xdevicealias dvd  ("dvd"  , sSystemFileDevice[DRIVE_TYPE_DVD], SYS_DEV_BDVD"/PS3_GAME/USRDIR/");
		xfilesystem::xdevicealias hdd0 ("hdd"  , sSystemFileDevice[DRIVE_TYPE_HDD], SYS_DEV_HDD0"/game/TEST12345/USRDIR/");		//[hj]: hack directory

		xfilesystem::xdevicealias::sRegister(host);
		xfilesystem::xdevicealias::sRegister(dvd);
		xfilesystem::xdevicealias::sRegister(hdd0);

		xfilesystem::xdevicealias appdir( "appdir", "host" );
		xfilesystem::xdevicealias curdir( "curdir", "host" );

		xfilesystem::xdevicealias::sRegister(appdir);
		xfilesystem::xdevicealias::sRegister(curdir);
		
		xfilesystem::initialise(max_open_streams);
		xfilesystem::xfs_common::s_instance()->setIoThreadInterface(io_thread);
	
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
				x_DestroyFileDevicePS3(device);
				sSystemFileDevice[i] = NULL;
			}
		}
		xfilesystem::setAllocator(NULL);
	}

};

#endif // TARGET_MAC
