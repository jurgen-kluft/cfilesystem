#include "cbase/c_target.h"
#include "cbase/c_runes.h"

#ifdef TARGET_MAC

#define _DARWIN_BETTER_REALPATH
#include <mach-o/dyld.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include "cbase/c_debug.h"
#include "cbase/c_va_list.h"

#include "cfilesystem/private/c_filedevice.h"
#include "cfilesystem/private/c_filesystem.h"
#include "cfilesystem/private/c_devicemanager.h"

#include "cfilesystem/c_filesystem.h"
#include "cfilesystem/c_filepath.h"

namespace ncore
{
    namespace
    {
        int getExecutablePath(char* out, int capacity, int* dirname_length)
        {
            char  buffer1[PATH_MAX];
            char  buffer2[PATH_MAX];
            char* path     = buffer1;
            char* resolved = nullptr;
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

            virtual runes_t allocate(s32 len, s32 cap, s32 type)
            {
                if (len > cap)
                    cap = len;
                runes_t str;
                str.m_utf32.m_bos      = (utf32::rune*)m_allocator->allocate((cap + 1) * sizeof(utf32::rune), sizeof(void*));
                str.m_utf32.m_str      = str.m_utf32.m_bos;
                str.m_utf32.m_end      = str.m_utf32.m_str + len;
                str.m_utf32.m_eos      = str.m_utf32.m_str + cap;
                str.m_utf32.m_str[cap] = '\0';
                str.m_utf32.m_str[len] = '\0';
                return str;
            }

            virtual void deallocate(runes_t& r)
            {
                m_allocator->deallocate(r.m_utf32.m_bos);
                r = runes_t();
            }

            DCORE_CLASS_PLACEMENT_NEW_DELETE
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
            runes_t dir32(adir32, adir32, adir32 + (sizeof(adir32) / sizeof(adir32[0])) - 1);
            crunes_t dir16((utf16::pcrune)dir);
            ncore::copy(dir16, dir32);
            runes_t appdir = findLastSelectUntilIncluded(dir32, '\\');
            imp->m_devman->add_alias("appdir:\\", appdir);
        }

        // Get the working directory
        result = 1;
        // getExecutablePath();
        if (result != 0)
        {
            runes_t curdir(adir32, adir32, adir32 + (sizeof(adir32) / sizeof(adir32[0])) - 1);
            crunes_t dir16((utf16::pcrune)dir);
            ncore::copy(dir16, curdir);
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
} // namespace ncore

#endif