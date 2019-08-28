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
    class xfile
    {
	public:
        xfilesys*   m_parent;
        void*       m_handle;

		XCORE_CLASS_PLACEMENT_NEW_DELETE
    };

	void*	INVALID_FILE_HANDLE	= (void*)-1;

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
		xfilesys* fs = get_filesystem(fp);
		xpath fs_path;
        device = fs->m_devman->find_device(fp.mPath, fs_path);
        return xfilepath(fs, fs_path);
    }

    xdirpath xfilesys::resolve(xdirpath const& dp, xfiledevice*& device)
    {
		xfilesys* fs = get_filesystem(dp);
        xpath fs_path;
		device = fs->m_devman->find_device(dp.mPath, fs_path);
        return xdirpath(fs, fs_path);
    }

    xpath&       xfilesys::get_xpath(xdirinfo& dirinfo)
    {
        return dirinfo.mPath.mPath;
    }

    xpath const& xfilesys::get_xpath(xdirinfo const& dirinfo)
    {
        return dirinfo.mPath.mPath;
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
        return nullptr;
    }

    void      xfilesys::destroy(xistream* stream)
    {
        
    }

    xfile* xfilesys::open(xfilepath const& filename, EFileMode mode) 
    {
        xfiledevice* fd = nullptr;
        xfilepath sys_filepath = resolve(filename, fd);
        if (fd == nullptr)
            return nullptr;

		void* fh = nullptr;
        if (fd->openFile(sys_filepath, mode, FileAccess_ReadWrite, FileOp_Sync, fh))
		{
			//@TODO: This should be more of a pool allocator
			xfile* file = xnew<xfile>(m_allocator);
			file->m_parent = this;
			file->m_handle = fh;
			return file;
		}

		return nullptr;
	}

    xstream* xfilesys::open_stream(const xfilepath& filename, EFileMode mode, EFileAccess access, EFileOp op) { return nullptr;}

    xwriter* xfilesys::writer(xfile*) { return nullptr; }

    xreader* xfilesys::reader(xfile*) {return nullptr; }

    void xfilesys::close(xfile*) {}
    void xfilesys::close(xfileinfo*) {}
    void xfilesys::close(xdirinfo*) {}
    void xfilesys::close(xreader*) {}
    void xfilesys::close(xwriter*) {}
    void xfilesys::close(xstream*) {}

    xfileinfo* xfilesys::info(xfilepath const& path) {return nullptr; }
    xdirinfo* xfilesys::info(xdirpath const& path) {return nullptr; }

    bool xfilesys::exists(xfileinfo*) {return false;}
    bool xfilesys::exists(xdirinfo*) {return false;}

    s64 xfilesys::size(xfileinfo*) { return 0;}

    xfile* xfilesys::open(xfileinfo*, EFileMode mode) {return nullptr; }
    void xfilesys::rename(xfileinfo*, xfilepath const&) { }
    void xfilesys::move(xfileinfo* src, xfileinfo* dst) {}
    void xfilesys::copy(xfileinfo* src, xfileinfo* dst) {}
    void xfilesys::rm(xfileinfo*) {}
    void xfilesys::rm(xdirinfo*) {}

    s32 xfilesys::read(xreader*, xbuffer&) {return 0;}

    s32 xfilesys::write(xwriter*, xcbuffer const&) {return 0;}

    void xfilesys::read_async(xreader*, xbuffer&) {}

    s32 xfilesys::wait_async(xreader*) {return 0;}

} // namespace xcore