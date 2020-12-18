#include "xbase/x_target.h"
#include "xbase/x_debug.h"
#include "xbase/x_runes.h"

#include "xtime/x_datetime.h"

#include "xfilesystem/x_dirinfo.h"
#include "xfilesystem/x_filepath.h"
#include "xfilesystem/x_dirpath.h"
#include "xfilesystem/x_attributes.h"
#include "xfilesystem/x_enumerator.h"
#include "xfilesystem/private/x_filedevice.h"
#include "xfilesystem/private/x_filesystem.h"

//==============================================================================
namespace xcore
{
    dirinfo_t::dirinfo_t() {}

    dirinfo_t::dirinfo_t(const dirinfo_t& dirinfo)
        : mPath(dirinfo.mPath)
    {
    }

    dirinfo_t::dirinfo_t(const dirpath_t& dir)
        : mPath(dir)
    {
    }

    bool            dirinfo_t::isRoot() const { return mPath.isRoot(); }
    bool            dirinfo_t::isRooted() const { return mPath.isRooted(); }
    bool            dirinfo_t::exists() const { return sExists(mPath); }
    bool            dirinfo_t::create() { return sCreate(mPath); }
    bool            dirinfo_t::remove() { return sDelete(mPath); }
    void            dirinfo_t::refresh() {}
    void            dirinfo_t::copy(const dirpath_t& toDirectory, bool overwrite) { sCopy(mPath, toDirectory, overwrite); }
    void            dirinfo_t::move(const dirpath_t& toDirectory) { sMove(mPath, toDirectory); }
    void            dirinfo_t::enumerate(enumerate_delegate_t& enumerator) { sEnumerate(mPath, enumerator); }
    dirpath_t const& dirinfo_t::getDirpath() const { return mPath; }
    bool            dirinfo_t::getRoot(dirinfo_t& outRootDirPath) const { return (mPath.getRoot(outRootDirPath.mPath)); }
    bool            dirinfo_t::getParent(dirinfo_t& outParentDirPath) const { return (mPath.getParent(outParentDirPath.mPath)); }
    bool            dirinfo_t::getTimes(filetimes_t& times) const { return sGetTime(mPath, times); }
    bool            dirinfo_t::setTimes(filetimes_t times) { return sSetTime(mPath, times); }
    bool            dirinfo_t::getAttrs(fileattrs_t& fattrs) const { return sGetAttrs(mPath, fattrs); }
    bool            dirinfo_t::setAttrs(fileattrs_t fattrs) { return sSetAttrs(mPath, fattrs); }

    dirinfo_t& dirinfo_t::operator=(const dirinfo_t& other)
    {
        if (this == &other)
            return *this;

        mPath = other.mPath;
        return *this;
    }

    dirinfo_t& dirinfo_t::operator=(const dirpath_t& other)
    {
        if (&mPath == &other)
            return *this;

        mPath = other;
        return *this;
    }

    bool dirinfo_t::operator==(const dirpath_t& other) const { return mPath == other; }
    bool dirinfo_t::operator!=(const dirpath_t& other) const { return mPath != other; }
    bool dirinfo_t::operator==(const dirinfo_t& other) const { return mPath == other.mPath; }
    bool dirinfo_t::operator!=(const dirinfo_t& other) const { return mPath != other.mPath; }

    // Static functions
    bool dirinfo_t::sCreate(const dirpath_t& dirpath)
    {
        filedevice_t* device;
        dirpath_t     syspath = dirpath.mParent->resolve(dirpath, device);
        return (device != nullptr && device->createDir(syspath));
    }

    bool dirinfo_t::sDelete(const dirpath_t& dirpath)
    {
        filedevice_t* device;
        dirpath_t     syspath = dirpath.mParent->resolve(dirpath, device);
        return (device != nullptr && device->deleteDir(syspath));
    }

    bool dirinfo_t::sExists(const dirpath_t& dirpath)
    {
        filedevice_t* device;
        dirpath_t     syspath = dirpath.mParent->resolve(dirpath, device);
        return (device != nullptr && device->hasDir(syspath));
    }

    void dirinfo_t::sEnumerate(const dirpath_t& dirpath, enumerate_delegate_t& enumerator)
    {
        filedevice_t* device;
        dirpath_t     syspath = dirpath.mParent->resolve(dirpath, device);
        if (device != nullptr)
            device->enumerate(syspath, enumerator);
    }

    bool dirinfo_t::sSetTime(const dirpath_t& dirpath, const filetimes_t& ftimes)
    {
        filedevice_t* device;
        dirpath_t     syspath = dirpath.mParent->resolve(dirpath, device);
        if (device != nullptr)
            return device->setDirTime(syspath, ftimes);
        return false;
    }

    bool dirinfo_t::sGetTime(const dirpath_t& dirpath, filetimes_t& ftimes)
    {
        filedevice_t* device;
        dirpath_t     syspath = dirpath.mParent->resolve(dirpath, device);
        if (device != nullptr)
            return device->setDirTime(syspath, ftimes);

        if (device != NULL && device->getDirTime(syspath, ftimes))
            return true;

        ftimes = filetimes_t();
        return false;
    }

    bool dirinfo_t::sSetAttrs(const dirpath_t& dirpath, const fileattrs_t& fattrs)
    {
        filedevice_t* device;
        dirpath_t     syspath = dirpath.mParent->resolve(dirpath, device);
        if (device != nullptr)
            return device->setDirAttr(syspath, fattrs);
        return false;
    }

    bool dirinfo_t::sGetAttrs(const dirpath_t& dirpath, fileattrs_t& fattrs)
    {
        filedevice_t* device;
        dirpath_t     syspath = dirpath.mParent->resolve(dirpath, device);
        if (device != nullptr)
        {
            return device->getDirAttr(syspath, fattrs);
        }
        fattrs = fileattrs_t();
        return false;
    }

    bool dirinfo_t::sCopy(const dirpath_t& srcdirpath, const dirpath_t& dstdirpath, bool overwrite)
    {
        filedevice_t* srcdevice;
        dirpath_t     srcsyspath = srcdirpath.mParent->resolve(srcdirpath, srcdevice);

        filedevice_t* dstdevice;
        dirpath_t     dstsyspath = dstdirpath.mParent->resolve(dstdirpath, dstdevice);

        if (srcdevice != nullptr && dstdevice != nullptr)
            return srcdevice->copyDir(srcdirpath, dstdirpath, overwrite);

        return false;
    }

    bool dirinfo_t::sMove(const dirpath_t& srcdirpath, const dirpath_t& dstdirpath, bool overwrite)
    {
        filedevice_t* srcdevice;
        dirpath_t     srcsyspath = srcdirpath.mParent->resolve(srcdirpath, srcdevice);

        filedevice_t* dstdevice;
        dirpath_t     dstsyspath = dstdirpath.mParent->resolve(dstdirpath, dstdevice);

        if (srcdevice != nullptr && dstdevice != nullptr)
            return srcdevice->moveDir(srcdirpath, dstdirpath, overwrite);

        return false;
    }

}; // namespace xcore
