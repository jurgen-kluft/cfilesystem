#include "ccore/c_target.h"
#ifdef TARGET_MAC

#include "ccore/c_debug.h"
#include "cbase/c_runes.h"
#include "cbase/c_va_list.h"

#include "ctime/c_datetime.h"

#include "cfilesystem/private/c_filesystem.h"

#include "cfilesystem/c_filesystem.h"
#include "cfilesystem/private/c_filedevice.h"

namespace ncore
{
    //------------------------------------------------------------------------------------------

    bool isPathUNIXStyle(void) { return true; }

    void doIO(io_thread_t* io_thread) {}

}; // namespace ncore

#endif // TARGET_PC
