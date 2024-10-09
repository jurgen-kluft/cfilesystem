#include "ccore/c_target.h"
#ifdef TARGET_PC

#    define WIN32_LEAN_AND_MEAN
#    define NOGDI
#    define NOKANJI
#    include <windows.h>
#    include <stdio.h>

#    include "ccore/c_debug.h"
#    include "cbase/c_runes.h"
#    include "cbase/c_va_list.h"

#    include "ctime/c_datetime.h"

#    include "cfilesystem/private/c_filesystem.h"

#    include "cfilesystem/c_filesystem.h"
#    include "cfilesystem/private/c_filedevice.h"

namespace ncore
{
    namespace nfs
    {
        bool isPathUNIXStyle(void) { return false; }
        void doIO(io_thread_t* io_thread) {}
    } // namespace nfs
}; // namespace ncore

#endif // TARGET_PC
