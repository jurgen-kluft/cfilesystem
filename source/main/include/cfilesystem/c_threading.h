#ifndef __C_FILESYSTEM_IOTHREAD_H__
#define __C_FILESYSTEM_IOTHREAD_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

//==============================================================================
#include "ccore/c_debug.h"

namespace ncore
{
    namespace nfs
    {
        class io_thread_t
        {
        public:
            virtual ~io_thread_t() {}

            virtual void sleep(u32 ms) = 0;
            virtual bool quit() const  = 0;
            virtual void wait()        = 0;
            virtual void signal()      = 0;
        };
    } // namespace nfs
}; // namespace ncore

#endif
