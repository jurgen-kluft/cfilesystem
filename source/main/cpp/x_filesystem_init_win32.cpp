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

        static filedevice_t* sFileDevices[DRIVE_TYPE_NUM] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL};

        static const wchar_t* sSystemDeviceLetters[] = {L"a", L"b", L"c", L"d", L"e", L"f", L"g", L"h", L"i", L"j", L"k", L"l", L"m", L"n", L"o", L"p", L"q", L"r", L"s", L"t", L"u", L"v", L"w", L"x", L"y", L"z"};
        static const wchar_t* sSystemDevicePaths[]   = {L"a:\\", L"b:\\", L"c:\\", L"d:\\", L"e:\\", L"f:\\", L"g:\\", L"h:\\", L"i:\\", L"j:\\", L"k:\\", L"l:\\", L"m:\\",
                                                      L"n:\\", L"o:\\", L"p:\\", L"q:\\", L"r:\\", L"s:\\", L"t:\\", L"u:\\", L"v:\\", L"w:\\", L"x:\\", L"y:\\", L"z:\\"};

        static void x_FileSystemRegisterSystemAliases(alloc_t* allocator, devicemanager_t* devman)
        {
            runez_t<utf32::rune, 255> string32;

            // Get all logical drives.
            DWORD drives   = GetLogicalDrives();
            s32   driveIdx = 0;
            while (drives)
            {
                if (drives & 1)
                {
                    const wchar_t* driveLetter = sSystemDeviceLetters[driveIdx];
                    const wchar_t* devicePath  = sSystemDevicePaths[driveIdx];

                    bool       boCanWrite      = true;
                    EDriveTypes eDriveType      = DRIVE_TYPE_UNKNOWN;
                    const u32   uDriveTypeWin32 = 1 << (GetDriveTypeW(devicePath));
                    if (uDriveTypeWin32 & (1 << (s32)DRIVE_TYPE_REMOVABLE))
                    {
                        eDriveType = DRIVE_TYPE_REMOVABLE;
                    }
                    else if (uDriveTypeWin32 & (1 << (s32)DRIVE_TYPE_CDROM))
                    {
                        eDriveType = DRIVE_TYPE_CDROM;
                        boCanWrite = false;
                    }
                    else if (uDriveTypeWin32 & (1 << (s32)DRIVE_TYPE_REMOTE))
                    {
                        eDriveType = DRIVE_TYPE_REMOTE;
                    }
                    else if (uDriveTypeWin32 & (1 << (s32)DRIVE_TYPE_FIXED))
                    {
                        eDriveType = DRIVE_TYPE_FIXED;
                    }

                    // Convert drivePath (Ascii) to utf-32
                    string32.reset();
                    crunes_t devicePath16((utf16::pcrune)devicePath);
                    copy(devicePath16, string32);

                    path_t devicename;
                    devicename.m_path = string32;
                    if (!devman->has_device(devicename))
                    {
                        if (sFileDevices[eDriveType] == NULL)
                        {
                            string32.reset();
                            runes_t  devicePath32(string32);
                            crunes_t devicePath16((utf16::pcrune)devicePath);
                            copy(devicePath16, devicePath32);
                            sFileDevices[eDriveType] = x_CreateFileDevice(allocator, crunes_t(devicePath32), boCanWrite);
                        }
                        filedevice_t* device = sFileDevices[eDriveType];

                        wchar_t local_alias[255];
                        local_alias[0] = '\0';
                        DWORD ret_val  = ::QueryDosDeviceW(driveLetter, local_alias, sizeof(local_alias));

                        string32.reset();
                        runes_t  local_alias32(string32);
                        crunes_t local_alias16((utf16::pcrune)local_alias);
                        copy(local_alias16, local_alias32);

                        runez_t<ascii::rune, 8> wincrap("\\??\\");
                        runes_t    wincrapsel = find(local_alias32, wincrap);

                        runez_t<utf32::rune, 255> string32b;
                        runes_t      devicePath32(string32b);
                        crunes_t     devicePath16((utf16::pcrune)devicePath);
                        copy(devicePath16, devicePath32);

                        if (ret_val != 0 && !wincrapsel.is_empty())
                        {
                            // Remove windows text crap.
                            runes_t alias32 = selectAfterExclude(local_alias32, wincrapsel);
                            if (alias32.size() > 0 && last_char(alias32) != '\\')
                            {
                                alias32.concatenate('\\');
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
        // The string allocator
        class fs_utfalloc : public runes_alloc_t
        {
            alloc_t* m_allocator;

        public:
            fs_utfalloc(alloc_t* _allocator) : m_allocator(_allocator) {}

            virtual runes_t allocate(s32 len, s32 cap, s32 type)
            {
                if (len > cap)
                    cap = len;
                runes_t str;
                str.m_runes.m_utf32.m_str      = (utf32::rune*)m_allocator->allocate((cap + 1) * sizeof(utf32::rune), sizeof(void*));
                str.m_runes.m_utf32.m_end      = str.m_runes.m_utf32.m_str + len;
                str.m_runes.m_utf32.m_eos      = str.m_runes.m_utf32.m_str + cap;
                str.m_runes.m_utf32.m_str[cap] = '\0';
                str.m_runes.m_utf32.m_str[len] = '\0';
                return str;
            }

            virtual void deallocate(runes_t& slice_t)
            {
                if (slice_t.is_nil())
                    return;
                m_allocator->deallocate(slice_t.m_runes.m_utf32.m_bos);
                slice_t = runes_t();
            }

            XCORE_CLASS_PLACEMENT_NEW_DELETE
        };
    } // namespace

    //------------------------------------------------------------------------------
    void filesystem_t::create(filesystem_t::context_t const& ctxt)
    {
        filesys_t* imp      = ctxt.m_allocator->construct<filesys_t>();
        imp->m_context      = ctxt;
        filesystem_t::mImpl = imp;

        imp->m_devman = ctxt.m_allocator->construct<devicemanager_t>(&imp->m_context);
        x_FileSystemRegisterSystemAliases(ctxt.m_allocator, imp->m_devman);

        utf32::rune adir32[512] = {'\0'};

        // Get the application directory (by removing the executable filename)
        wchar_t dir[512] = {'\0'}; // Needs to end with a backslash!
        DWORD   result   = ::GetModuleFileNameW(0, dir, sizeof(dir) - 1);
        if (result != 0)
        {
            runes_t dir32(adir32, adir32, adir32 + (sizeof(adir32) / sizeof(adir32[0])) - 1);
            crunes_t dir16((utf16::pcrune)dir);
            xcore::copy(dir16, dir32);
            runes_t appdir = findLastSelectUntilIncluded(dir32, '\\');
            imp->m_devman->add_alias("appdir:\\", appdir);
        }

        // Get the working directory
        result = ::GetCurrentDirectoryW(sizeof(dir) - 1, dir);
        if (result != 0)
        {
            runes_t curdir(adir32, adir32, adir32 + (sizeof(adir32) / sizeof(adir32[0])) - 1);
            crunes_t dir16((utf16::pcrune)dir);
            xcore::copy(dir16, curdir);
            imp->m_devman->add_alias("curdir:\\", curdir);
        }
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
    void filesystem_t::destroy()
    {
        mImpl->m_devman->exit();

        mImpl->m_context.m_allocator->destruct(mImpl->m_context.m_stralloc);
        mImpl->m_context.m_allocator->destruct(mImpl->m_devman);
        mImpl->m_context.m_allocator->destruct(mImpl);
        mImpl = nullptr;
    }

}; // namespace xcore

#endif // TARGET_PC