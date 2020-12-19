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
    class file_t
    {
    public:
        filesys_t* m_parent;
        void*      m_handle;

        XCORE_CLASS_PLACEMENT_NEW_DELETE
    };

    void* INVALID_FILE_HANDLE = (void*)-1;

    filesys_t* filesystem_t::mImpl = nullptr;

    bool filesystem_t::register_device(const crunes_t& device_name, filedevice_t* device) { return mImpl->register_device(device_name, device); }

    filepath_t filesystem_t::filepath(const char* str) { return mImpl->filepath(str); }
    dirpath_t  filesystem_t::dirpath(const char* str) { return mImpl->dirpath(str); }
    filepath_t filesystem_t::filepath(const crunes_t& str) { return mImpl->filepath(str); }
    dirpath_t  filesystem_t::dirpath(const crunes_t& str) { return mImpl->dirpath(str); }
    void       filesystem_t::to_ascii(filepath_t const& fp, runes_t& str) { mImpl->to_ascii(fp, str); }
    void       filesystem_t::to_ascii(dirpath_t const& dp, runes_t& str) { mImpl->to_ascii(dp, str); }

    file_t*   filesystem_t::open(filepath_t const& filename, EFileMode mode) { return mImpl->open(filename, mode); }
    stream_t* filesystem_t::open_stream(const filepath_t& filename, EFileMode mode, EFileAccess access, EFileOp op) { return mImpl->open_stream(filename, mode, access, op); }
    writer_t* filesystem_t::writer(file_t* xf) { return mImpl->writer(xf); }
    reader_t* filesystem_t::reader(file_t* xf) { return mImpl->reader(xf); }

    void filesystem_t::close(file_t* xf) { return mImpl->close(xf); }
    void filesystem_t::close(fileinfo_t* xfi) { return mImpl->close(xfi); }
    void filesystem_t::close(dirinfo_t* xdi) { return mImpl->close(xdi); }
    void filesystem_t::close(reader_t* xr) { return mImpl->close(xr); }
    void filesystem_t::close(writer_t* xw) { return mImpl->close(xw); }
    void filesystem_t::close(stream_t* xs) { return mImpl->close(xs); }

    fileinfo_t* filesystem_t::info(filepath_t const& path) { return mImpl->info(path); }
    dirinfo_t*  filesystem_t::info(dirpath_t const& path) { return mImpl->info(path); }
    bool        filesystem_t::exists(fileinfo_t* xfi) { return mImpl->exists(xfi); }
    bool        filesystem_t::exists(dirinfo_t* xdi) { return mImpl->exists(xdi); }
    s64         filesystem_t::size(fileinfo_t* xfi) { return mImpl->size(xfi); }

    file_t* filesystem_t::open(fileinfo_t* xfi, EFileMode mode) { return mImpl->open(xfi, mode); }
    void    filesystem_t::rename(fileinfo_t* xfi, filepath_t const& xfp) { mImpl->rename(xfi, xfp); }
    void    filesystem_t::move(fileinfo_t* src, fileinfo_t* dst) { mImpl->move(src, dst); }
    void    filesystem_t::copy(fileinfo_t* src, fileinfo_t* dst) { mImpl->copy(src, dst); }
    void    filesystem_t::rm(fileinfo_t* xfi) { mImpl->rm(xfi); }
    void    filesystem_t::rm(dirinfo_t* xdi) { mImpl->rm(xdi); }
    s32     filesystem_t::read(reader_t* xr, buffer_t& xb) { return mImpl->read(xr, xb); }
    s32     filesystem_t::write(writer_t* xw, cbuffer_t const& xb) { return mImpl->write(xw, xb); }
    void    filesystem_t::read_async(reader_t* xr, buffer_t& xb) { mImpl->read_async(xr, xb); }
    s32     filesystem_t::wait_async(reader_t* xr) { return mImpl->wait_async(xr); }

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

    path_t& filesys_t::get_xpath(dirinfo_t& dirinfo) { return dirinfo.mPath.mPath; }
    path_t const& filesys_t::get_xpath(dirinfo_t const& dirinfo) { return dirinfo.mPath.mPath; }
    path_t& filesys_t::get_xpath(dirpath_t& dirpath) { return dirpath.mPath; }
    path_t const& filesys_t::get_xpath(dirpath_t const& dirpath) { return dirpath.mPath; }
    path_t& filesys_t::get_xpath(filepath_t& filepath) { return filepath.mPath; }
    path_t const& filesys_t::get_xpath(filepath_t const& filepath) { return filepath.mPath; }
    filesys_t* filesys_t::get_filesystem(dirpath_t const& dirpath) { return dirpath.mParent; }
    filesys_t* filesys_t::get_filesystem(filepath_t const& filepath) { return filepath.mParent; }

    istream_t* filesys_t::create_filestream(const filepath_t& filepath, EFileMode fm, EFileAccess fa, EFileOp fo) { return nullptr; }
    void filesys_t::destroy(istream_t* stream) {}

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

    void filesys_t::to_ascii(filepath_t const& fp, runes_t& str) {}
    void filesys_t::to_ascii(dirpath_t const& dp, runes_t& str) {}

    bool filesys_t::register_device(const crunes_t& device_name, filedevice_t* device) { return m_devman->add_device(device_name, device); }

    file_t* filesys_t::open(filepath_t const& filename, EFileMode mode)
    {
        filedevice_t* fd           = nullptr;
        filepath_t    sys_filepath = resolve(filename, fd);
        if (fd == nullptr)
            return nullptr;

        void* fh = nullptr;
        if (fd->openFile(sys_filepath, mode, FileAccess_ReadWrite, FileOp_Sync, fh))
        {
            //@TODO: This should be more of a pool allocator
            file_t* file   = xnew<file_t>();
            file->m_parent = this;
            file->m_handle = fh;
            return file;
        }

        return nullptr;
    }

    stream_t* filesys_t::open_stream(const filepath_t& filename, EFileMode mode, EFileAccess access, EFileOp op) { return nullptr; }

    writer_t* filesys_t::writer(file_t*) { return nullptr; }
    reader_t* filesys_t::reader(file_t*) { return nullptr; }

    void filesys_t::close(file_t*) {}
    void filesys_t::close(fileinfo_t*) {}
    void filesys_t::close(dirinfo_t*) {}
    void filesys_t::close(reader_t*) {}
    void filesys_t::close(writer_t*) {}
    void filesys_t::close(stream_t*) {}

    fileinfo_t* filesys_t::info(filepath_t const& path) { return nullptr; }
    dirinfo_t*  filesys_t::info(dirpath_t const& path) { return nullptr; }

    bool filesys_t::exists(fileinfo_t*) { return false; }
    bool filesys_t::exists(dirinfo_t*) { return false; }

    s64 filesys_t::size(fileinfo_t*) { return 0; }

    file_t* filesys_t::open(fileinfo_t*, EFileMode mode) { return nullptr; }
    void    filesys_t::rename(fileinfo_t*, filepath_t const&) {}
    void    filesys_t::move(fileinfo_t* src, fileinfo_t* dst) {}
    void    filesys_t::copy(fileinfo_t* src, fileinfo_t* dst) {}
    void    filesys_t::rm(fileinfo_t*) {}
    void    filesys_t::rm(dirinfo_t*) {}

    s32 filesys_t::read(reader_t*, buffer_t&) { return 0; }
    s32 filesys_t::write(writer_t*, cbuffer_t const&) { return 0; }

    void filesys_t::read_async(reader_t*, buffer_t&) {}
    s32  filesys_t::wait_async(reader_t*) { return 0; }

} // namespace xcore