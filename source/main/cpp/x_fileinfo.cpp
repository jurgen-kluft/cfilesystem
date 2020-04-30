#include "xbase/x_target.h"
#include "xbase/x_limits.h"
#include "xbase/x_runes.h"

#include "xtime/x_datetime.h"

#include "xfilesystem/x_dirinfo.h"
#include "xfilesystem/x_fileinfo.h"
#include "xfilesystem/x_stream.h"
#include "xfilesystem/x_filesystem.h"
#include "xfilesystem/private/x_filedevice.h"
#include "xfilesystem/private/x_filesystem.h"
#include "xfilesystem/private/x_istream.h"

//==============================================================================
namespace xcore
{
    xfileinfo::xfileinfo() {}

    xfileinfo::xfileinfo(const xfileinfo& dirinfo)
        : mPath(dirinfo.mPath)
    {
    }

    xfileinfo::xfileinfo(const xfilepath& path)
        : mPath(path)
    {
    }

    u64 xfileinfo::getLength() const { return sGetLength(mPath); }

    void xfileinfo::setLength(u64 length) { sSetLength(mPath, length); }

    bool xfileinfo::isValid() const
    {
        xfiledevice* device;
        xfilepath    sysfilepath = mParent->resolve(mPath, device);
        return device != nullptr;
    }

    bool xfileinfo::isRooted() const { return mPath.isRooted(); }

	bool xfileinfo::create(xstream*& outFilestream) const
	{
		return (sCreate(mPath, outFilestream));
	}
    
	bool xfileinfo::exists() const { return sExists(mPath); }

    bool xfileinfo::create()
    {
        xstream* fs;
        if (sCreate(mPath, fs))
        {
            mParent->close(fs);
            return true;
        }
        return false;
    }

    bool xfileinfo::remove() { return sDelete(mPath); }

    void xfileinfo::refresh() {}

    bool xfileinfo::open(xstream*& outFilestream) { return (sCreate(mPath, outFilestream)); }

    bool xfileinfo::openRead(xstream*& outFileStream) { return sOpenRead(mPath, outFileStream); }

    bool xfileinfo::openWrite(xstream*& outFileStream) { return sOpenWrite(mPath, outFileStream); }

    u64 xfileinfo::readAllBytes(xbyte* buffer, u64 count) { return sReadAllBytes(mPath, buffer, count); }

    u64 xfileinfo::writeAllBytes(const xbyte* buffer, u64 count) { return sWriteAllBytes(mPath, buffer, count); }

    bool xfileinfo::getParent(xdirpath& parent) const
    {
        xdirpath dirpath;
        if (mPath.getDirname(dirpath))
        {
            dirpath.getParent(parent);
            return true;
        }
        return false;
    }

    bool xfileinfo::getRoot(xdirpath& outInfo) const
    {
        if (mPath.isRooted())
        {
            mPath.getRoot(outInfo);
            return true;
        }
        return false;
    }

    void xfileinfo::getFilepath(xfilepath& filepath) const { filepath = mPath; }

    xfilepath const& xfileinfo::getFilepath() const { return mPath; }

    void xfileinfo::getDirpath(xdirpath& dirpath) const { mPath.getDirname(dirpath); }

    void xfileinfo::getFilename(xfilepath& filename) const { mPath.getFilename(filename); }

    void xfileinfo::getFilenameWithoutExtension(xfilepath& extension) const { mPath.getFilenameWithoutExtension(extension); }

    void xfileinfo::getExtension(xfilepath& extension) const { mPath.getExtension(extension); }

    bool xfileinfo::copy_to(const xfilepath& toFilename, bool overwrite) { return sCopy(mPath, toFilename, overwrite); }

    bool xfileinfo::move_to(const xfilepath& toFilename, bool overwrite) { return sMove(mPath, toFilename); }

    void xfileinfo::up() { mPath.up(); }

    void xfileinfo::down(xdirpath const& dir) { mPath.down(dir); }

    bool xfileinfo::getAttrs(xfileattrs& fattrs) const { return sGetAttrs(mPath, fattrs); }
    bool xfileinfo::getTimes(xfiletimes& ftimes) const { return sGetTime(mPath, ftimes); }
    bool xfileinfo::setAttrs(xfileattrs fattrs) { return sSetAttrs(mPath, fattrs); }
    bool xfileinfo::setTimes(xfiletimes ftimes) { return sSetTime(mPath, ftimes); }

    xfileinfo& xfileinfo::operator=(const xfileinfo& other)
    {
        if (this == &other)
            return *this;

        mPath = other.mPath;
        return *this;
    }

    xfileinfo& xfileinfo::operator=(const xfilepath& other)
    {
        if (&mPath == &other)
            return *this;

        mPath = other;
        return *this;
    }

    bool xfileinfo::operator==(const xfileinfo& other) const { return mPath == other.mPath; }

    bool xfileinfo::operator!=(const xfileinfo& other) const { return mPath != other.mPath; }

    ///< Static functions
    bool xfileinfo::sExists(const xfilepath& filepath)
    {
        xfiledevice* device;
        xfilepath    syspath = xfilesys::resolve(filepath, device);
        if (device != nullptr)
            return device->hasFile(syspath);
        return false;
    }

	bool xfileinfo::sCreate(const xfilepath& filepath, xstream*& outFileStream)
	{
        xfiledevice* device;
        xfilepath    syspath = xfilesys::resolve(filepath, device);
        if (device != nullptr)
            return device->createStream(syspath, true, true, outFileStream);
        return false;
	}

    bool xfileinfo::sGetFileAttributes(const xfilepath& filepath, xfileattrs& outAttr)
    {
        xfiledevice* device;
        xfilepath    syspath = xfilesys::resolve(filepath, device);

        if (device != nullptr)
            return device->getFileAttr(syspath, outAttr);

        return false;
    }

    bool xfileinfo::sIsArchive(const xfilepath& filename)
    {
        xfileattrs a;
        if (sGetFileAttributes(filename, a))
            return a.isArchive();
        return false;
    }

    bool xfileinfo::sIsReadOnly(const xfilepath& filename)
    {
        xfileattrs a;
        if (sGetFileAttributes(filename, a))
            return a.isReadOnly();
        return false;
    }

    bool xfileinfo::sIsHidden(const xfilepath& filename)
    {
        xfileattrs a;
        if (sGetFileAttributes(filename, a))
            return a.isHidden();
        return false;
    }

    bool xfileinfo::sIsSystem(const xfilepath& filename)
    {
        xfileattrs a;
        if (sGetFileAttributes(filename, a))
            return a.isSystem();
        return false;
    }

    bool xfileinfo::sOpen(const xfilepath& filepath, xstream*& outFileStream)
    {
        xfiledevice* device;
        xfilepath    syspath = xfilesys::resolve(filepath, device);
        if (device != nullptr)
        {
            if (device->createStream(syspath, true, true, outFileStream))
            {
                return true;
            }
        }
        return false;
    }
    bool xfileinfo::sOpenRead(const xfilepath& filepath, xstream*& outFileStream)
    {
        xfiledevice* device;
        xfilepath    syspath = xfilesys::resolve(filepath, device);
        if (device != nullptr)
        {
            if (device->createStream(syspath, true, true, outFileStream))
            {
                return true;
            }
        }
        return false;
    }
    bool xfileinfo::sOpenWrite(const xfilepath& filepath, xstream*& outFileStream)
    {
        xfiledevice* device;
        xfilepath    syspath = xfilesys::resolve(filepath, device);
        if (device != nullptr)
        {
            if (device->createStream(syspath, true, true, outFileStream))
            {
                return true;
            }
        }
        return false;
    }

    bool xfileinfo::sDelete(const xfilepath& filepath)
    {
        xfiledevice* device;
        xfilepath    syspath = xfilesys::resolve(filepath, device);
        if (device != nullptr)
        {
            return device->deleteFile(filepath);
        }
        return false;
    }

    u64 xfileinfo::sGetLength(const xfilepath& filepath)
    {
        u64 fileLength = X_U64_MAX;

        xfiledevice* device;
        xfilepath    syspath = xfilesys::resolve(filepath, device);
        if (device != nullptr)
        {
            void* nFileHandle;
            if (device->openFile(syspath, FileMode_Open, FileAccess_Read, FileOp_Sync, nFileHandle))
            {
                device->getLengthOfFile(nFileHandle, fileLength);
                device->closeFile(nFileHandle);
            }
        }
        return fileLength;
    }

    void xfileinfo::sSetLength(const xfilepath& filepath, u64 fileLength)
    {
        xfiledevice* device;
        xfilepath    syspath = xfilesys::resolve(filepath, device);
        if (device != nullptr)
        {
            void* nFileHandle;
            if (device->openFile(syspath, FileMode_Open, FileAccess_Write, FileOp_Sync, nFileHandle))
            {
                device->setLengthOfFile(nFileHandle, fileLength);
                device->closeFile(nFileHandle);
            }
        }
    }

    u64 xfileinfo::sReadAllBytes(const xfilepath& filepath, xbyte* buffer, u64 count)
    {
        u64       rc     = 0;
        xistream* stream = xfilesys::create_filestream(filepath, FileMode_Open, FileAccess_Read, FileOp_Sync);
        if (stream->isOpen())
        {
            rc = stream->read(buffer, count);
            stream->close();
        }
        xfilesys::destroy(stream);
        return rc;
    }

    u64 xfileinfo::sWriteAllBytes(const xfilepath& filepath, const xbyte* buffer, u64 count)
    {
        u64       rc     = 0;
        xistream* stream = xfilesys::create_filestream(filepath, FileMode_Open, FileAccess_Write, FileOp_Sync);
        if (stream->isOpen())
        {
            stream->setLength(0);
            rc = stream->write(buffer, count);
            stream->close();
        }
        xfilesys::destroy(stream);
        return rc;
    }

    bool xfileinfo::sSetTime(const xfilepath& filepath, const xfiletimes& ftimes)
    {
        xfiledevice* device;
        xfilepath    syspath = xfilesys::resolve(filepath, device);
        if (device != nullptr)
        {
            return device->setFileTime(syspath, ftimes);
        }
        return false;
    }

    bool xfileinfo::sGetTime(const xfilepath& filepath, xfiletimes& ftimes)
    {
        xfiledevice* device;
        xfilepath    syspath = xfilesys::resolve(filepath, device);
        if (device != nullptr)
        {
            return device->getFileTime(syspath, ftimes);
        }
        return false;
    }

    bool xfileinfo::sSetAttrs(const xfilepath& filepath, const xfileattrs& fattrs)
    {
        xfiledevice* device;
        xfilepath    syspath = xfilesys::resolve(filepath, device);
        if (device != nullptr)
        {
            return device->setFileAttr(syspath, fattrs);
        }
        return false;
    }

    bool xfileinfo::sGetAttrs(const xfilepath& filepath, xfileattrs& fattrs)
    {
        xfiledevice* device;
        xfilepath    syspath = xfilesys::resolve(filepath, device);
        if (device != nullptr)
        {
            return device->getFileAttr(syspath, fattrs);
        }
        return false;
    }

    bool xfileinfo::sCopy(const xfilepath& srcfilepath, const xfilepath& dstfilepath, bool overwrite)
    {
        xfiledevice* srcdevice;
        xfilepath    srcsyspath = xfilesys::resolve(srcfilepath, srcdevice);
        xfiledevice* dstdevice;
        xfilepath    dstsyspath = xfilesys::resolve(dstfilepath, dstdevice);

        if (srcdevice != NULL && dstdevice != NULL)
            return srcdevice->copyFile(srcsyspath, dstsyspath, overwrite);

        return false;
    }

    bool xfileinfo::sMove(const xfilepath& srcfilepath, const xfilepath& dstfilepath, bool overwrite)
    {
        xfiledevice* srcdevice;
        xfilepath    srcsyspath = xfilesys::resolve(srcfilepath, srcdevice);
        xfiledevice* dstdevice;
        xfilepath    dstsyspath = xfilesys::resolve(dstfilepath, dstdevice);

        if (srcdevice != NULL && dstdevice != NULL)
            return srcdevice->moveFile(srcsyspath, dstsyspath, overwrite);

        return false;
    }
} // namespace xcore
