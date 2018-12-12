#ifndef __X_FILESYSTEM_FILESYSTEM_IMP_H__
#define __X_FILESYSTEM_FILESYSTEM_IMP_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

//==============================================================================
#include "xbase/x_allocator.h"
#include "xbase/x_runes.h"

#include "xfilesystem/x_filepath.h"
#include "xfilesystem/x_dirpath.h"

namespace xcore
{
    class xdatetime;

    class xfiledevice;
    class xfileinfo;
    class xdirinfo;
    struct xfileattrs;
    struct xfiletimes;
    class xstream;

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

#endif