#include "xbase/x_target.h"
#ifdef TARGET_PC

#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOKANJI
#include <windows.h>
#include <stdio.h>

#include "xbase/x_debug.h"
#include "xbase/x_va_list.h"

#include "xfilesystem/private/x_filedevice.h"
#include "xfilesystem/private/x_filesystem.h"
#include "xfilesystem/private/x_devicemanager.h"

#include "xfilesystem/x_filesystem.h"
#include "xfilesystem/x_filepath.h"

namespace xcore
{
    namespace
    {
        // Register all system devices for windows.
        enum EDriveTypes
        {
            DRIVE_TYPE_UNKNOWN     = 0,
            DRIVE_TYPE_NO_ROOT_DIR = 1,
            DRIVE_TYPE_REMOVABLE   = 2,
            DRIVE_TYPE_FIXED       = 3,
            DRIVE_TYPE_REMOTE      = 4,
            DRIVE_TYPE_CDROM       = 5,
            DRIVE_TYPE_RAMDISK     = 6,
            DRIVE_TYPE_NUM         = 7,
        };

        static xfiledevice* sFileDevices[DRIVE_TYPE_NUM] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL};

        static const wchar_t* sSystemDeviceLetters[] = {L"a", L"b", L"c", L"d", L"e", L"f", L"g", L"h", L"i", L"j", L"k", L"l", L"m",
                                                        L"n", L"o", L"p", L"q", L"r", L"s", L"t", L"u", L"v", L"w", L"x", L"y", L"z"};
        static const wchar_t* sSystemDevicePaths[]   = {L"a:\\", L"b:\\", L"c:\\", L"d:\\", L"e:\\", L"f:\\", L"g:\\", L"h:\\", L"i:\\", L"j:\\", L"k:\\", L"l:\\", L"m:\\",
                                                      L"n:\\", L"o:\\", L"p:\\", L"q:\\", L"r:\\", L"s:\\", L"t:\\", L"u:\\", L"v:\\", L"w:\\", L"x:\\", L"y:\\", L"z:\\"};

        static void x_FileSystemRegisterSystemAliases(xdevicemanager* devman)
        {
            utf32::runez<255> string32;

            // Get all logical drives.
            DWORD drives   = GetLogicalDrives();
            s32   driveIdx = 0;
            while (drives)
            {
                if (drives & 1)
                {
                    const wchar_t* driveLetter = sSystemDeviceLetters[driveIdx];
                    const wchar_t* devicePath  = sSystemDevicePaths[driveIdx];

                    xbool       boCanWrite      = true;
                    EDriveTypes eDriveType      = DRIVE_TYPE_UNKNOWN;
                    const u32   uDriveTypeWin32 = 1 << (GetDriveTypeW(devicePath));
                    if (uDriveTypeWin32 & (1 << DRIVE_TYPE_REMOVABLE))
                    {
                        eDriveType = DRIVE_TYPE_REMOVABLE;
                    }
                    else if (uDriveTypeWin32 & (1 << DRIVE_TYPE_CDROM))
                    {
                        eDriveType = DRIVE_TYPE_CDROM;
                        boCanWrite = false;
                    }
                    else if (uDriveTypeWin32 & (1 << DRIVE_TYPE_REMOTE))
                    {
                        eDriveType = DRIVE_TYPE_REMOTE;
                    }
                    else if (uDriveTypeWin32 & (1 << DRIVE_TYPE_FIXED))
                    {
                        eDriveType = DRIVE_TYPE_FIXED;
                    }

                    // Convert driveLetter (Ascii) to utf-32
					string32.reset();
                    utf16::crunes driveLetter16((utf16::pcrune)driveLetter);
                    utf::copy(driveLetter16, string32);

					xpath devicename;
					devicename.m_path = string32;
                    if (devman->has_device(devicename))
                    {
                        if (sFileDevices[eDriveType] == NULL)
                        {
                            utf32::runes  devicePath32(string32);
                            utf16::crunes devicePath16((utf16::pcrune)devicePath);
                            utf::copy(devicePath16, devicePath32);
                            sFileDevices[eDriveType] = x_CreateFileDevice(utf32::crunes(devicePath32), boCanWrite);
                        }
                        xfiledevice* device = sFileDevices[eDriveType];

                        wchar_t local_alias[255];
                        local_alias[0] = '\0';
                        DWORD ret_val  = ::QueryDosDeviceW(driveLetter, local_alias, sizeof(local_alias));

                        utf32::runes  local_alias32(string32);
                        utf16::crunes local_alias16((utf16::pcrune)local_alias);
                        utf::copy(local_alias16, local_alias32);

                        utf32::runez<3> wincrap("\\??\\");
                        utf32::runes    wincrapsel = utf32::find(local_alias32, wincrap);

                        utf32::runez<255> string32b;
                        utf32::runes      devicePath32(string32b);
                        utf16::crunes     devicePath16((utf16::pcrune)devicePath);
                        utf::copy(devicePath16, devicePath32);

                        if (ret_val != 0 && !wincrapsel.is_empty())
                        {
                            // Remove windows text crap.
                            utf32::runes alias32 = utf32::selectUntilEndExcludeSelection(local_alias32, wincrapsel);
                            if (alias32.size() > 0 && alias32.m_end[-1] != '\\')
                            {
                                alias32.m_end[0] = '\\';
                                alias32.m_end[1] = '\0';
                                alias32.m_end++;
                            }

                            devman->add_alias(alias32, devicePath32);
                            devman->add_device(devicePath32, device);
                        }
                        else
                        {
                            // Register system device.
                            devman->add_device(devicePath32, device);
                        }
                    }
                }
                drives >>= 1;
                driveIdx++;
            }
        }

        //------------------------------------------------------------------------------
        // Author:
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
        class fs_utfalloc : public utf32::alloc
        {
            xalloc* m_allocator;

        public:
            fs_utfalloc(xalloc* _allocator)
                : m_allocator(_allocator)
            {
            }

            virtual utf32::runes allocate(s32 len, s32 cap)
            {
                if (len > cap)
                    cap = len;
                utf32::runes str;
                str.m_str      = (utf32::rune*)m_allocator->allocate((cap + 1) * sizeof(utf32::rune), sizeof(void*));
                str.m_end      = str.m_str + len;
                str.m_eos      = str.m_str + cap;
                str.m_str[cap] = '\0';
                str.m_str[len] = '\0';
                return str;
            }

            virtual void deallocate(utf32::runes& slice)
            {
                m_allocator->deallocate(slice.m_str);
                slice = utf32::runes();
            }

			XCORE_CLASS_PLACEMENT_NEW_DELETE
        };
	}

    void xfilesystem::create(xfilesyscfg const& cfg)
    {
        xdevicemanager* devman = cfg.m_allocator->construct<xdevicemanager>();
        x_FileSystemRegisterSystemAliases(devman);

        utf32::rune adir32[512] = {'\0'};

        // Get the application directory (by removing the executable filename)
        wchar_t dir[512] = {'\0'}; // Needs to end with a backslash!
        DWORD   result   = ::GetModuleFileNameW(0, dir, sizeof(dir) - 1);
        if (result != 0)
        {
            utf32::runes dir32(adir32, adir32, adir32 + sizeof(adir32) - 1);
            utf::copy(utf16::crunes((utf16::pcrune)dir), dir32);
            utf32::runes appdir = utf32::findLastSelectUntilIncluded(dir32, '\\');
            devman->add_alias("appdir", appdir);
        }

        // Get the working directory
        result = ::GetCurrentDirectoryW(sizeof(dir) - 1, dir);
        if (result != 0)
        {
            utf32::runes dir32(adir32, adir32, adir32 + sizeof(adir32) - 1);
            utf::copy(utf16::crunes((utf16::pcrune)dir), dir32);
            utf32::runes curdir = utf32::findLastSelectUntilIncluded(dir32, '\\');
            devman->add_alias("curdir", curdir);
        }

        xfilesys* imp    = cfg.m_allocator->construct<xfilesys>();
        imp->m_slash     = cfg.m_default_slash;
        imp->m_allocator = cfg.m_allocator;
        imp->m_stralloc  = cfg.m_allocator->construct<fs_utfalloc>(cfg.m_allocator);
        imp->m_devman    = devman;

		xfilesystem::mImpl = imp;
    }

    //------------------------------------------------------------------------------
    // Summary:
    //     Terminate the filesystem.
    // Arguments:
    //     void
    // Returns:
    //     void
    // Description:
    //------------------------------------------------------------------------------
    void xfilesystem::destroy()
    {
        mImpl->m_devman->exit();

        mImpl->m_allocator->destruct(mImpl->m_stralloc);
        mImpl->m_allocator->destruct(mImpl->m_devman);
        mImpl->m_allocator->destruct(mImpl);
        mImpl = nullptr;
    }

};    // namespace xcore

#endif // TARGET_PC