#ifndef __X_FILESYSTEM_UTILS_H__
#define __X_FILESYSTEM_UTILS_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

#include "xbase/x_runes.h"

namespace xcore
{
    void        fixSlashes(utf16::runes& str, uchar32 old_slash, uchar32 new_slash);
    void		setOrReplaceDeviceName(utf16::runes& ioStr, utf16::runes const& inDeviceName) const;
    void		setOrReplaceDevicePart(utf16::runes& ioStr, utf16::runes const& inDeviceName) const;
}


#endif