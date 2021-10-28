#include "xbase/x_target.h"
#include "xfilesystem/x_filesystem.h"
#include "xfilesystem/x_filepath.h"
#include "xfilesystem/x_fileinfo.h"
#include "xfilesystem/x_dirpath.h"
#include "xfilesystem/x_dirinfo.h"
#include "xfilesystem/x_stream.h"
#include "xfilesystem/private/x_devicemanager.h"
#include "xfilesystem/private/x_filesystem.h"
#include "xfilesystem/private/x_filedevice.h"

namespace xcore
{
    void* INVALID_FILE_HANDLE = (void*)-1;
    void* PENDING_FILE_HANDLE = (void*)-2;
    void* INVALID_DIR_HANDLE = (void*)-1;

    filesys_t* filesystem_t::mImpl = nullptr;

    bool filesystem_t::register_device(const crunes_t& device_name, filedevice_t* device) { return mImpl->register_device(device_name, device); }

    filepath_t filesystem_t::filepath(const char* str) { filepath_t fp;  mImpl->filepath(str, fp); return fp; }
    dirpath_t  filesystem_t::dirpath(const char* str) { dirpath_t dp; mImpl->dirpath(str, dp); return dp; }
    filepath_t filesystem_t::filepath(const crunes_t& str) { filepath_t fp; mImpl->filepath(str, fp); return fp; }
    dirpath_t  filesystem_t::dirpath(const crunes_t& str) { dirpath_t dp; mImpl->dirpath(str, dp); return dp; }

    void filesystem_t::open(const filepath_t& filename, EFileMode mode, EFileAccess access, EFileOp op, stream_t& out_stream) 
    { 
        mImpl->open(filename, mode, access, op, out_stream); 
    }

    void filesystem_t::close(stream_t& xs) { return mImpl->close(xs); }

    bool filesystem_t::exists(fileinfo_t const& xfi) { return mImpl->exists(xfi); }
    bool filesystem_t::exists(dirinfo_t const& xdi) { return mImpl->exists(xdi); }
    s64  filesystem_t::size(fileinfo_t const& xfi) { return mImpl->size(xfi); }

    void filesystem_t::rename(fileinfo_t const& xfi, filepath_t const& xfp) { mImpl->rename(xfi, xfp); }
    void filesystem_t::move(fileinfo_t const& src, fileinfo_t const& dst) { mImpl->move(src, dst); }
    void filesystem_t::copy(fileinfo_t const& src, fileinfo_t const& dst) { mImpl->copy(src, dst); }
    void filesystem_t::rm(fileinfo_t const& xfi) { mImpl->rm(xfi); }
    void filesystem_t::rm(dirinfo_t const& xdi) { mImpl->rm(xdi); }

    // -----------------------------------------------------------
    // -----------------------------------------------------------
    // -----------------------------------------------------------
    // private implementation
    // -----------------------------------------------------------
    // -----------------------------------------------------------
    // -----------------------------------------------------------
    void filesys_t::create_filestream(const filepath_t& filepath, EFileMode fm, EFileAccess fa, EFileOp fo, stream_t& out_stream) 
    { out_stream = stream_t(); }

    void filesys_t::destroy(stream_t& stream) {}

    void filesys_t::resolve(filepath_t const& fp, pathdevice_t*& device, path_t*& dir, pathname_t*& filename, pathname_t*& extension)
    {
        filesys_t* fs = get_filesystem(fp);
        device = get_pathdevice(fp);
        dir = get_path(fp);
        filename = get_filename(fp);
        extension = get_extension(fp);
    }

    void filesys_t::resolve(dirpath_t const& dp, pathdevice_t*& device, path_t*& dir)
    {
        filesys_t* fs = get_filesystem(dp);
        device = get_pathdevice(dp);
        dir = get_path(dp);
    }

    pathdevice_t*       filesys_t::get_pathdevice(dirinfo_t const& dirinfo) { return dirinfo.m_path.m_device; }
    pathdevice_t * filesys_t::get_pathdevice(dirpath_t const& dirpath) { return dirpath.m_device; }
    pathdevice_t * filesys_t::get_pathdevice(filepath_t const& filepath) { return filepath.m_dirpath.m_device; }

    path_t * filesys_t::get_path(dirinfo_t const& dirinfo) { return dirinfo.m_path.m_path; }
    path_t * filesys_t::get_path(dirpath_t const& dirpath) { return dirpath.m_path; }
    path_t * filesys_t::get_path(filepath_t const& filepath) { return filepath.m_dirpath.m_path; }

    pathname_t * filesys_t::get_filename(filepath_t const& filepath) { return filepath.m_filename; }
    pathname_t * filesys_t::get_extension(filepath_t const& filepath) { return filepath.m_extension; }

    filesys_t* filesys_t::get_filesystem(dirpath_t const& dirpath) { return dirpath.m_device->m_root->m_owner; }
    filesys_t* filesys_t::get_filesystem(filepath_t const& filepath) { return filepath.root().m_device->m_root->m_owner; }


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
        pathname_t* devicename = nullptr;
        path_t*     path       = nullptr;
        pathname_t* filename   = nullptr;
        pathname_t* extension  = nullptr;
        register_fullfilepath(str, devicename, path, filename, extension);
        pathdevice_t* device = register_device(devicename);

        dirpath_t  dirpath(device, path);
        filepath_t filepath(dirpath, filename, extension);
        fp = filepath;
    }

    void filesys_t::dirpath(const crunes_t& str, dirpath_t& dp)
    {
        pathname_t* devicename = nullptr;
        path_t*     path       = nullptr;
        register_directory(str, devicename, path);
        pathdevice_t* device = register_device(devicename);
        dirpath_t     dirpath(device, path);
        dp = dirpath;
    }

    bool filesys_t::register_device(const crunes_t& device_name, filedevice_t* device) { return m_devman->add_device(device_name, device); }

    void filesys_t::open(const filepath_t& filename, EFileMode mode, EFileAccess access, EFileOp op, stream_t& out_stream) { out_stream = stream_t(); }

    void filesys_t::close(stream_t& stream) {}

    bool filesys_t::exists(fileinfo_t const&) { return false; }
    bool filesys_t::exists(dirinfo_t const&) { return false; }
    s64  filesys_t::size(fileinfo_t const&) { return 0; }
    void filesys_t::rename(fileinfo_t const&, filepath_t const&) {}
    void filesys_t::move(fileinfo_t const& src, fileinfo_t const& dst) {}
    void filesys_t::copy(fileinfo_t const& src, fileinfo_t const& dst) {}
    void filesys_t::rm(fileinfo_t const&) {}
    void filesys_t::rm(dirinfo_t const&) {}

} // namespace xcore