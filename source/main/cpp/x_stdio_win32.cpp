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

#include "xstring\x_string.h"

#include "xfilesystem\x_filesystem.h"
#include "xfilesystem\private\x_filesystem_common.h"

//==============================================================================
// xCore namespace
//==============================================================================
namespace xcore
{

	//------------------------------------------------------------------------------

	// Register all system devices for windows.
	enum EDriveTypes
	{
		DRIVE_TYPE_UNKNOWN					= 0,
		DRIVE_TYPE_NO_ROOT_DIR				= 1,
		DRIVE_TYPE_REMOVABLE				= 2,
		DRIVE_TYPE_FIXED					= 3,
		DRIVE_TYPE_REMOTE					= 4,
		DRIVE_TYPE_CDROM					= 5,
		DRIVE_TYPE_RAMDISK					= 6
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
				xstring_tmp drive_name("%s%s", x_va_list(driveLetter, ":\\"));

				xfilesystem::ESourceType eSource = xfilesystem::FS_SOURCE_UNDEFINED;
				const u32 type = 1 << (GetDriveTypeA(drive_name));
				if (type & (1<<DRIVE_TYPE_REMOVABLE))
				{
					eSource = xfilesystem::FS_SOURCE_MS;
				}
				else if (type & (1<<DRIVE_TYPE_CDROM))
				{
					eSource = xfilesystem::FS_SOURCE_DVD;
				}
				else if (type & (1<<DRIVE_TYPE_REMOTE))
				{
					eSource = xfilesystem::FS_SOURCE_REMOTE;
				}
				else if (type & (1<<DRIVE_TYPE_FIXED))
				{
					eSource = xfilesystem::FS_SOURCE_HDD;
				}
					
				if (eSource != xfilesystem::FS_SOURCE_UNDEFINED)
				{
					if (xfilesystem::FindAlias(driveLetter) == NULL)
					{
						XSTRING_BUFFER(local_alias, 1024);
						DWORD ret_val = QueryDosDeviceA(driveLetter, local_aliasBuffer, sizeof(local_aliasBuffer));
						if (ret_val!=0 && local_alias.find("\\??\\")==0)
						{
							// Remove windows text crap.
							local_alias.remove(0, 4);
							if (local_alias.lastChar() != '\\')
								local_alias += '\\';

							xfilesystem::AddAlias(xfilesystem::xalias(driveLetter, eSource, sSystemDevicePaths[driveIdx]));
						}
						else
						{

							// Register system device.
							xfilesystem::AddAlias(xfilesystem::xalias(driveLetter, eSource, sSystemDevicePaths[driveIdx]));
						}
					}
				}
			}
			drives>>= 1;
			driveIdx++;
		}
	}

	//------------------------------------------------------------------------------
	// Author:
	//     Tomas Arce
	// Summary:
	//     Initialize the stdio system
	// Arguments:
	//     void
	// Returns:
	//     void
	// Description:
	// See Also:
	//      x_StdioExit()
	//------------------------------------------------------------------------------
	static char sAppDir[1024] = { '\0' };										///< Needs to end with a backslash!
	static char sWorkDir[1024] = { '\0' };										///< Needs to end with a backslash!
	void x_StdioInit(void)
	{
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
		const xfilesystem::xalias* workDirAlias = xfilesystem::FindAliasFromFilename(sAppDir);
		const xfilesystem::xalias* appDirAlias  = xfilesystem::FindAliasFromFilename(sWorkDir);

		// After this, xsystem can initialize the aliases
		xfilesystem::AddAlias(xfilesystem::xalias("appdir", appDirAlias->source(), sAppDir));
		xfilesystem::AddAlias(xfilesystem::xalias("curdir", workDirAlias->source(), sWorkDir));

		xfilesystem::AddAlias(xfilesystem::xalias("host", xfilesystem::FS_SOURCE_HDD, sWorkDir));
		xfilesystem::AddAlias(xfilesystem::xalias("dvd", xfilesystem::FS_SOURCE_DVD, sWorkDir));
		xfilesystem::AddAlias(xfilesystem::xalias("HDD", xfilesystem::FS_SOURCE_MS, sWorkDir));

		xfilesystem::Initialise(64, xTRUE);
	}

	//------------------------------------------------------------------------------
	// Author:
	//     Tomas Arce
	// Summary:
	//     Exit the stdio system.
	// Arguments:
	//     void
	// Returns:
	//     void
	// Description:
	// See Also:
	//      x_StdioInit()
	//------------------------------------------------------------------------------
	void x_StdioExit()
	{
		xfilesystem::Shutdown();
		xfilesystem::ExitAlias();
	}

//==============================================================================
// END xCore namespace
//==============================================================================
};

#endif // TARGET_PC