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
    fileinfo_t::fileinfo_t() : mFileExists(false), mFileTimes(), mFileAttributes(), m_path() {}
    fileinfo_t::fileinfo_t(const fileinfo_t& fi) : mFileExists(fi.mFileExists), mFileTimes(fi.mFileTimes), mFileAttributes(fi.mFileAttributes), m_path(fi.m_path) {}
    fileinfo_t::fileinfo_t(const filepath_t& fp) : m_path(fp) {}
    fileinfo_t::fileinfo_t(const filepath_t& fp, const fileattrs_t& attrs, const filetimes_t& times)
        : m_path(fp), mFileExists(false), mFileTimes(times), mFileAttributes(attrs)
    {
    }
    fileinfo_t::~fileinfo_t() {}

    u64  fileinfo_t::getLength() const { return sGetLength(m_path); }
    void fileinfo_t::setLength(u64 length) { sSetLength(m_path, length); }

    bool fileinfo_t::isValid() const
    {
        pathdevice_t* pathdevice = nullptr;
        path_t* path = nullptr;
        pathname_t* filename = nullptr;
        pathname_t* extension = nullptr;
        filesys_t* root = m_path.m_dirpath.m_device->m_root;
        root->resolve(m_path, pathdevice, path, filename, extension);
        return pathdevice != root->sNilDevice && filename != root->sNilName;
    }

    bool fileinfo_t::isRooted() const { return m_path.isRooted(); }
    bool fileinfo_t::exists() const { return sExists(m_path); }
    bool fileinfo_t::create(stream_t& outFilestream) const { return (sCreate(m_path, outFilestream)); }

    bool fileinfo_t::create()
    {
        stream_t fs;
        if (sCreate(m_path, fs))
        {
            m_path.m_dirpath.m_device->m_root->close(fs);
            return true;
        }
        return false;
    }

    bool fileinfo_t::remove() { return sDelete(m_path); }
    void fileinfo_t::refresh() {}
    bool fileinfo_t::open(stream_t& outFilestream) { return (sCreate(m_path, outFilestream)); }
    bool fileinfo_t::openRead(stream_t& outFileStream) { return sOpenRead(m_path, outFileStream); }
    bool fileinfo_t::openWrite(stream_t& outFileStream) { return sOpenWrite(m_path, outFileStream); }
    u64  fileinfo_t::readAllBytes(xbyte* buffer, u64 count) { return sReadAllBytes(m_path, buffer, count); }
    u64  fileinfo_t::writeAllBytes(const xbyte* buffer, u64 count) { return sWriteAllBytes(m_path, buffer, count); }

    void fileinfo_t::getParent(dirpath_t& parent) const
    {
        filesys_t* root = m_path.m_dirpath.m_device->m_root;
        root->release_device(parent.m_device);
        parent.m_device = m_path.m_dirpath.m_device->attach();
        path_t* parentpath = root->get_parent_path(parent.m_path);
        root->release_path(parent.m_path);
        parent.m_path = parentpath->attach();
    }

    void fileinfo_t::getRoot(dirpath_t& outInfo) const
    {
        filesys_t* root = m_path.m_dirpath.m_device->m_root;
        root->release_device(outInfo.m_device);
        outInfo.m_device = m_path.m_dirpath.m_device->attach();
    }

    void fileinfo_t::getDirpath(dirpath_t& dirpath) const 
    { 
        filesys_t* root = m_path.m_dirpath.m_device->m_root;
        root->release_device(dirpath.m_device);
        dirpath.m_device = m_path.m_dirpath.m_device->attach();
        root->release_path(dirpath.m_path);
        dirpath.m_path = m_path.m_dirpath.m_path->attach();
    }

    void fileinfo_t::getFilename(filepath_t& filename) const
    {
        filesys_t* root = m_path.m_dirpath.m_device->m_root;
        root->release_filename(filename.m_filename); 
        filename.m_filename = m_path.m_filename->incref(); 
        root->release_extension(filename.m_extension);
        filename.m_extension = m_path.m_extension->incref(); 
    }

    void fileinfo_t::getFilenameWithoutExtension(filepath_t& filename_without_extension) const 
    { 
        filesys_t* root = m_path.m_dirpath.m_device->m_root;
        root->release_filename(filename_without_extension.m_filename); 
        filename_without_extension.m_filename = m_path.m_filename->incref(); 
    }

    void fileinfo_t::getExtension(filepath_t& extension) const 
    { 
        filesys_t* root = m_path.m_dirpath.m_device->m_root;
        root->release_extension(extension.m_extension);
        extension.m_extension = m_path.m_extension->incref(); 
    }

    bool fileinfo_t::copy_to(const filepath_t& toFilename, bool overwrite) { return sCopy(m_path, toFilename, overwrite); }
    bool fileinfo_t::move_to(const filepath_t& toFilename, bool overwrite) { return sMove(m_path, toFilename); }

    dirpath_t fileinfo_t::getParent() const
    {
        dirpath_t dp;
        getParent(dp);
        return dp;
    }

    dirpath_t fileinfo_t::getRoot() const
    {
        dirpath_t dp;
        getRoot(dp);
        return dp;
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

    void              fileinfo_t::getFilepath(filepath_t& filepath) const { filepath = m_path; }
    filepath_t const& fileinfo_t::getFilepath() const { return m_path; }

    void fileinfo_t::up() { m_path.up(); }
    void fileinfo_t::down(dirpath_t const& dir) { m_path.m_dirpath = m_path.m_dirpath + dir; }

    bool fileinfo_t::getAttrs(fileattrs_t& fattrs) const { return sGetAttrs(m_path, fattrs); }
    bool fileinfo_t::getTimes(filetimes_t& ftimes) const { return sGetTime(m_path, ftimes); }
    bool fileinfo_t::setAttrs(fileattrs_t fattrs) { return sSetAttrs(m_path, fattrs); }
    bool fileinfo_t::setTimes(filetimes_t ftimes) { return sSetTime(m_path, ftimes); }

    fileinfo_t& fileinfo_t::operator=(const fileinfo_t& other)
    {
        if (this == &other)
            return *this;

        m_path = other.m_path;
        return *this;
    }

    fileinfo_t& fileinfo_t::operator=(const filepath_t& other)
    {
        if (&m_path == &other)
            return *this;

        m_path = other;
        return *this;
    }

    bool fileinfo_t::operator==(const fileinfo_t& other) const { return m_path == other.m_path; }

    bool fileinfo_t::operator!=(const fileinfo_t& other) const { return m_path != other.m_path; }

    ///< Static functions
    bool fileinfo_t::sExists(const filepath_t& filepath)
    {
        filedevice_t* fd = sGetFileDevice(filepath);
        if (fd != nullptr)
            return fd->hasFile(filepath);
        return false;
    }

    bool fileinfo_t::sCreate(const filepath_t& filepath, stream_t& outFileStream)
    {
        filedevice_t* fd = sGetFileDevice(filepath);
        if (fd != nullptr)
            return fd->createStream(filepath, true, true, outFileStream);
        return false;
    }

    filedevice_t* fileinfo_t::sGetFileDevice(const filepath_t& fp)
    {
        pathdevice_t* device = nullptr;
        path_t* path = nullptr;
        pathname_t* filename = nullptr;
        pathname_t* extension = nullptr;
        filesys_t* root = fp.m_dirpath.m_device->m_root;
        root->resolve(fp, device, path, filename, extension);
        return device->m_fd;
    }

    bool fileinfo_t::sGetFileAttributes(const filepath_t& filepath, fileattrs_t& outAttr)
    {
        filedevice_t* fd = sGetFileDevice(filepath);
        if (fd != nullptr)
            return fd->getFileAttr(filepath, outAttr);

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

    bool fileinfo_t::sOpen(const filepath_t& filepath, stream_t& outFileStream)
    {
        filedevice_t* fd = sGetFileDevice(filepath);
        if (fd != nullptr)
        {
            if (fd->createStream(filepath, true, true, outFileStream))
            {
                return true;
            }
        }
        return false;
    }
    bool fileinfo_t::sOpenRead(const filepath_t& filepath, stream_t& outFileStream)
    {
        filedevice_t* fd = sGetFileDevice(filepath);
        if (fd != nullptr)
        {
            if (fd->createStream(filepath, true, true, outFileStream))
            {
                return true;
            }
        }
        return false;
    }
    bool fileinfo_t::sOpenWrite(const filepath_t& filepath, stream_t& outFileStream)
    {
        filedevice_t* fd = sGetFileDevice(filepath);
        if (fd != nullptr)
        {
            if (fd->createStream(filepath, true, true, outFileStream))
            {
                return true;
            }
        }
        return false;
    }

    bool fileinfo_t::sDelete(const filepath_t& filepath)
    {
        filedevice_t* fd = sGetFileDevice(filepath);
        if (fd != nullptr)
        {
            return fd->deleteFile(filepath);
        }
        return false;
    }

    u64 fileinfo_t::sGetLength(const filepath_t& filepath)
    {
        u64 fileLength = X_U64_MAX;

        filedevice_t* fd = sGetFileDevice(filepath);
        if (fd != nullptr)
        {
            void* nFileHandle;
            if (fd->openFile(filepath, FileMode_Open, FileAccess_Read, FileOp_Sync, nFileHandle))
            {
                fd->getLengthOfFile(nFileHandle, fileLength);
                fd->closeFile(nFileHandle);
            }
        }
        return fileLength;
    }

    void fileinfo_t::sSetLength(const filepath_t& filepath, u64 fileLength)
    {
        filedevice_t* fd = sGetFileDevice(filepath);
        if (fd != nullptr)
        {
            void* nFileHandle;
            if (fd->openFile(filepath, FileMode_Open, FileAccess_Write, FileOp_Sync, nFileHandle))
            {
                fd->setLengthOfFile(nFileHandle, fileLength);
                fd->closeFile(nFileHandle);
            }
        }
    }

    u64 fileinfo_t::sReadAllBytes(const filepath_t& filepath, xbyte* buffer, u64 count)
    {
        u64        rc     = 0;
        stream_t stream;
        filesys_t::create_filestream(filepath, FileMode_Open, FileAccess_Read, FileOp_Sync, stream);
        if (stream.isOpen())
        {
            rc = stream.read(buffer, count);
            stream.close();
        }
        filesys_t::destroy(stream);
        return rc;
    }

    u64 fileinfo_t::sWriteAllBytes(const filepath_t& filepath, const xbyte* buffer, u64 count)
    {
        u64        rc     = 0;
        stream_t stream;
        filesys_t::create_filestream(filepath, FileMode_Open, FileAccess_Write, FileOp_Sync, stream);
        if (stream.isOpen())
        {
            stream.setLength(0);
            rc = stream.write(buffer, count);
            stream.close();
        }
        filesys_t::destroy(stream);
        return rc;
    }

    bool fileinfo_t::sSetTime(const filepath_t& filepath, const filetimes_t& ftimes)
    {
        filedevice_t* fd = sGetFileDevice(filepath);
        if (fd != nullptr)
        {
            return fd->setFileTime(filepath, ftimes);
        }
        return false;
    }

    bool fileinfo_t::sGetTime(const filepath_t& filepath, filetimes_t& ftimes)
    {
        filedevice_t* fd = sGetFileDevice(filepath);
        if (fd != nullptr)
        {
            return fd->getFileTime(filepath, ftimes);
        }
        return false;
    }

    bool fileinfo_t::sSetAttrs(const filepath_t& filepath, const fileattrs_t& fattrs)
    {
        filedevice_t* fd = sGetFileDevice(filepath);
        if (fd != nullptr)
        {
            return fd->setFileAttr(filepath, fattrs);
        }
        return false;
    }

    bool fileinfo_t::sGetAttrs(const filepath_t& filepath, fileattrs_t& fattrs)
    {
        filedevice_t* fd = sGetFileDevice(filepath);
        if (fd != nullptr)
        {
            return fd->getFileAttr(filepath, fattrs);
        }
        return false;
    }

    bool fileinfo_t::sCopy(const filepath_t& srcfilepath, const filepath_t& dstfilepath, bool overwrite)
    {
        filedevice_t* sfd = sGetFileDevice(srcfilepath);
        filedevice_t* dfd = sGetFileDevice(dstfilepath);

        if (sfd != NULL && dfd!= NULL)
            return sfd->copyFile(srcfilepath, dstfilepath, overwrite);

        return false;
    }

    bool fileinfo_t::sMove(const filepath_t& srcfilepath, const filepath_t& dstfilepath, bool overwrite)
    {
        filedevice_t* sfd = sGetFileDevice(srcfilepath);
        filedevice_t* dfd = sGetFileDevice(dstfilepath);

        if (sfd != NULL && dfd != NULL)
            return sfd->moveFile(srcfilepath, dstfilepath, overwrite);

        return false;
    }
} // namespace xcore
