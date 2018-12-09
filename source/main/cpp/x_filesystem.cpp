#include "xbase/x_target.h"
#include "xbase/x_allocator.h"
#include "xbase/x_debug.h"
#include "xbase/x_runes.h"

#include "xstring/x_string.h"

#include "xfilesystem/x_filesystem.h"
#include "xfilesystem/x_filepath.h"
#include "xfilesystem/x_dirpath.h"

#include "xfilesystem/private/x_enumerations.h"
#include "xfilesystem/private/x_filedevice.h"

namespace xcore
{
    // This class should be in "private/x_filesystem.h" so that xfilepath, xdirpath etc.. can
    // use the private details of xfilesystem.

    class _xfilesystem_
    {
    public:
        char          m_slash;
        xalloc*       m_allocator;
        utf32::alloc* m_stralloc;

        xfilepath m_filepath; // Clone root
        xdirpath  m_dirpath;

        xfilepath resolve(xfilepath const&) const;
        xdirpath  resolve(xdirpath const&) const;
    };

}; // namespace xcore
