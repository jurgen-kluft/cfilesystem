#ifndef __X_FILESYSTEM_FILESYSTEM_IMP_H__
#define __X_FILESYSTEM_FILESYSTEM_IMP_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

//==============================================================================
#include "xbase/x_allocator.h"
#include "xbase/x_runes.h"

#include "xfilesystem/private/x_filesystemprivate.h"
#include "xfilesystem/x_filepath.h"
#include "xfilesystem/x_dirpath.h"

namespace xcore
{
    class xdatetime;

    class xfiledevice;
	class xdevicemanager;
    class xfileinfo;
    class xdirinfo;
    struct xfileattrs;
    struct xfiletimes;
    class xstream;

    class _xfilesystem_ : public xfilesystemprivate
    {
    public:
        char            m_slash;
        xalloc*         m_allocator;
        utf32::alloc*   m_stralloc;
        xdevicemanager* m_devman;

        xfilepath resolve(xfilepath const&, xfiledevice*& device) const;
        xdirpath  resolve(xdirpath const&, xfiledevice*& device) const;

        static xfilesystem* create_fs(_xfilesystem_* _fs_);
    };

}; // namespace xcore

#endif