#include "xbase/x_target.h"
#include "xbase/x_limits.h"
#include "xbase/x_runes.h"

#include "xtime/x_datetime.h"

#include "xfilesystem/x_dirpath.h"
#include "xfilesystem/x_filepath.h"
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
    fileinfo_t::fileinfo_t() : mFileExists(false), mFileTimes(), mFileAttributes(), mParent(nullptr), mPath() {}
    fileinfo_t::fileinfo_t(const fileinfo_t& fi) : mFileExists(fi.mFileExists), mFileTimes(fi.mFileTimes), mFileAttributes(fi.mFileAttributes), mParent(fi.mParent), mPath(fi.mPath) {}
    fileinfo_t::fileinfo_t(const filepath_t& fp) : mParent(fp.mParent), mPath(fp) {}

    u64  fileinfo_t::getLength() const { return sGetLength(mPath); }
    void fileinfo_t::setLength(u64 length) { sSetLength(mPath, length); }

    bool fileinfo_t::isValid() const
    {
        filedevice_t* device;
        filepath_t    sysfilepath = mParent->resolve(mPath, device);
        return device != nullptr;
    }

    bool fileinfo_t::isRooted() const { return mPath.isRooted(); }
    bool fileinfo_t::exists() const { return sExists(mPath); }
    bool fileinfo_t::create(stream_t*& outFilestream) const { return (sCreate(mPath, outFilestream)); }

    bool fileinfo_t::create()
    {
        stream_t* fs;
        if (sCreate(mPath, fs))
        {
            mParent->close(fs);
            return true;
        }
        return false;
    }

    bool fileinfo_t::remove() { return sDelete(mPath); }
    void fileinfo_t::refresh() {}
    bool fileinfo_t::open(stream_t*& outFilestream) { return (sCreate(mPath, outFilestream)); }
    bool fileinfo_t::openRead(stream_t*& outFileStream) { return sOpenRead(mPath, outFileStream); }
    bool fileinfo_t::openWrite(stream_t*& outFileStream) { return sOpenWrite(mPath, outFileStream); }
    u64  fileinfo_t::readAllBytes(xbyte* buffer, u64 count) { return sReadAllBytes(mPath, buffer, count); }
    u64  fileinfo_t::writeAllBytes(const xbyte* buffer, u64 count) { return sWriteAllBytes(mPath, buffer, count); }

    bool fileinfo_t::getParent(dirpath_t& parent) const
    {
        dirpath_t dirpath;
        if (mPath.getDirname(dirpath))
        {
            dirpath.getParent(parent);
            return true;
        }
        return false;
    }

    bool fileinfo_t::getRoot(dirpath_t& outInfo) const
    {
        if (mPath.isRooted())
        {
            mPath.getRoot(outInfo);
            return true;
        }
        return false;
    }

    void fileinfo_t::getDirpath(dirpath_t& dirpath) const { mPath.getDirname(dirpath); }
    void fileinfo_t::getFilename(filepath_t& filename) const { mPath.getFilename(filename); }
    void fileinfo_t::getFilenameWithoutExtension(filepath_t& extension) const { mPath.getFilenameWithoutExtension(extension); }
    void fileinfo_t::getExtension(filepath_t& extension) const { mPath.getExtension(extension); }

    bool fileinfo_t::copy_to(const filepath_t& toFilename, bool overwrite) { return sCopy(mPath, toFilename, overwrite); }
    bool fileinfo_t::move_to(const filepath_t& toFilename, bool overwrite) { return sMove(mPath, toFilename); }

    dirpath_t fileinfo_t::getParent() const
    {
        dirpath_t dp;
        if (getParent(dp))
        {
            return dp;
        }
        path_t nullpath(mParent->m_stralloc);
        return dirpath_t(mParent, nullpath);
    }

    dirpath_t fileinfo_t::getRoot() const
    {
        dirpath_t dp;
        if (getRoot(dp))
        {
            return dp;
        }
        path_t nullpath(mParent->m_stralloc);
        return dirpath_t(mParent, nullpath);
    }

    dirpath_t fileinfo_t::getDirpath() const
    {
        dirpath_t dp;
        getDirpath(dp);
        return dp;
    }

    filepath_t fileinfo_t::getFilename() const
    {
        filepath_t fp;
        getFilename(fp);
        return fp;
    }

    filepath_t fileinfo_t::getFilenameWithoutExtension() const
    {
        filepath_t fp;
        getFilenameWithoutExtension(fp);
        return fp;
    }

    filepath_t fileinfo_t::getExtension() const
    {
        filepath_t fp;
        getExtension(fp);
        return fp;
    }

    void              fileinfo_t::getFilepath(filepath_t& filepath) const { filepath = mPath; }
    filepath_t const& fileinfo_t::getFilepath() const { return mPath; }

    void fileinfo_t::up() { mPath.up(); }
    void fileinfo_t::down(dirpath_t const& dir) { mPath.down(dir); }

    bool fileinfo_t::getAttrs(fileattrs_t& fattrs) const { return sGetAttrs(mPath, fattrs); }
    bool fileinfo_t::getTimes(filetimes_t& ftimes) const { return sGetTime(mPath, ftimes); }
    bool fileinfo_t::setAttrs(fileattrs_t fattrs) { return sSetAttrs(mPath, fattrs); }
    bool fileinfo_t::setTimes(filetimes_t ftimes) { return sSetTime(mPath, ftimes); }

    fileinfo_t& fileinfo_t::operator=(const fileinfo_t& other)
    {
        if (this == &other)
            return *this;

        mPath = other.mPath;
        return *this;
    }

    fileinfo_t& fileinfo_t::operator=(const filepath_t& other)
    {
        if (&mPath == &other)
            return *this;

        mPath = other;
        return *this;
    }

    bool fileinfo_t::operator==(const fileinfo_t& other) const { return mPath == other.mPath; }

    bool fileinfo_t::operator!=(const fileinfo_t& other) const { return mPath != other.mPath; }

    ///< Static functions
    bool fileinfo_t::sExists(const filepath_t& filepath)
    {
        filedevice_t* device;
        filepath_t    syspath = filesys_t::resolve(filepath, device);
        if (device != nullptr)
            return device->hasFile(syspath);
        return false;
    }

    bool fileinfo_t::sCreate(const filepath_t& filepath, stream_t*& outFileStream)
    {
        filedevice_t* device;
        filepath_t    syspath = filesys_t::resolve(filepath, device);
        if (device != nullptr)
            return device->createStream(syspath, true, true, outFileStream);
        return false;
    }

    bool fileinfo_t::sGetFileAttributes(const filepath_t& filepath, fileattrs_t& outAttr)
    {
        filedevice_t* device;
        filepath_t    syspath = filesys_t::resolve(filepath, device);

        if (device != nullptr)
            return device->getFileAttr(syspath, outAttr);

        return false;
    }

    bool fileinfo_t::sIsArchive(const filepath_t& filename)
    {
        fileattrs_t a;
        if (sGetFileAttributes(filename, a))
            return a.isArchive();
        return false;
    }

    bool fileinfo_t::sIsReadOnly(const filepath_t& filename)
    {
        fileattrs_t a;
        if (sGetFileAttributes(filename, a))
            return a.isReadOnly();
        return false;
    }

    bool fileinfo_t::sIsHidden(const filepath_t& filename)
    {
        fileattrs_t a;
        if (sGetFileAttributes(filename, a))
            return a.isHidden();
        return false;
    }

    bool fileinfo_t::sIsSystem(const filepath_t& filename)
    {
        fileattrs_t a;
        if (sGetFileAttributes(filename, a))
            return a.isSystem();
        return false;
    }

    bool fileinfo_t::sOpen(const filepath_t& filepath, stream_t*& outFileStream)
    {
        filedevice_t* device;
        filepath_t    syspath = filesys_t::resolve(filepath, device);
        if (device != nullptr)
        {
            if (device->createStream(syspath, true, true, outFileStream))
            {
                return true;
            }
        }
        return false;
    }
    bool fileinfo_t::sOpenRead(const filepath_t& filepath, stream_t*& outFileStream)
    {
        filedevice_t* device;
        filepath_t    syspath = filesys_t::resolve(filepath, device);
        if (device != nullptr)
        {
            if (device->createStream(syspath, true, true, outFileStream))
            {
                return true;
            }
        }
        return false;
    }
    bool fileinfo_t::sOpenWrite(const filepath_t& filepath, stream_t*& outFileStream)
    {
        filedevice_t* device;
        filepath_t    syspath = filesys_t::resolve(filepath, device);
        if (device != nullptr)
        {
            if (device->createStream(syspath, true, true, outFileStream))
            {
                return true;
            }
        }
        return false;
    }

    bool fileinfo_t::sDelete(const filepath_t& filepath)
    {
        filedevice_t* device;
        filepath_t    syspath = filesys_t::resolve(filepath, device);
        if (device != nullptr)
        {
            return device->deleteFile(filepath);
        }
        return false;
    }

    u64 fileinfo_t::sGetLength(const filepath_t& filepath)
    {
        u64 fileLength = X_U64_MAX;

        filedevice_t* device;
        filepath_t    syspath = filesys_t::resolve(filepath, device);
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

    void fileinfo_t::sSetLength(const filepath_t& filepath, u64 fileLength)
    {
        filedevice_t* device;
        filepath_t    syspath = filesys_t::resolve(filepath, device);
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

    u64 fileinfo_t::sReadAllBytes(const filepath_t& filepath, xbyte* buffer, u64 count)
    {
        u64        rc     = 0;
        istream_t* stream = filesys_t::create_filestream(filepath, FileMode_Open, FileAccess_Read, FileOp_Sync);
        if (stream->isOpen())
        {
            rc = stream->read(buffer, count);
            stream->close();
        }
        filesys_t::destroy(stream);
        return rc;
    }

    u64 fileinfo_t::sWriteAllBytes(const filepath_t& filepath, const xbyte* buffer, u64 count)
    {
        u64        rc     = 0;
        istream_t* stream = filesys_t::create_filestream(filepath, FileMode_Open, FileAccess_Write, FileOp_Sync);
        if (stream->isOpen())
        {
            stream->setLength(0);
            rc = stream->write(buffer, count);
            stream->close();
        }
        filesys_t::destroy(stream);
        return rc;
    }

    bool fileinfo_t::sSetTime(const filepath_t& filepath, const filetimes_t& ftimes)
    {
        filedevice_t* device;
        filepath_t    syspath = filesys_t::resolve(filepath, device);
        if (device != nullptr)
        {
            return device->setFileTime(syspath, ftimes);
        }
        return false;
    }

    bool fileinfo_t::sGetTime(const filepath_t& filepath, filetimes_t& ftimes)
    {
        filedevice_t* device;
        filepath_t    syspath = filesys_t::resolve(filepath, device);
        if (device != nullptr)
        {
            return device->getFileTime(syspath, ftimes);
        }
        return false;
    }

    bool fileinfo_t::sSetAttrs(const filepath_t& filepath, const fileattrs_t& fattrs)
    {
        filedevice_t* device;
        filepath_t    syspath = filesys_t::resolve(filepath, device);
        if (device != nullptr)
        {
            return device->setFileAttr(syspath, fattrs);
        }
        return false;
    }

    bool fileinfo_t::sGetAttrs(const filepath_t& filepath, fileattrs_t& fattrs)
    {
        filedevice_t* device;
        filepath_t    syspath = filesys_t::resolve(filepath, device);
        if (device != nullptr)
        {
            return device->getFileAttr(syspath, fattrs);
        }
        return false;
    }

    bool fileinfo_t::sCopy(const filepath_t& srcfilepath, const filepath_t& dstfilepath, bool overwrite)
    {
        filedevice_t* srcdevice;
        filepath_t    srcsyspath = filesys_t::resolve(srcfilepath, srcdevice);
        filedevice_t* dstdevice;
        filepath_t    dstsyspath = filesys_t::resolve(dstfilepath, dstdevice);

        if (srcdevice != NULL && dstdevice != NULL)
            return srcdevice->copyFile(srcsyspath, dstsyspath, overwrite);

        return false;
    }

    bool fileinfo_t::sMove(const filepath_t& srcfilepath, const filepath_t& dstfilepath, bool overwrite)
    {
        filedevice_t* srcdevice;
        filepath_t    srcsyspath = filesys_t::resolve(srcfilepath, srcdevice);
        filedevice_t* dstdevice;
        filepath_t    dstsyspath = filesys_t::resolve(dstfilepath, dstdevice);

        if (srcdevice != NULL && dstdevice != NULL)
            return srcdevice->moveFile(srcsyspath, dstsyspath, overwrite);

        return false;
    }
} // namespace xcore
