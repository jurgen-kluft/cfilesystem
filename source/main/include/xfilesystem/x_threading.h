#ifndef __X_FILESYSTEM_IOTHREAD_H__
#define __X_FILESYSTEM_IOTHREAD_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

//==============================================================================
#include "xbase/x_debug.h"

namespace ncore
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
}; // namespace ncore

#endif