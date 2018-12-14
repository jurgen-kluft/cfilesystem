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

    xfileinfo::xfileinfo(const xfileinfo& dirinfo) : mFilePath(dirinfo.mFilePath) {}

    xfileinfo::xfileinfo(const xfilepath& path) : mFilePath(path) {}

    u64 xfileinfo::getLength() const { return sGetLength(mFilePath); }

    void xfileinfo::setLength(u64 length) { sSetLength(mFilePath, length); }

    bool xfileinfo::isValid() const
    {
        xfiledevice* device;
        xfilepath    sysfilepath = mFileSystem->resolve(mFilePath, device);
        return device != nullptr;
    }

    bool xfileinfo::isRooted() const { return mFilePath.isRooted(); }

    bool xfileinfo::exists() const { return sExists(mFilePath); }

    bool xfileinfo::create()
    {
        xstream* fs;
        if (sCreate(mFilePath, fs))
        {
            mFileSystem->close(fs);
            return true;
        }
        return false;
    }


    bool xfileinfo::remove() { return sDelete(mFilePath); }

    void xfileinfo::refresh() {}

	bool xfileinfo::open(xstream*& outFilestream) { return (sCreate(mFilePath, outFilestream)); }

    bool xfileinfo::openRead(xstream*& outFileStream) { return sOpenRead(mFilePath, outFileStream); }

    bool xfileinfo::openWrite(xstream*& outFileStream) { return sOpenWrite(mFilePath, outFileStream); }

    u64 xfileinfo::readAllBytes(xbyte* buffer, u64 count) { return sReadAllBytes(mFilePath, buffer, count); }

    u64 xfileinfo::writeAllBytes(const xbyte* buffer, u64 count) { return sWriteAllBytes(mFilePath, buffer, count); }

    bool xfileinfo::getParent(xdirpath& parent) const
    {
        xdirpath dirpath;
        if (mFilePath.getDirname(dirpath))
        {
            dirpath.getParent(parent);
            return true;
        }
        return false;
    }

    bool xfileinfo::getRoot(xdirpath& outInfo) const
    {
        if (mFilePath.isRooted())
        {
            mFilePath.getRoot(outInfo);
            return true;
        }
        return false;
    }

	void xfileinfo::getFilepath(xfilepath& filepath) const { filepath = mFilePath; }

	xfilepath const& xfileinfo::getFilepath() const { return mFilePath; }

    void xfileinfo::getDirpath(xdirpath& dirpath) const { mFilePath.getDirname(dirpath); }

    void xfileinfo::getFilename(xfilepath& filename) const { mFilePath.getFilename(filename); }

    void xfileinfo::getFilenameWithoutExtension(xfilepath& extension) const
    {
        mFilePath.getFilenameWithoutExtension(extension);
    }

	void xfileinfo::getExtension(xfilepath& extension) const
	{
        mFilePath.getExtension(extension);
	}

    bool xfileinfo::copy_to(const xfilepath& toFilename, bool overwrite) { return sCopy(mFilePath, toFilename, overwrite); }

    bool xfileinfo::move_to(const xfilepath& toFilename, bool overwrite) { return sMove(mFilePath, toFilename); }

    void xfileinfo::up() { mFilePath.up(); }

    void xfileinfo::down(xdirpath const& dir) { mFilePath.down(dir); }

    bool xfileinfo::getTimes(xfiletimes& ftimes) const { return sGetTime(mFilePath, ftimes); }

    bool xfileinfo::setTimes(xfiletimes ftimes) { return sSetTime(mFilePath, ftimes); }

    xfileinfo& xfileinfo::operator=(const xfileinfo& other)
    {
        if (this == &other)
            return *this;

        mFilePath = other.mFilePath;
        return *this;
    }

    xfileinfo& xfileinfo::operator=(const xfilepath& other)
    {
        if (&mFilePath == &other)
            return *this;

        mFilePath = other;
        return *this;
    }

    bool xfileinfo::operator==(const xfileinfo& other) const { return mFilePath == other.mFilePath; }

    bool xfileinfo::operator!=(const xfileinfo& other) const { return mFilePath != other.mFilePath; }

    ///< Static functions
    bool xfileinfo::sExists(const xfilepath& filepath)
    {
        xfiledevice* device;
        xfilepath    syspath = xfilesys::resolve(filepath, device);
        if (device != nullptr)
            return device->hasFile(syspath);
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
            if (device->openFile(syspath, true, false, nFileHandle))
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
            if (device->openFile(syspath, true, false, nFileHandle))
            {
                device->setLengthOfFile(nFileHandle, fileLength);
                device->closeFile(nFileHandle);
            }
        }
    }

    u64 xfileinfo::sReadAllBytes(const xfilepath& filepath, xbyte* buffer, u64 count)
    {
        u64 rc = 0;
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
        u64 rc = 0;
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
