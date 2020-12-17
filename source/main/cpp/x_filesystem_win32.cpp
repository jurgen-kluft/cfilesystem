#include "xbase/x_target.h"
#ifdef TARGET_PC

#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOKANJI
#include <windows.h>
#include <stdio.h>

#include "xbase/x_debug.h"
#include "xbase/x_runes.h"
#include "xbase/va_list_t.h"

#include "xtime/x_datetime.h"

#include "filesystem_t/private/x_filesystem.h"

#include "filesystem_t/x_filesystem.h"
#include "filesystem_t/private/x_filedevice.h"

namespace xcore
{
    //------------------------------------------------------------------------------------------

    bool isPathUNIXStyle(void) { return false; }

    void doIO(io_thread_t* io_thread) {}

}; // namespace xcore

#endif // TARGET_PC
