#include "xbase/x_target.h"
#include "xfilesystem/x_filesystem.h"
#include "xfilesystem/x_filepath.h"
#include "xfilesystem/x_fileinfo.h"
#include "xfilesystem/x_dirpath.h"
#include "xfilesystem/x_dirinfo.h"
#include "xfilesystem/private/x_devicemanager.h"
#include "xfilesystem/private/x_filesystem.h"

namespace xcore
{

    xfile*   xfilesystem::open(xfilepath const& filename, EFileMode mode) { return mImpl->open(filename, mode); }
    xstream* xfilesystem::open_stream(const xfilepath& filename, EFileMode mode, EFileAccess access, EFileOp op) { return mImpl->open_stream(filename, mode, access, op); }
    xwriter* xfilesystem::writer(xfile* xf) { return mImpl->writer(xf); }
    xreader* xfilesystem::reader(xfile* xf) { return mImpl->reader(xf); }

    void xfilesystem::close(xfile* xf) { return mImpl->close(xf); }
    void xfilesystem::close(xfileinfo* xfi) { return mImpl->close(xfi); }
    void xfilesystem::close(xdirinfo* xdi) { return mImpl->close(xdi); }
    void xfilesystem::close(xreader* xr) { return mImpl->close(xr); }
    void xfilesystem::close(xwriter* xw) { return mImpl->close(xw); }
    void xfilesystem::close(xstream* xs) { return mImpl->close(xs); }

    xfileinfo* xfilesystem::info(xfilepath const& path) { return mImpl->info(path); }
    xdirinfo*  xfilesystem::info(xdirpath const& path) { return mImpl->info(path); }
    bool       xfilesystem::exists(xfileinfo* xfi) { return mImpl->exists(xfi); }
    bool       xfilesystem::exists(xdirinfo* xdi) { return mImpl->exists(xdi); }
    s64        xfilesystem::size(xfileinfo* xfi) { return mImpl->size(xfi); }

    xfile* xfilesystem::open(xfileinfo* xfi, EFileMode mode) { return mImpl->open(xfi, mode); }
    void   xfilesystem::rename(xfileinfo* xfi, xfilepath const& xfp) { mImpl->rename(xfi, xfp); }
    void   xfilesystem::move(xfileinfo* src, xfileinfo* dst) { mImpl->move(src, dst); }
    void   xfilesystem::copy(xfileinfo* src, xfileinfo* dst) { mImpl->copy(src, dst); }
    void   xfilesystem::rm(xfileinfo* xfi) { mImpl->rm(xfi); }
    void   xfilesystem::rm(xdirinfo* xdi) { mImpl->rm(xdi); }
    s32    xfilesystem::read(xreader* xr, xbuffer& xb) { return mImpl->read(xr, xb); }
    s32    xfilesystem::write(xwriter* xw, xcbuffer const& xb) { return mImpl->write(xw, xb); }
    void   xfilesystem::read_async(xreader* xr, xbuffer& xb) { mImpl->read_async(xr, xb); }
    s32    xfilesystem::wait_async(xreader* xr) { return mImpl->wait_async(xr); }

    // -----------------------------------------------------------
    // -----------------------------------------------------------
    // -----------------------------------------------------------
    // private implementation
    // -----------------------------------------------------------
    // -----------------------------------------------------------
    // -----------------------------------------------------------

    xfilepath xfilesys::resolve(xfilepath const& fp, xfiledevice*& device)
    {
		xfilesys* fs = xfilesys::get_filesystem(fp);
        // search the filedevice using the device part of @fp and the device manager.
		xfilepath sys_fp = fs->
        device = fs->m_devman->find_device(fp.mPath,);
        return device_filepath;
    }

    xdirpath xfilesys::resolve(xdirpath const& dp, xfiledevice*& device)
    {
        // search the filedevice using the device part of @dp and the device manager.
        xdirpath device_dirpath = m_devman->find_device(dp.mPath, device);
        return device_dirpath;
    }

    xpath&       xfilesys::get_xpath(xdirinfo& dirinfo)
    {
        return dirinfo.mDirPath.mPath;
    }

    xpath const& xfilesys::get_xpath(xdirinfo const& dirinfo)
    {
        return dirinfo.mDirPath.mPath;
    }

    xpath&       xfilesys::get_xpath(xdirpath& dirpath)
    {
        return dirpath.mPath;
    }

    xpath const& xfilesys::get_xpath(xdirpath const& dirpath)
    {
        return dirpath.mPath;
    }

    xpath&       xfilesys::get_xpath(xfilepath& filepath)
    {
        return filepath.mPath;
    }

    xpath const& xfilesys::get_xpath(xfilepath const& filepath)
    {
        return filepath.mPath;
    }

    xfilesys*    xfilesys::get_filesystem(xdirpath const& dirpath)
    {
        return dirpath.mParent;
    }

    xfilesys*    xfilesys::get_filesystem(xfilepath const& filepath)
    {
        return filepath.mParent;
    }

    xistream* xfilesys::create_filestream(const xfilepath& filepath, EFileMode fm, EFileAccess fa, EFileOp fo)
    {
        // (xalloc*, const xfilepath& filepath, EFileMode mode, EFileAccess access, EFileOp op);
        xistream* stream = xistream::create_filestream(m_allocator, filepath, fm, fa, fo);
        return stream;
    }

    void      xfilesys::destroy(xistream* stream)
    {
        xistream::destroy_filestream(m_allocator, stream);
    }

    xfile* xfilesys::open(xfilepath const& filename, EFileMode fm) 
    {
        xfiledevice* fd = nullptr;
        xfilepath sys_filepath = resolve(filename, xfiledevice*& fd);
        if (fd == nullptr)
            return nullptr;
        xfile* file = xnew<xfile>(m_allocator);
        file->m_parent = this;
        file->m_handle = nullptr;
        // bool openFile(xfilepath const& szFilename, bool boRead, bool boWrite, void*& outHandle)
        if (fm == FileMode_
        fd->openFile(sys_filepath, mode == FileMode_)
    }

    xstream* xfilesys::open_stream(const xfilepath& filename, EFileMode mode, EFileAccess access, EFileOp op) {}

    xwriter* xfilesys::writer(xfile*) {}

    xreader* xfilesys::reader(xfile*) {}

    void xfilesys::close(xfile*) {}

    void xfilesys::close(xfileinfo*) {}

    void xfilesys::close(xdirinfo*) {}

    void xfilesys::close(xreader*) {}

    void xfilesys::close(xwriter*) {}

    void xfilesys::close(xstream*) {}

    xfileinfo* xfilesys::info(xfilepath const& path) {}

    xdirinfo* xfilesys::info(xdirpath const& path) {}

    bool xfilesys::exists(xfileinfo*) {}

    bool xfilesys::exists(xdirinfo*) {}

    s64 xfilesys::size(xfileinfo*) {}

    xfile* xfilesys::open(xfileinfo*, EFileMode mode) {}

    void xfilesys::rename(xfileinfo*, xfilepath const&) {}

    void xfilesys::move(xfileinfo* src, xfileinfo* dst) {}

    void xfilesys::copy(xfileinfo* src, xfileinfo* dst) {}

    void xfilesys::rm(xfileinfo*) {}

    void xfilesys::rm(xdirinfo*) {}

    s32 xfilesys::read(xreader*, xbuffer&) {}

    s32 xfilesys::write(xwriter*, xcbuffer const&) {}

    void xfilesys::read_async(xreader*, xbuffer&) {}

    s32 xfilesys::wait_async(xreader*) {}

} // namespace xcore