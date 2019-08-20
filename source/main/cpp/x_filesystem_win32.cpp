#include "xbase/x_target.h"
#ifdef TARGET_PC

#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOKANJI
#include <windows.h>
#include <stdio.h>

#include "xbase/x_debug.h"
#include "xbase/x_string_std.h"
#include "xbase/x_va_list.h"

#include "xtime/x_datetime.h"

#include "xfilesystem/private/x_filesystem_common.h"
#include "xfilesystem/private/x_filesystem_win32.h"

#include "xfilesystem/x_filesystem.h"
#include "xfilesystem/x_filedevice.h"
#include "xfilesystem/private/x_filedata.h"
#include "xfilesystem/private/x_fileasync.h"

namespace xcore
{
    namespace xfilesystem
    {
        //---------
        // Forward declares
        //---------

        //---------------
        // Public Methods
        //---------------

        //------------------------------------------------------------------------------------------

        void getOpenCreatedTime(u32 uHandle, xdatetime& pTimeAndDate)
        {
            // ASSERTS (pTimeAndDate, "GetOpenCreatedTime() : Pointer to xsystem::TimeAndDate is NULL!");

            // CellFsStat	xStat;
            // CellFsErrno eError = cellFsFstat(m_OpenAsyncFile[uHandle].m_nFileHandle, &xStat);

            pTimeAndDate = xdatetime::sFromFileTime(0);
        }

        //------------------------------------------------------------------------------------------

        void getOpenModifiedTime(u32 uHandle, xdatetime& pTimeAndDate)
        {
            // ASSERTS (pTimeAndDate, "GetOpenModifiedTime() : Pointer to xsystem::TimeAndDate is NULL!");

            // CellFsStat	xStat;
            // CellFsErrno eError = cellFsFstat(m_OpenAsyncFile[uHandle].m_nFileHandle, &xStat);

            pTimeAndDate = xdatetime::sFromFileTime(0);
        }

        //------------------------------------------------------------------------------------------

        u64 getFreeSize(const char* szPath)
        {
            u32 uBlockSize  = 0;
            u64 uBlockCount = 0;
            // 			cellFsGetFreeSize(szPath, &uBlockSize, &uBlockCount);

            return uBlockSize * uBlockCount;
        }

        //------------------------------------------------------------------------------------------

        void initialise(u32 uMaxOpenStreams)
        {
            xfs_common::s_create();
            xfs_common::s_instance()->initialiseCommon(uMaxOpenStreams);
        }

        //------------------------------------------------------------------------------------------

        void shutdown(void)
        {
            xfs_common::s_instance()->shutdownCommon();
            xfs_common::s_destroy();
        }

        //------------------------------------------------------------------------------------------

        xbool isPathUNIXStyle(void) { return false; }

    }; // namespace xfilesystem

}; // namespace xcore

#endif // TARGET_PC
