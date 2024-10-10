#include "ccore/c_target.h"
#include "cfilesystem/c_filesystem.h"
#include "cfilesystem/c_stream.h"
#include "cfilesystem/private/c_istream.h"
#include "cfilesystem/private/c_filesystem.h"
#include "cfilesystem/private/c_filedevice.h"

namespace ncore
{
    namespace nfs
    {
        void* INVALID_FILE_HANDLE = (void*)-1;
        void* PENDING_FILE_HANDLE = (void*)-2;
        void* INVALID_DIR_HANDLE  = (void*)-1;

        filesys_t* mImpl = nullptr;

        void open(const filepath_t& filename, EFileMode::Enum mode, EFileAccess::Enum access, EFileOp::Enum op, stream_t& out_stream) { mImpl->open(filename, mode, access, op, out_stream); }
        void close(stream_t& stream) { return mImpl->close(stream); }
        bool exists(filepath_t const& filepath) { return mImpl->exists(filepath); }
        bool exists(dirpath_t const& dirpath) { return mImpl->exists(dirpath); }
        s64  size(filepath_t const& filepath) { return mImpl->size(filepath); }
        void rename(filepath_t const& filepath, filepath_t const& xfp) { mImpl->rename(filepath, xfp); }
        void move(filepath_t const& src, filepath_t const& dst) { mImpl->move(src, dst); }
        void copy(filepath_t const& src, filepath_t const& dst) { mImpl->copy(src, dst); }
        void rm(filepath_t const& filepath) { mImpl->rm(filepath); }
        void rm(dirpath_t const& dirpath) { mImpl->rm(dirpath); }

        // -----------------------------------------------------------
        // -----------------------------------------------------------
        // -----------------------------------------------------------
        // private implementation
        // -----------------------------------------------------------
        // -----------------------------------------------------------
        // -----------------------------------------------------------

        void filesys_t::destroy(stream_t& stream) {}

        extern istream_t* get_filestream();

        void filesys_t::open(const filepath_t& filename, EFileMode::Enum mode, EFileAccess::Enum access, EFileOp::Enum op, stream_t& out_stream)
        {
            filedevice_t* fd = filename.m_dirpath.m_device->m_fileDevice;

            void* filehandle;
            if (fd->openFile(filename, mode, access, op, filehandle))
            {
                filehandle_t* fh = obtain_filehandle();
                fh->m_owner      = this;
                fh->m_handle     = filehandle;
                fh->m_filedevice = fd;
                // fh->m_filename   = m_paths->attach(filename.m_filename);
                // fh->m_extension  = m_paths->attach(filename.m_extension);
                // fh->m_device     = m_paths->attach(filename.m_dirpath.m_device);
                // fh->m_path       = m_paths->attach(filename.m_dirpath.m_path);
                out_stream       = stream_t(get_filestream(), fh);
            }
            else
            {
                out_stream = stream_t(get_nullstream(), nullptr);
            }
        }

        void filesys_t::close(stream_t& stream)
        {
            filehandle_t* fh = stream.m_filehandle;
            filedevice_t* fd = fh->m_filedevice;

            if (fh->m_refcount == 1)
            {
                fd->closeFile(fh->m_handle);
                fh->m_handle = nullptr;

                m_allocator->deallocate(fh);
            }
            fh->m_refcount -= 1;

            release_filehandle(fh);

            stream.m_filehandle = nullptr;
            stream.m_pimpl      = nullptr;
        }

        bool filesys_t::exists(filepath_t const&) { return false; }
        bool filesys_t::exists(dirpath_t const&) { return false; }
        s64  filesys_t::size(filepath_t const&) { return 0; }
        void filesys_t::rename(filepath_t const&, filepath_t const&) {}
        void filesys_t::move(filepath_t const& src, filepath_t const& dst) {}
        void filesys_t::copy(filepath_t const& src, filepath_t const& dst) {}
        void filesys_t::rm(filepath_t const&) {}
        void filesys_t::rm(dirpath_t const&) {}

        filehandle_t* filesys_t::obtain_filehandle()
        {
            // obtain from freelist or free index
            filehandle_t* fh = nullptr;
            if (m_filehandles_free == nullptr)
            {
                fh = &m_filehandles_array[m_filehandles_free_index++];
            }
            else
            {
                fh                 = m_filehandles_free;
                m_filehandles_free = fh->m_next;
                if (m_filehandles_free != nullptr)
                {
                    m_filehandles_free->m_prev = nullptr;
                }
            }

            // insert into active list
            fh->m_next = m_filehandles_active;
            fh->m_prev = nullptr;
            if (m_filehandles_active != nullptr)
            {
                m_filehandles_active->m_prev = fh;
            }
            m_filehandles_active = fh;
            return fh;
        }

        void filesys_t::release_filehandle(filehandle_t* fh)
        {
            // remove from active list
            if (fh->m_prev != nullptr)
            {
                fh->m_prev->m_next = fh->m_next;
            }
            if (fh->m_next != nullptr)
            {
                fh->m_next->m_prev = fh->m_prev;
            }
            if (m_filehandles_active == fh)
            {
                m_filehandles_active = fh->m_next;
            }

            // insert into free list
            if (m_filehandles_free == nullptr)
            {
                m_filehandles_free = fh;
                fh->m_prev         = nullptr;
                fh->m_next         = nullptr;
            }
            else
            {
                fh->m_prev                 = nullptr;
                fh->m_next                 = m_filehandles_free;
                m_filehandles_free->m_prev = fh;
                m_filehandles_free         = fh;
            }
        }
    } // namespace nfs
} // namespace ncore
