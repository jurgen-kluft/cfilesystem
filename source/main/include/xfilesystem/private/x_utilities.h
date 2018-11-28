#ifndef __X_FILESYSTEM_UTILS_H__
#define __X_FILESYSTEM_UTILS_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif


namespace xcore
{
    class xstring;

    namespace filesystem_utils
    {
        void        fixSlashes(xstring& str, uchar32 old_slash, uchar32 new_slash);
        void		setOrReplaceDeviceName(xstring& ioStr, xstring const& inDeviceName) const;
        void		setOrReplaceDevicePart(xstring& ioStr, xstring const& inDeviceName) const;
    }
}


#endif