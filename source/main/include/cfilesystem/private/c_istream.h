#ifndef __CFILESYSTEM_XISTREAM_H__
#define __CFILESYSTEM_XISTREAM_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "cbase/c_stream.h"
#include "cfilesystem/private/c_enumerations.h"

namespace ncore
{
    class alloc_t;
    class filepath_t;

    namespace nfs
    {
        class filedevice_t;
        struct filehandle_t;

        extern void*      open_filestream(alloc_t* a, filedevice_t* fd, const filepath_t& filename, EFileMode mode, EFileAccess access, EFileOp op, u32 out_caps);
        extern istream_t* get_nullstream();
    } // namespace nfs
}; // namespace ncore

#endif
