#ifndef __C_FILESYSTEM_ENUMERATOR_H__
#define __C_FILESYSTEM_ENUMERATOR_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

namespace ncore
{
    class filepath_t;
    class dirpath_t;

    namespace nfs
    {
        class fileattrs_t;
        class filetimes_t;

        class enumerate_delegate_t
        {
        public:
            // When you receive a 'dirinfo' and return true you indicate that you want to
            // recurse into that directory. When returning 'false' you indicate to not
            // want to recurse into that directory.
            // When you receive a 'fileinfo' and return false you indicate that you want to
            // terminate the iteration.
            virtual bool operator()(s32 depth, filepath_t const& fi, fileattrs_t const& fa, filetimes_t const& ft) = 0;
            virtual bool operator()(s32 depth, dirpath_t const& di)                                                = 0;
        };
    } // namespace nfs
}; // namespace ncore

#endif
