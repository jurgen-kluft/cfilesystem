#include "ccore/c_target.h"
#include "cbase/c_runes.h"

#ifdef TARGET_MAC

#    define _DARWIN_BETTER_REALPATH
#    include <mach-o/dyld.h>
#    include <limits.h>
#    include <stdlib.h>
#    include <string.h>
#    include <dlfcn.h>

#    include "ccore/c_debug.h"
#    include "cbase/c_va_list.h"

#    include "cfilesystem/private/c_filedevice.h"
#    include "cfilesystem/private/c_filesystem.h"

#    include "cfilesystem/c_filesystem.h"


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

    } // namespace

    namespace nfs
    {
        static filesys_t* sImpl = nullptr;

        void create(context_t const& ctxt)
        {
            filesys_t* imp       = ctxt.m_allocator->construct<filesys_t>();
            imp->m_default_slash = ctxt.m_default_slash;
            imp->m_allocator     = ctxt.m_allocator;
            sImpl = imp;

            //        imp->m_devman = ctxt.m_allocator->construct<devicemanager_t>(imp->m_stralloc);

            // TODO: Register attach devices

            utf32::rune adir32[512] = {'\0'};

            // Mac OS, getting the current working directory
            // TODO: Correct implementation

            // Get the application directory (by removing the executable filename)
            wchar_t dir[512] = {'~', '/', '\0'}; // Needs to end with a backslash!
            s32     result   = 1;
            if (result != 0)
            {
                runes_t  dir32 = make_runes(adir32, 0, 0, (u32)(sizeof(adir32) / sizeof(adir32[0])) - 1);
                crunes_t dir16 = make_crunes((utf16::pcrune)dir);
                nrunes::copy(dir16, dir32);
                runes_t appdir = nrunes::findLastSelectUntilIncluded(dir32, '\\');
                //            imp->m_devman->add_alias("appdir:\\", appdir);
            }

            // Get the working directory
            result = 1;
            // getExecutablePath();
            if (result != 0)
            {
                runes_t  curdir = make_runes(adir32, 0, (u32)(sizeof(adir32) / sizeof(adir32[0])) - 1);
                crunes_t dir16 = make_crunes((utf16::pcrune)dir);
                nrunes::copy(dir16, curdir);
                //            imp->m_devman->add_alias("curdir:\\", curdir);
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
        void destroy()
        {
            //        mImpl->m_devman->exit();
            //        mImpl->m_allocator->destruct(mImpl->m_devman);
            sImpl->m_allocator->destruct(sImpl);
            sImpl = nullptr;
        }
    } // namespace nfs
} // namespace ncore

#endif
