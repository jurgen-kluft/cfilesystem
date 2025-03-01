#include "ccore/c_target.h"
#ifdef TARGET_PC

#    define WIN32_LEAN_AND_MEAN
#    define NOGDI
#    define NOKANJI
#    include <windows.h>
#    include <stdio.h>

#    include "ccore/c_debug.h"
#    include "cbase/c_va_list.h"

#    include "cfilesystem/private/c_filedevice.h"
#    include "cfilesystem/private/c_filesystem.h"

#    include "cfilesystem/c_filesystem.h"
#    include "cfilesystem/c_filepath.h"

namespace ncore
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

        static filedevice_t* sFileDevices[DRIVE_TYPE_NUM] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};

        static const wchar_t* sSystemDeviceLetters[] = {L"a", L"b", L"c", L"d", L"e", L"f", L"g", L"h", L"i", L"j", L"k", L"l", L"m", L"n", L"o", L"p", L"q", L"r", L"s", L"t", L"u", L"v", L"w", L"x", L"y", L"z"};
        static const wchar_t* sSystemDevicePaths[]   = {L"a:\\", L"b:\\", L"c:\\", L"d:\\", L"e:\\", L"f:\\", L"g:\\", L"h:\\", L"i:\\", L"j:\\", L"k:\\", L"l:\\", L"m:\\",
                                                        L"n:\\", L"o:\\", L"p:\\", L"q:\\", L"r:\\", L"s:\\", L"t:\\", L"u:\\", L"v:\\", L"w:\\", L"x:\\", L"y:\\", L"z:\\"};

        static void gFileSystemDestroyFileDevices(filesys_t* ctxt)
        {
            for (s32 i = 0; i < DRIVE_TYPE_NUM; i++)
            {
                filedevice_t* fd = sFileDevices[i];
                if (fd == nullptr)
                    continue;

                for (s32 j = 0; j < DRIVE_TYPE_NUM; j++)
                {
                    if (sFileDevices[j] == fd)
                        sFileDevices[j] = nullptr;
                }
                x_DestroyFileDevice(fd);
            }
        }

        static void gFileSystemRegisterSystemAliases(filesys_t* ctxt)
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

                    bool        boCanWrite      = true;
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

                    if (!ctxt->has_device(string32))
                    {
                        if (sFileDevices[eDriveType] == nullptr)
                        {
                            sFileDevices[eDriveType] = x_CreateFileDevice(boCanWrite);
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
                        runes_t                 wincrapsel = find(local_alias32, wincrap);

                        runez_t<utf32::rune, 255> string32b;
                        runes_t                   devicePath32(string32b);
                        crunes_t                  devicePath16((utf16::pcrune)devicePath);
                        copy(devicePath16, devicePath32);

                        if (ret_val != 0 && !wincrapsel.is_empty())
                        {
                            // Remove windows text crap.
                            runes_t alias32 = selectAfterExclude(local_alias32, wincrapsel);
                            if (alias32.size() > 0 && last_char(alias32) != '\\')
                            {
                                concatenate(alias32, crunes_t("\\"));
                            }

                            ctxt->register_alias(alias32, devicePath32);
                            ctxt->register_device(devicePath32, device);
                        }
                        else
                        {
                            // Register system device.
                            ctxt->register_device(devicePath32, device);
                        }
                    }
                }
                drives >>= 1;
                driveIdx++;
            }
        }

    } // namespace

    namespace nfs
    {

        //------------------------------------------------------------------------------
        void filesystem_t::create(filesystem_t::context_t const& ctxt)
        {
            filesys_t* root          = ctxt.m_allocator->construct<filesys_t>();
            root->m_allocator        = ctxt.m_allocator;
            root->m_default_slash    = ctxt.m_default_slash;
            root->m_max_open_files   = ctxt.m_max_open_files;
            root->m_max_path_objects = ctxt.m_max_path_objects;
            filesystem_t::mImpl      = root;

            root->init(ctxt.m_allocator);

            gFileSystemRegisterSystemAliases(root);

            utf32::rune adir32[512] = {'\0'};

            // Get the application directory (by removing the executable filename)
            wchar_t dir[512] = {'\0'}; // Needs to end with a backslash!
            DWORD   result   = ::GetModuleFileNameW(0, dir, sizeof(dir) - 1);
            if (result != 0)
            {
                runes_t  dir32(adir32, adir32, adir32 + (sizeof(adir32) / sizeof(adir32[0])) - 1);
                crunes_t dir16((utf16::pcrune)dir);
                ncore::copy(dir16, dir32);
                runes_t appdir = findLastSelectUntilIncluded(dir32, '\\');
                root->register_alias("appdir:\\", appdir);
            }

            // Get the working directory
            result = ::GetCurrentDirectoryW(sizeof(dir) - 1, dir);
            if (result != 0)
            {
                runes_t  curdir(adir32, adir32, adir32 + (sizeof(adir32) / sizeof(adir32[0])) - 1);
                crunes_t dir16((utf16::pcrune)dir);
                ncore::copy(dir16, curdir);
                root->register_alias("curdir:\\", curdir);
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
            gFileSystemDestroyFileDevices(mImpl);
            mImpl->exit(mImpl->m_allocator);
            mImpl->m_allocator->destruct(mImpl->m_stralloc);
            mImpl->m_allocator->destruct(mImpl);
            mImpl = nullptr;
        }
    } // namespace nfs
}; // namespace ncore

#endif // TARGET_PC
