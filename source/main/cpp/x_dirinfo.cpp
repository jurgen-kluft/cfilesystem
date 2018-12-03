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

//==============================================================================
namespace xcore
{
    xdirinfo::xdirinfo() {}

    xdirinfo::xdirinfo(const xdirinfo &dirinfo) : mDirPath(dirinfo.mDirPath) {}

    xdirinfo::xdirinfo(const xdirpath &dir) : mDirPath(dir) {}

    bool xdirinfo::isRoot() const { return mDirPath.isRoot(); }

    bool xdirinfo::isRooted() const { return mDirPath.isRooted(); }

    bool xdirinfo::exists() const { return sExists(mDirPath); }

    bool xdirinfo::create() { return sCreate(mDirPath); }

    bool xdirinfo::remove() { return sDelete(mDirPath); }

    void xdirinfo::refresh() {}

    void xdirinfo::copy(const xdirpath &toDirectory, xbool overwrite) { sCopy(mDirPath, toDirectory, overwrite); }

    void xdirinfo::move(const xdirpath &toDirectory) { sMove(mDirPath, toDirectory); }

    void xdirinfo::enumerate(enumerate_delegate &enumerator) { sEnumerate(mDirPath, enumerator); }

    bool xdirinfo::getDirpath(xdirpath& outDirpath) const
	{
		outDirpath = mDirPath;
		return true;
	}

    bool xdirinfo::getRoot(xdirinfo &outRootDirPath) const { return (mDirPath.getRoot(outRootDirPath.mDirPath)); }

    bool xdirinfo::getParent(xdirinfo &outParentDirPath) const
    {
        return (mDirPath.getParent(outParentDirPath.mDirPath));
    }

    bool xdirinfo::getTimes(xfiletimes& times) const { return sGetTime(mDirPath, times); }

    bool xdirinfo::setTimes(xfiletimes times) { return sSetTime(mDirPath, times); }

    xdirinfo &xdirinfo::operator=(const xdirinfo &other)
    {
        if (this == &other)
            return *this;

        mDirPath = other.mDirPath;
        return *this;
    }

    xdirinfo &xdirinfo::operator=(const xdirpath &other)
    {
        if (&mDirPath == &other)
            return *this;

        mDirPath = other;
        return *this;
    }

    bool xdirinfo::operator==(const xdirpath &other) const { return mDirPath == other; }

    bool xdirinfo::operator!=(const xdirpath &other) const { return mDirPath != other; }

    bool xdirinfo::operator==(const xdirinfo &other) const { return mDirPath == other.mDirPath; }

    bool xdirinfo::operator!=(const xdirinfo &other) const { return mDirPath != other.mDirPath; }

    ///< Static functions
    bool xdirinfo::sCreate(const xdirpath &dirpath)
    {
		xfiledevice* device;
		xdirpath syspath = dirpath.resolve(device);
        return (device != nullptr && device->createDir(syspath));
    }

    bool xdirinfo::sDelete(const xdirpath& dirpath)
    {
		xfiledevice* device;
		xdirpath	 syspath = dirpath.resolve(device);
		return (device != nullptr && device->deleteDir(syspath));
    }

    bool xdirinfo::sExists(const xdirpath& dirpath)
    {
		xfiledevice* device;
		xdirpath	 syspath = dirpath.resolve(device);
		return (device != nullptr && device->hasDir(syspath));
    }

    void xdirinfo::sEnumerate(const xdirpath& dirpath, enumerate_delegate& enumerator)
    {
		xfiledevice* device;
		xdirpath	 syspath = dirpath.resolve(device);
		if (device != nullptr)
			device->enumerate(syspath, &enumerator);
    }

    bool xdirinfo::sSetTime(const xdirpath& dirpath, const xfiletimes& ftimes)
    {
		xfiledevice* device;
		xdirpath	 syspath = dirpath.resolve(device);
		if (device != nullptr)
			return device->setDirTime(syspath, ftimes);
        return false;
    }

    bool xdirinfo::sGetTime(const xdirpath &directory, xfiletimes& ftimes)
    {
		xfiledevice* device;
		xdirpath	 syspath = dirpath.resolve(device);
		if (device != nullptr)
			return device->setDirTime(syspath, ftimes);

		if (device != NULL && device->getDirTime(systemDir.c_str(), creationTime, lastAccessTime, lastWriteTime))
            return true;
        creationTime   = xdatetime::sMinValue;
        lastAccessTime = xdatetime::sMinValue;
        lastWriteTime  = xdatetime::sMinValue;
        return false;
    }

    bool xdirinfo::sSetCreationTime(const xdirpath &directory, const xdatetime &creationTime)
    {
        char         systemDirBuffer[xdirpath::XDIRPATH_BUFFER_SIZE];
        xcstring     systemDir(systemDirBuffer, sizeof(systemDirBuffer));
        xfiledevice *device = directory.getSystem(systemDir);
        if (device != NULL)
        {
            xdatetime _creationTime, _lastAccessTime, _lastWriteTime;
            if (device->getDirTime(systemDir.c_str(), _creationTime, _lastAccessTime, _lastWriteTime))
                if (device->setDirTime(systemDir.c_str(), creationTime, _lastAccessTime, _lastWriteTime))
                    return true;
        }
        return false;
    }

    bool xdirinfo::sGetCreationTime(const xdirpath &directory, xdatetime &outCreationTime)
    {
        char         systemDirBuffer[xdirpath::XDIRPATH_BUFFER_SIZE];
        xcstring     systemDir(systemDirBuffer, sizeof(systemDirBuffer));
        xfiledevice *device = directory.getSystem(systemDir);
        xdatetime    _lastAccessTime, _lastWriteTime;
        if (device != NULL && device->getDirTime(systemDir.c_str(), outCreationTime, _lastAccessTime, _lastWriteTime))
            return true;
        outCreationTime = xdatetime::sMinValue;
        return false;
    }

    bool xdirinfo::sSetLastAccessTime(const xdirpath &directory, const xdatetime &lastAccessTime)
    {
        char         systemDirBuffer[xdirpath::XDIRPATH_BUFFER_SIZE];
        xcstring     systemDir(systemDirBuffer, sizeof(systemDirBuffer));
        xfiledevice *device = directory.getSystem(systemDir);
        if (device != NULL)
        {
            xdatetime _creationTime, _lastAccessTime, _lastWriteTime;
            if (device->getDirTime(systemDir.c_str(), _creationTime, _lastAccessTime, _lastWriteTime))
                if (device->setDirTime(systemDir.c_str(), _creationTime, lastAccessTime, _lastWriteTime))
                    return true;
        }
        return false;
    }

    bool xdirinfo::sGetLastAccessTime(const xdirpath &directory, xdatetime &outLastAccessTime)
    {
        char         systemDirBuffer[xdirpath::XDIRPATH_BUFFER_SIZE];
        xcstring     systemDir(systemDirBuffer, sizeof(systemDirBuffer));
        xfiledevice *device = directory.getSystem(systemDir);
        xdatetime    _creationTime, _lastWriteTime;
        if (device != NULL && device->getDirTime(systemDir.c_str(), _creationTime, outLastAccessTime, _lastWriteTime))
            return true;
        outLastAccessTime = xdatetime::sMinValue;
        return false;
    }

    bool xdirinfo::sSetLastWriteTime(const xdirpath &directory, const xdatetime &lastWriteTime)
    {
        char         systemDirBuffer[xdirpath::XDIRPATH_BUFFER_SIZE];
        xcstring     systemDir(systemDirBuffer, sizeof(systemDirBuffer));
        xfiledevice *device = directory.getSystem(systemDir);
        if (device != NULL)
        {
            xdatetime _creationTime, _lastAccessTime, _lastWriteTime;
            if (device->getDirTime(systemDir.c_str(), _creationTime, _lastAccessTime, _lastWriteTime))
                if (device->setDirTime(systemDir.c_str(), _creationTime, _lastAccessTime, lastWriteTime))
                    return true;
        }
        return false;
    }

    bool xdirinfo::sGetLastWriteTime(const xdirpath &directory, xdatetime &outLastWriteTime)
    {
        char         systemDirBuffer[xdirpath::XDIRPATH_BUFFER_SIZE];
        xcstring     systemDir(systemDirBuffer, sizeof(systemDirBuffer));
        xfiledevice *device = directory.getSystem(systemDir);
        xdatetime    _creationTime, _lastAccessTime;
        if (device != NULL && device->getDirTime(systemDir.c_str(), _creationTime, _lastAccessTime, outLastWriteTime))
            return true;
        outLastWriteTime = xdatetime::sMinValue;
        return false;
    }

    bool xdirinfo::sCopy(const xdirpath &sourceDirectory, const xdirpath &destDirectory, xbool overwrite)
    {
        char         srcSystemDirBuffer[xdirpath::XDIRPATH_BUFFER_SIZE];
        xcstring     srcSystemDir(srcSystemDirBuffer, sizeof(srcSystemDirBuffer));
        xfiledevice *src_device = sourceDirectory.getSystem(srcSystemDir);
        char         dstSystemDirBuffer[xdirpath::XDIRPATH_BUFFER_SIZE];
        xcstring     dstSystemDir(dstSystemDirBuffer, sizeof(dstSystemDirBuffer));
        xfiledevice *dst_device = destDirectory.getSystem(dstSystemDir);

        if (src_device != NULL && dst_device != NULL)
            return src_device->copyDir(srcSystemDir.c_str(), dstSystemDir.c_str(), overwrite);

        return false;
    }

    bool xdirinfo::sMove(const xdirpath &sourceDirectory, const xdirpath &destDirectory)
    {
        char         srcSystemDirBuffer[xdirpath::XDIRPATH_BUFFER_SIZE];
        xcstring     srcSystemDir(srcSystemDirBuffer, sizeof(srcSystemDirBuffer));
        xfiledevice *src_device = sourceDirectory.getSystem(srcSystemDir);
        char         dstSystemDirBuffer[xdirpath::XDIRPATH_BUFFER_SIZE];
        xcstring     dstSystemDir(dstSystemDirBuffer, sizeof(dstSystemDirBuffer));
        xfiledevice *dst_device = destDirectory.getSystem(dstSystemDir);

        if (src_device != NULL && dst_device != NULL)
            return src_device->moveDir(srcSystemDir.c_str(), dstSystemDir.c_str());

        return false;
    }
};
