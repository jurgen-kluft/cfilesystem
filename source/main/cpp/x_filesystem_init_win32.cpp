//==============================================================================
// INCLUDES
//==============================================================================
#include "xbase\x_target.h"
#ifdef TARGET_PC

#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOKANJI
#include <windows.h>
#include <stdio.h>

#include "xbase\x_debug.h"
#include "xbase\x_string_std.h"
#include "xbase\x_va_list.h"

#include "xfilesystem\x_filedevice.h"
#include "xfilesystem\private\x_filesystem_common.h"
#include "xfilesystem\private\x_filesystem_win32.h"
#include "xfilesystem\private\x_devicealias.h"

#include "xfilesystem\x_filesystem.h"
#include "xfilesystem\x_filepath.h"

//==============================================================================
// xcore namespace
//==============================================================================
namespace xcore
{
	namespace xfilesystem
	{
		// Register all system devices for windows.
		enum EDriveTypes
		{
			DRIVE_TYPE_UNKNOWN					= 0,
			DRIVE_TYPE_NO_ROOT_DIR				= 1,
			DRIVE_TYPE_REMOVABLE				= 2,
			DRIVE_TYPE_FIXED					= 3,
			DRIVE_TYPE_REMOTE					= 4,
			DRIVE_TYPE_CDROM					= 5,
			DRIVE_TYPE_RAMDISK					= 6,
			DRIVE_TYPE_NUM						= 7,
		};

		static xfiledevice*	sFileDevices[DRIVE_TYPE_NUM] =
		{
			NULL, 
			NULL, 
			NULL, 
			NULL, 
			NULL, 
			NULL, 
			NULL
		};

		static const char* sSystemDeviceLetters[] =
		{
			"a","b","c","d","e","f","g","h","i","j","k","l","m","n","o","p","q","r","s","t","u","v","w","x","y","z"
		};
		static const char* sSystemDevicePaths[] =
		{
			"a:\\","b:\\","c:\\","d:\\","e:\\","f:\\","g:\\","h:\\","i:\\","j:\\","k:\\","l:\\","m:\\","n:\\","o:\\","p:\\","q:\\","r:\\","s:\\","t:\\","u:\\","v:\\","w:\\","x:\\","y:\\","z:\\"
		};

		static void x_FileSystemRegisterSystemAliases()
		{
			// Get all logical drives.
			DWORD		drives	= GetLogicalDrives();
			s32			driveIdx = 0;
			while (drives)
			{
				if (drives&1)
				{
					const char*	driveLetter = sSystemDeviceLetters[driveIdx];

					char drive_name[64];
					drive_name[0] = '\0';
					x_sprintf(drive_name, sizeof(drive_name)-1, "%s%s", x_va(driveLetter), x_va(":\\"));

					xbool boCanWrite = true;
					EDriveTypes eDriveType = DRIVE_TYPE_UNKNOWN;
					const u32 uDriveTypeWin32 = 1 << (GetDriveTypeA(drive_name));
					if (uDriveTypeWin32 & (1<<DRIVE_TYPE_REMOVABLE))
					{
						eDriveType = DRIVE_TYPE_REMOVABLE;
					}
					else if (uDriveTypeWin32 & (1<<DRIVE_TYPE_CDROM))
					{
						eDriveType = DRIVE_TYPE_CDROM;
						boCanWrite = false;
					}
					else if (uDriveTypeWin32 & (1<<DRIVE_TYPE_REMOTE))
					{
						eDriveType = DRIVE_TYPE_REMOTE;
					}
					else if (uDriveTypeWin32 & (1<<DRIVE_TYPE_FIXED))
					{
						eDriveType = DRIVE_TYPE_FIXED;
					}

					if (xdevicealias::sFind(driveLetter) == NULL)
					{
						if (sFileDevices[eDriveType]==NULL)
							sFileDevices[eDriveType] = x_CreateFileDevicePC(sSystemDevicePaths[driveIdx], boCanWrite);
						xfiledevice* device = sFileDevices[eDriveType];

						char local_alias[1024];
						local_alias[0] = '\0';
						DWORD ret_val = ::QueryDosDeviceA(driveLetter, local_alias, sizeof(local_alias));
						if (ret_val!=0 && x_strFind(local_alias, "\\??\\")!=0)
						{
							// Remove windows text crap.
							char* alias = &local_alias[4];
							s32 alias_len = x_strlen(alias);
							if (alias_len>0 && alias[alias_len-1] != '\\')
							{
								alias[alias_len  ] = '\\';
								alias[alias_len+1] = '\0';
								alias_len++;
							}

							xdevicealias::sRegister(xfilesystem::xdevicealias(driveLetter, device, sSystemDevicePaths[driveIdx]));
						}
						else
						{
							// Register system device.
							xdevicealias::sRegister(xfilesystem::xdevicealias(driveLetter, device, sSystemDevicePaths[driveIdx]));
						}
					}

				}
				drives>>= 1;
				driveIdx++;
			}
		}

		//------------------------------------------------------------------------------
		// Author:
		//     Virtuos
		// Summary:
		//     Initialize the filesystem
		// Arguments:
		//     void
		// Returns:
		//     void
		// Description:
		// See Also:
		//      xfilesystem::exit()
		//------------------------------------------------------------------------------
		static char sAppDir[1024] = { '\0' };										///< Needs to end with a backslash!
		static char sWorkDir[1024] = { '\0' };										///< Needs to end with a backslash!
		void init(u32 max_open_streams, xio_thread* io_thread, x_iallocator* allocator)
		{
			xfilesystem::setAllocator(allocator);

			xdevicealias::init();


			// Get the application directory (by removing the executable filename)
			::GetModuleFileName(0, sAppDir, sizeof(sAppDir) - 1);
			char* lastBackSlash = sAppDir + x_strlen(sAppDir);
			while (lastBackSlash > sAppDir) { if (*lastBackSlash == '\\') break; lastBackSlash--; }
			if (lastBackSlash > sAppDir)
			{
				++lastBackSlash;													///< Keep the backslash
				*lastBackSlash = '\0';
			}

			// Get the working directory
			::GetCurrentDirectory(sizeof(sWorkDir)-1, sWorkDir);
			if (sWorkDir[x_strlen(sWorkDir)-1] != '\\')								///< Make sure the path ends with a backslash
			{
				sWorkDir[x_strlen(sWorkDir)] = '\\';
				sWorkDir[x_strlen(sWorkDir)+1] = '\0';
			}

			x_FileSystemRegisterSystemAliases();

			// Determine the source type of the app and work dir
			const xfilesystem::xdevicealias* workDirAlias = xdevicealias::sFind(xfilepath(sWorkDir));
			const xfilesystem::xdevicealias* appDirAlias  = xdevicealias::sFind(xfilepath(sAppDir));

			// After this, user can initialize the aliases
			xdevicealias::sRegister(xfilesystem::xdevicealias("appdir", appDirAlias->device(), sAppDir));
			xdevicealias::sRegister(xfilesystem::xdevicealias("curdir", workDirAlias->device(), sWorkDir));

			xdevicealias::sRegister(xfilesystem::xdevicealias("host", sFileDevices[DRIVE_TYPE_FIXED], ""));
			xdevicealias::sRegister(xfilesystem::xdevicealias("dvd", sFileDevices[DRIVE_TYPE_CDROM], ""));
			xdevicealias::sRegister(xfilesystem::xdevicealias("hdd", sFileDevices[DRIVE_TYPE_FIXED], ""));


			xfilesystem::initialise(max_open_streams);

			xfilesystem::xfs_common::s_instance()->setIoThreadInterface(io_thread);
		}

		//------------------------------------------------------------------------------
		// Author:
		//     Virtuos
		// Summary:
		//     Exit the filesystem.
		// Arguments:
		//     void
		// Returns:
		//     void
		// Description:
		// See Also:
		//      xfilesystem::init()
		//------------------------------------------------------------------------------
		void exit()
		{

			xfilesystem::shutdown();
			xdevicealias::exit();

			for (s32 i=0; i<DRIVE_TYPE_NUM; ++i)
			{
				xfiledevice* device = sFileDevices[i];
				if (device != NULL)
				{
					x_DestroyFileDevicePC(device);
					sFileDevices[i] = NULL;
				}
			}


			xfilesystem::setAllocator(NULL);
		}
	}

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

#endif // TARGET_PC