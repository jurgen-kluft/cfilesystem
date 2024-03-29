#include "ccore/c_target.h"
#include "cfilesystem/c_filesystem.h"
#include "cfilesystem/c_filepath.h"
#include "cfilesystem/c_dirpath.h"
#include "cfilesystem/c_stream.h"
#include "cfilesystem/private/c_istream.h"
#include "cfilesystem/private/c_filesystem.h"
#include "cfilesystem/private/c_filedevice.h"

namespace ncore
{
    void* INVALID_FILE_HANDLE = (void*)-1;
    void* PENDING_FILE_HANDLE = (void*)-2;
    void* INVALID_DIR_HANDLE  = (void*)-1;

    filesys_t* filesystem_t::mImpl = nullptr;

    bool filesystem_t::register_device(const crunes_t& device_name, filedevice_t* device) { return mImpl->register_device(device_name, device); }

    filepath_t filesystem_t::filepath(const char* str)
    {
        filepath_t fp;
        mImpl->filepath(str, fp);
        return fp;
    }
    dirpath_t filesystem_t::dirpath(const char* str)
    {
        dirpath_t dp;
        mImpl->dirpath(str, dp);
        return dp;
    }
    filepath_t filesystem_t::filepath(const crunes_t& str)
    {
        filepath_t fp;
        mImpl->filepath(str, fp);
        return fp;
    }
    dirpath_t filesystem_t::dirpath(const crunes_t& str)
    {
        dirpath_t dp;
        mImpl->dirpath(str, dp);
        return dp;
    }

    void filesystem_t::open(const filepath_t& filename, EFileMode mode, EFileAccess access, EFileOp op, stream_t& out_stream) { mImpl->open(filename, mode, access, op, out_stream); }
    void filesystem_t::close(stream_t& stream) { return mImpl->close(stream); }
    bool filesystem_t::exists(filepath_t const& filepath) { return mImpl->exists(filepath); }
    bool filesystem_t::exists(dirpath_t const& dirpath) { return mImpl->exists(dirpath); }
    s64  filesystem_t::size(filepath_t const& filepath) { return mImpl->size(filepath); }
    void filesystem_t::rename(filepath_t const& filepath, filepath_t const& xfp) { mImpl->rename(filepath, xfp); }
    void filesystem_t::move(filepath_t const& src, filepath_t const& dst) { mImpl->move(src, dst); }
    void filesystem_t::copy(filepath_t const& src, filepath_t const& dst) { mImpl->copy(src, dst); }
    void filesystem_t::rm(filepath_t const& filepath) { mImpl->rm(filepath); }
    void filesystem_t::rm(dirpath_t const& dirpath) { mImpl->rm(dirpath); }

    // -----------------------------------------------------------
    // -----------------------------------------------------------
    // -----------------------------------------------------------
    // private implementation
    // -----------------------------------------------------------
    // -----------------------------------------------------------
    // -----------------------------------------------------------

    void filesys_t::destroy(stream_t& stream) {}

    void filesys_t::resolve(filepath_t const& fp, pathdevice_t*& device, pathnode_t*& dir, pathstr_t*& filename, pathstr_t*& extension)
    {
        device    = get_pathdevice(fp);
        dir       = get_path(fp);
        filename  = get_filename(fp);
        extension = get_extension(fp);
    }

    void filesys_t::resolve(dirpath_t const& dp, pathdevice_t*& device, pathnode_t*& dir)
    {
        device = get_pathdevice(dp);
        dir    = get_path(dp);
    }

    pathdevice_t* filesys_t::get_pathdevice(dirpath_t const& dirpath) { return dirpath.m_device; }
    pathdevice_t* filesys_t::get_pathdevice(filepath_t const& filepath) { return filepath.m_dirpath.m_device; }
    pathnode_t*   filesys_t::get_path(dirpath_t const& dirpath) { return dirpath.m_path; }
    pathnode_t*   filesys_t::get_path(filepath_t const& filepath) { return filepath.m_dirpath.m_path; }
    pathstr_t*    filesys_t::get_filename(filepath_t const& filepath) { return filepath.m_filename; }
    pathstr_t*    filesys_t::get_extension(filepath_t const& filepath) { return filepath.m_extension; }
    filesys_t*    filesys_t::get_filesystem(dirpath_t const& dirpath) { return dirpath.m_device->m_root; }
    filesys_t*    filesys_t::get_filesystem(filepath_t const& filepath) { return filepath.m_dirpath.m_device->m_root; }

    void filesys_t::filepath(const char* str, filepath_t& fp)
    {
        crunes_t filepathstr(str);
        filepath(filepathstr, fp);
    }

    void filesys_t::dirpath(const char* str, dirpath_t& dp)
    {
        crunes_t dirpathstr(str);
        dirpath(dirpathstr, dp);
    }

    void filesys_t::filepath(const crunes_t& str, filepath_t& fp)
    {
        pathstr_t*  devicename = nullptr;
        pathnode_t* path       = nullptr;
        pathstr_t*  filename   = nullptr;
        pathstr_t*  extension  = nullptr;
        register_fullfilepath(str, devicename, path, filename, extension);
        pathdevice_t* device = register_device(devicename);

        filepath_t filepath(device, path, filename, extension);
        fp = filepath;
    }

    void filesys_t::dirpath(const crunes_t& str, dirpath_t& dp)
    {
        pathstr_t*  devicename = nullptr;
        pathnode_t* path       = nullptr;
        register_fulldirpath(str, devicename, path);
        pathdevice_t* device = register_device(devicename);
        dirpath_t     dirpath(device, path);
        dp = dirpath;
    }

    extern istream_t* get_filestream();

    void filesys_t::open(const filepath_t& filename, EFileMode mode, EFileAccess access, EFileOp op, stream_t& out_stream)
    {
        filedevice_t* fd = filename.m_dirpath.m_device->m_fileDevice;

        void* filehandle;
        if (fd->openFile(filename, mode, access, op, filehandle))
        {
            filehandle_t* fh = obtain_filehandle();
            fh->m_owner      = this;
            fh->m_handle     = filehandle;
            fh->m_filedevice = fd;
            fh->m_filename   = m_paths->attach(filename.m_filename);
            fh->m_extension  = m_paths->attach(filename.m_extension);
            fh->m_device     = m_paths->attach(filename.m_dirpath.m_device);
            fh->m_path       = m_paths->attach(filename.m_dirpath.m_path);
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

            release_device(fh->m_device);
            release_path(fh->m_path);
            release_filename(fh->m_filename);
            release_extension(fh->m_extension);

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

    bool filesys_t::has_device(const crunes_t& device_name)
    {
        pathstr_t* devname = find_name(device_name);
        if (devname != nullptr)
        {
            pathdevice_t* dev = find_device(devname);
            return dev != nullptr;
        }
        return false;
    }

    bool filesys_t::register_device(const crunes_t& devpathstr, filedevice_t* device)
    {
        pathstr_t*  devname = nullptr;
        pathnode_t* devpath = nullptr;
        register_fulldirpath(devpathstr, devname, devpath);

        pathdevice_t* dev = register_device(devname);
        dev->m_devicePath = devpath;
        if (dev->m_fileDevice == nullptr)
        {
            dev->m_fileDevice = device;
        }
        return true;
    }

    bool filesys_t::register_alias(const crunes_t& aliasstr, const crunes_t& devpathstr)
    {
        pathstr_t* aliasname;
        register_name(aliasstr, aliasname);

        pathstr_t*  devname = nullptr;
        pathnode_t* devpath = nullptr;
        register_fulldirpath(devpathstr, devname, devpath);

        pathdevice_t* alias = register_device(aliasname);
        alias->m_alias      = aliasname;
        alias->m_deviceName = devname;
        alias->m_devicePath = devpath;
        alias->m_redirector = register_device(devname);

        return true;
    }

    filehandle_t* filesys_t::obtain_filehandle()
    {
        s32           index       = --m_filehandles_count;
        filehandle_t* fh          = m_filehandles_free[index];
        m_filehandles_free[index] = nullptr;
        return fh;
    }

    void filesys_t::release_filehandle(filehandle_t* fh) { m_filehandles_free[m_filehandles_count++] = fh; }

} // namespace ncore