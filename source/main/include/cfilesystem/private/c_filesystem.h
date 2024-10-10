#ifndef __C_FILESYSTEM_FILESYSTEM_IMP_H__
#define __C_FILESYSTEM_FILESYSTEM_IMP_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "cbase/c_allocator.h"

#include "cfilesystem/private/c_enumerations.h"

namespace ncore
{
    struct runes_t;
    struct crunes_t;
    class alloc_t;
    class istream_t;

    namespace nfs
    {
        class filesys_t;
        class filedevice_t;
        class stream_t;

        struct filehandle_t
        {
            void*         m_handle;
            filesys_t*    m_owner;
            s32           m_refcount;
            s32           m_salt;
            filedevice_t* m_filedevice;
            filehandle_t* m_prev;
            filehandle_t* m_next;
        };

        class filesys_t
        {
        public:
            void init(alloc_t* allocator);
            void exit(alloc_t* allocator);

            // -----------------------------------------------------------
            static void destroy(stream_t& stream);

            // -----------------------------------------------------------
            void open(const filepath_t& filename, EFileMode::Enum mode, EFileAccess::Enum access, EFileOp::Enum op, stream_t& out_stream);
            void close(stream_t& stream);
            bool exists(filepath_t const&);
            bool exists(dirpath_t const&);
            s64  size(filepath_t const&);
            void rename(filepath_t const&, filepath_t const&);
            void move(filepath_t const& src, filepath_t const& dst);
            void copy(filepath_t const& src, filepath_t const& dst);
            void rm(filepath_t const&);
            void rm(dirpath_t const&);

            // -----------------------------------------------------------
            //
            u32      m_max_open_files;
            u32      m_max_path_objects;
            char     m_default_slash;
            alloc_t* m_allocator;

            // -----------------------------------------------------------
            filehandle_t* obtain_filehandle();
            void          release_filehandle(filehandle_t* fh);

            filehandle_t* m_filehandles_free;
            filehandle_t* m_filehandles_active;
            filehandle_t* m_filehandles_array;
            s32           m_filehandles_free_index;
            s32           m_filehandles_count;

            DCORE_CLASS_PLACEMENT_NEW_DELETE
        };
    } // namespace nfs
}; // namespace ncore

#endif
