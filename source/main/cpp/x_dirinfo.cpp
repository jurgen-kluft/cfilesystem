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

    xdirinfo::xdirinfo(const xdirinfo& dirinfo) : mDirPath(dirinfo.mDirPath) {}

    xdirinfo::xdirinfo(const xdirpath& dir) : mDirPath(dir) {}

    bool xdirinfo::isRoot() const { return mDirPath.isRoot(); }

    bool xdirinfo::isRooted() const { return mDirPath.isRooted(); }

    bool xdirinfo::exists() const { return sExists(mDirPath); }

    bool xdirinfo::create() { return sCreate(mDirPath); }

    bool xdirinfo::remove() { return sDelete(mDirPath); }

    void xdirinfo::refresh() {}

    void xdirinfo::copy(const xdirpath& toDirectory, xbool overwrite) { sCopy(mDirPath, toDirectory, overwrite); }

    void xdirinfo::move(const xdirpath& toDirectory) { sMove(mDirPath, toDirectory); }

    void xdirinfo::enumerate(enumerate_delegate& enumerator) { sEnumerate(mDirPath, enumerator); }

    xdirpath const& xdirinfo::getDirpath() const { return mDirPath; }

    bool xdirinfo::getRoot(xdirinfo& outRootDirPath) const { return (mDirPath.getRoot(outRootDirPath.mDirPath)); }

    bool xdirinfo::getParent(xdirinfo& outParentDirPath) const { return (mDirPath.getParent(outParentDirPath.mDirPath)); }

    bool xdirinfo::getTimes(xfiletimes& times) const { return sGetTime(mDirPath, times); }

    bool xdirinfo::setTimes(xfiletimes times) { return sSetTime(mDirPath, times); }

    bool xdirinfo::getAttrs(xfileattrs& fattrs) const { return sGetAttrs(mDirPath, fattrs); }

    bool xdirinfo::setAttrs(xfileattrs fattrs) { return sSetAttrs(mDirPath, fattrs); }

    xdirinfo& xdirinfo::operator=(const xdirinfo& other)
    {
        if (this == &other)
            return *this;

        mDirPath = other.mDirPath;
        return *this;
    }

    xdirinfo& xdirinfo::operator=(const xdirpath& other)
    {
        if (&mDirPath == &other)
            return *this;

        mDirPath = other;
        return *this;
    }

    bool xdirinfo::operator==(const xdirpath& other) const { return mDirPath == other; }
    bool xdirinfo::operator!=(const xdirpath& other) const { return mDirPath != other; }
    bool xdirinfo::operator==(const xdirinfo& other) const { return mDirPath == other.mDirPath; }
    bool xdirinfo::operator!=(const xdirinfo& other) const { return mDirPath != other.mDirPath; }

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
