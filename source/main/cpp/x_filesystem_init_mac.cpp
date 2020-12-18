#include "xbase/x_target.h"
#include "xbase/x_runes.h"

#ifdef TARGET_MAC

#define _DARWIN_BETTER_REALPATH
#include <mach-o/dyld.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include "xbase/x_debug.h"
#include "xbase/x_va_list.h"

#include "filesystem_t/private/x_filedevice.h"
#include "filesystem_t/private/x_filesystem.h"
#include "filesystem_t/private/x_devicemanager.h"

#include "filesystem_t/x_filesystem.h"
#include "filesystem_t/x_filepath.h"

namespace xcore
{
    namespace
    {
        int getExecutablePath(char* out, int capacity, int* dirname_length)
        {
            char  buffer1[PATH_MAX];
            char  buffer2[PATH_MAX];
            char* path     = buffer1;
            char* resolved = NULL;
            int   length   = -1;

            for (;;)
            {
                uint32_t size = (uint32_t)sizeof(buffer1);
                if (_NSGetExecutablePath(path, &size) == -1)
                {
                    return -1;
                }

                resolved = realpath(path, buffer2);
                if (!resolved)
                    break;

                length = (int)strlen(resolved);
                if (length <= capacity)
                {
                    memcpy(out, resolved, length);

                    if (dirname_length)
                    {
                        int i;

                        for (i = length - 1; i >= 0; --i)
                        {
                            if (out[i] == '/')
                            {
                                *dirname_length = i;
                                break;
                            }
                        }
                    }
                }

                break;
            }

            if (path != buffer1)
                return -1;

            return length;
        }

        //------------------------------------------------------------------------------
        // The string allocator
        class fs_utfalloc : public runes_alloc_t
        {
            alloc_t* m_allocator;

        public:
            fs_utfalloc(alloc_t* _allocator) : m_allocator(_allocator) {}
            ~fs_utfalloc() {}

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

            virtual void deallocate(utf32::runes& slice_t)
            {
                m_allocator->deallocate(slice_t.m_str);
                slice_t = utf32::runes();
            }

            XCORE_CLASS_PLACEMENT_NEW_DELETE
        };

    } // namespace

    void filesystem_t::create(filesyscfg_t const& cfg)
    {
        filesys_t* imp      = cfg.m_allocator->construct<filesys_t>();
        imp->m_slash       = cfg.m_default_slash;
        imp->m_allocator   = cfg.m_allocator;
        imp->m_stralloc    = cfg.m_allocator->construct<fs_utfalloc>(cfg.m_allocator);
        filesystem_t::mImpl = imp;

        imp->m_devman = cfg.m_allocator->construct<devicemanager_t>(imp->m_stralloc);

        // TODO: Register attach devices

        utf32::rune adir32[512] = {'\0'};

        // Mac OS, getting the current working directory
        // TODO: Correct implementation

        // Get the application directory (by removing the executable filename)
        wchar_t dir[512] = {'~', '/', '\0'}; // Needs to end with a backslash!
        s32     result   = 1;
        if (result != 0)
        {
            utf32::runes dir32(adir32, adir32, adir32 + (sizeof(adir32) / sizeof(adir32[0])) - 1);
            utf::copy(utf16::crunes((utf16::pcrune)dir), dir32);
            utf32::runes appdir = utf32::findLastSelectUntilIncluded(dir32, '\\');
            imp->m_devman->add_alias("appdir:\\", appdir);
        }

        // Get the working directory
        result = 1;
        // getExecutablePath();
        if (result != 0)
        {
            utf32::runes curdir(adir32, adir32, adir32 + (sizeof(adir32) / sizeof(adir32[0])) - 1);
            utf::copy(utf16::crunes((utf16::pcrune)dir), curdir);
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

        mImpl->m_allocator->destruct(mImpl->m_stralloc);
        mImpl->m_allocator->destruct(mImpl->m_devman);
        mImpl->m_allocator->destruct(mImpl);
        mImpl = nullptr;
    }
} // namespace xcore

#endif