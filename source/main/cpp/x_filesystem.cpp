#include "xbase/x_target.h"
#include "xfilesystem/x_filesystem.h"
#include "xfilesystem/x_filepath.h"
#include "xfilesystem/x_fileinfo.h"
#include "xfilesystem/x_dirpath.h"
#include "xfilesystem/x_dirinfo.h"
#include "xfilesystem/private/x_devicemanager.h"
#include "xfilesystem/private/x_filesystem.h"
#include "xfilesystem/private/x_filedevice.h"

namespace xcore
{
    void* INVALID_FILE_HANDLE = (void*)-1;

    filesys_t* filesystem_t::mImpl = nullptr;

    bool filesystem_t::register_device(const crunes_t& device_name, filedevice_t* device) { return mImpl->register_device(device_name, device); }

    filepath_t filesystem_t::filepath(const char* str) { return mImpl->filepath(str); }
    dirpath_t  filesystem_t::dirpath(const char* str) { return mImpl->dirpath(str); }
    filepath_t filesystem_t::filepath(const crunes_t& str) { return mImpl->filepath(str); }
    dirpath_t  filesystem_t::dirpath(const crunes_t& str) { return mImpl->dirpath(str); }

    stream_t  filesystem_t::open(const filepath_t& filename, EFileMode mode, EFileAccess access, EFileOp op) 
    { return mImpl->open(filename, mode, access, op); }

    void filesystem_t::close(stream_t& xs) { return mImpl->close(xs); }

    bool        filesystem_t::exists(fileinfo_t const& xfi) { return mImpl->exists(xfi); }
    bool        filesystem_t::exists(dirinfo_t const& xdi) { return mImpl->exists(xdi); }
    s64         filesystem_t::size(fileinfo_t const& xfi) { return mImpl->size(xfi); }

    void    filesystem_t::rename(fileinfo_t const& xfi, filepath_t const& xfp) { mImpl->rename(xfi, xfp); }
    void    filesystem_t::move(fileinfo_t const& src, fileinfo_t const& dst) { mImpl->move(src, dst); }
    void    filesystem_t::copy(fileinfo_t const& src, fileinfo_t const& dst) { mImpl->copy(src, dst); }
    void    filesystem_t::rm(fileinfo_t const& xfi) { mImpl->rm(xfi); }
    void    filesystem_t::rm(dirinfo_t const& xdi) { mImpl->rm(xdi); }

    // -----------------------------------------------------------
    // -----------------------------------------------------------
    // -----------------------------------------------------------
    // private implementation
    // -----------------------------------------------------------
    // -----------------------------------------------------------
    // -----------------------------------------------------------

    filepath_t filesys_t::resolve(filepath_t const& fp, filedevice_t*& device)
    {
        filesys_t* fs = get_filesystem(fp);
        path_t     fs_path;
        device = fs->m_devman->find_device(fp.mPath, fs_path);
        return filepath_t(fs, fs_path);
    }

    dirpath_t filesys_t::resolve(dirpath_t const& dp, filedevice_t*& device)
    {
        filesys_t* fs = get_filesystem(dp);
        path_t     fs_path;
        device = fs->m_devman->find_device(dp.mPath, fs_path);
        return dirpath_t(fs, fs_path);
    }

    path_t& filesys_t::get_path(dirinfo_t& dirinfo) { return dirinfo.mPath.mPath; }
    path_t const& filesys_t::get_path(dirinfo_t const& dirinfo) { return dirinfo.mPath.mPath; }
    path_t& filesys_t::get_path(dirpath_t& dirpath) { return dirpath.mPath; }
    path_t const& filesys_t::get_path(dirpath_t const& dirpath) { return dirpath.mPath; }
    path_t& filesys_t::get_path(filepath_t& filepath) { return filepath.mPath; }
    path_t const& filesys_t::get_path(filepath_t const& filepath) { return filepath.mPath; }
    filesys_t* filesys_t::get_filesystem(dirpath_t const& dirpath) { return dirpath.mParent; }
    filesys_t* filesys_t::get_filesystem(filepath_t const& filepath) { return filepath.mParent; }

    stream_t filesys_t::create_filestream(const filepath_t& filepath, EFileMode fm, EFileAccess fa, EFileOp fo)
    {
        return stream_t(); 
    }
    
    filepath_t filesys_t::filepath(const char* str)
    {
        filepath_t filepath;
        filepath.mParent = this;
        filepath.mPath   = path_t(m_stralloc, str);
        return filepath;
    }

    dirpath_t filesys_t::dirpath(const char* str)
    {
        dirpath_t dirpath;
        dirpath.mParent = this;
        dirpath.mPath   = path_t(m_stralloc, str);
        return dirpath;
    }

    filepath_t filesys_t::filepath(const crunes_t& str)
    {
        filepath_t filepath;
        filepath.mParent = this;
        filepath.mPath   = path_t(m_stralloc, str);
        return filepath;
    }

    dirpath_t filesys_t::dirpath(const crunes_t& str)
    {
        dirpath_t dirpath;
        dirpath.mParent = this;
        dirpath.mPath   = path_t(m_stralloc, str);
        return dirpath;
    }

    bool filesys_t::register_device(const crunes_t& device_name, filedevice_t* device) { return m_devman->add_device(device_name, device); }

    stream_t filesys_t::open(const filepath_t& filename, EFileMode mode, EFileAccess access, EFileOp op) 
    {
        return stream_t();
    }

    void filesys_t::close(stream_t& stream) {}

    bool       filesys_t::exists(fileinfo_t const&) { return false; }
    bool       filesys_t::exists(dirinfo_t const&) { return false; }
    s64        filesys_t::size(fileinfo_t const&) { return 0; }
    void       filesys_t::rename(fileinfo_t const&, filepath_t const&) {}
    void       filesys_t::move(fileinfo_t const& src, fileinfo_t const& dst) {}
    void       filesys_t::copy(fileinfo_t const& src, fileinfo_t const& dst) {}
    void       filesys_t::rm(fileinfo_t const&) {}
    void       filesys_t::rm(dirinfo_t const&) {}

} // namespace xcore