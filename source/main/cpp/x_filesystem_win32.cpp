#include "xbase/x_target.h"
#ifdef TARGET_PC

#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOKANJI
#include <windows.h>
#include <stdio.h>

#include "xbase/x_debug.h"
#include "xbase/x_runes.h"
#include "xbase/x_va_list.h"

#include "xtime/x_datetime.h"

#include "xfilesystem/private/x_filesystem.h"

#include "xfilesystem/x_filesystem.h"
#include "xfilesystem/private/x_filedevice.h"

namespace ncore
{
    //------------------------------------------------------------------------------------------

    bool isPathUNIXStyle(void) { return false; }

    void doIO(io_thread_t* io_thread) {}

}; // namespace ncore

#endif // TARGET_PC
