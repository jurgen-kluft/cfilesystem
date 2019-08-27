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
    xdirinfo::xdirinfo() {}

    xdirinfo::xdirinfo(const xdirinfo& dirinfo)
        : mPath(dirinfo.mPath)
    {
    }

    xdirinfo::xdirinfo(const xdirpath& dir)
        : mPath(dir)
    {
    }

    bool            xdirinfo::isRoot() const { return mPath.isRoot(); }
    bool            xdirinfo::isRooted() const { return mPath.isRooted(); }
    bool            xdirinfo::exists() const { return sExists(mPath); }
    bool            xdirinfo::create() { return sCreate(mPath); }
    bool            xdirinfo::remove() { return sDelete(mPath); }
    void            xdirinfo::refresh() {}
    void            xdirinfo::copy(const xdirpath& toDirectory, xbool overwrite) { sCopy(mPath, toDirectory, overwrite); }
    void            xdirinfo::move(const xdirpath& toDirectory) { sMove(mPath, toDirectory); }
    void            xdirinfo::enumerate(enumerate_delegate& enumerator) { sEnumerate(mPath, enumerator); }
    xdirpath const& xdirinfo::getDirpath() const { return mPath; }
    bool            xdirinfo::getRoot(xdirinfo& outRootDirPath) const { return (mPath.getRoot(outRootDirPath.mPath)); }
    bool            xdirinfo::getParent(xdirinfo& outParentDirPath) const { return (mPath.getParent(outParentDirPath.mPath)); }
    bool            xdirinfo::getTimes(xfiletimes& times) const { return sGetTime(mPath, times); }
    bool            xdirinfo::setTimes(xfiletimes times) { return sSetTime(mPath, times); }
    bool            xdirinfo::getAttrs(xfileattrs& fattrs) const { return sGetAttrs(mPath, fattrs); }
    bool            xdirinfo::setAttrs(xfileattrs fattrs) { return sSetAttrs(mPath, fattrs); }

    xdirinfo& xdirinfo::operator=(const xdirinfo& other)
    {
        if (this == &other)
            return *this;

        mPath = other.mPath;
        return *this;
    }

    xdirinfo& xdirinfo::operator=(const xdirpath& other)
    {
        if (&mPath == &other)
            return *this;

        mPath = other;
        return *this;
    }

    bool xdirinfo::operator==(const xdirpath& other) const { return mPath == other; }
    bool xdirinfo::operator!=(const xdirpath& other) const { return mPath != other; }
    bool xdirinfo::operator==(const xdirinfo& other) const { return mPath == other.mPath; }
    bool xdirinfo::operator!=(const xdirinfo& other) const { return mPath != other.mPath; }

    // Static functions
    bool xdirinfo::sCreate(const xdirpath& dirpath)
    {
        xfiledevice* device;
        xdirpath     syspath = dirpath.mParent->resolve(dirpath, device);
        return (device != nullptr && device->createDir(syspath));
    }

    bool xdirinfo::sDelete(const xdirpath& dirpath)
    {
        xfiledevice* device;
        xdirpath     syspath = dirpath.mParent->resolve(dirpath, device);
        return (device != nullptr && device->deleteDir(syspath));
    }

    bool xdirinfo::sExists(const xdirpath& dirpath)
    {
        xfiledevice* device;
        xdirpath     syspath = dirpath.mParent->resolve(dirpath, device);
        return (device != nullptr && device->hasDir(syspath));
    }

    void xdirinfo::sEnumerate(const xdirpath& dirpath, enumerate_delegate& enumerator)
    {
        xfiledevice* device;
        xdirpath     syspath = dirpath.mParent->resolve(dirpath, device);
        if (device != nullptr)
            device->enumerate(syspath, enumerator);
    }

    bool xdirinfo::sSetTime(const xdirpath& dirpath, const xfiletimes& ftimes)
    {
        xfiledevice* device;
        xdirpath     syspath = dirpath.mParent->resolve(dirpath, device);
        if (device != nullptr)
            return device->setDirTime(syspath, ftimes);
        return false;
    }

    bool xdirinfo::sGetTime(const xdirpath& dirpath, xfiletimes& ftimes)
    {
        xfiledevice* device;
        xdirpath     syspath = dirpath.mParent->resolve(dirpath, device);
        if (device != nullptr)
            return device->setDirTime(syspath, ftimes);

        if (device != NULL && device->getDirTime(syspath, ftimes))
            return true;

        ftimes = xfiletimes();
        return false;
    }

    bool xdirinfo::sSetAttrs(const xdirpath& dirpath, const xfileattrs& fattrs)
    {
        xfiledevice* device;
        xdirpath     syspath = dirpath.mParent->resolve(dirpath, device);
        if (device != nullptr)
            return device->setDirAttr(syspath, fattrs);
        return false;
    }

    bool xdirinfo::sGetAttrs(const xdirpath& dirpath, xfileattrs& fattrs)
    {
        xfiledevice* device;
        xdirpath     syspath = dirpath.mParent->resolve(dirpath, device);
        if (device != nullptr)
        {
            return device->getDirAttr(syspath, fattrs);
        }
        fattrs = xfileattrs();
        return false;
    }

    bool xdirinfo::sCopy(const xdirpath& srcdirpath, const xdirpath& dstdirpath, xbool overwrite)
    {
        xfiledevice* srcdevice;
        xdirpath     srcsyspath = srcdirpath.mParent->resolve(srcdirpath, srcdevice);

        xfiledevice* dstdevice;
        xdirpath     dstsyspath = dstdirpath.mParent->resolve(dstdirpath, dstdevice);

        if (srcdevice != nullptr && dstdevice != nullptr)
            return srcdevice->copyDir(srcdirpath, dstdirpath, overwrite);

        return false;
    }

    bool xdirinfo::sMove(const xdirpath& srcdirpath, const xdirpath& dstdirpath, xbool overwrite)
    {
        xfiledevice* srcdevice;
        xdirpath     srcsyspath = srcdirpath.mParent->resolve(srcdirpath, srcdevice);

        xfiledevice* dstdevice;
        xdirpath     dstsyspath = dstdirpath.mParent->resolve(dstdirpath, dstdevice);

        if (srcdevice != nullptr && dstdevice != nullptr)
            return srcdevice->moveDir(srcdirpath, dstdirpath, overwrite);

        return false;
    }

}; // namespace xcore
