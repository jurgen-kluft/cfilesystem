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
    dirinfo_t::dirinfo_t(const dirinfo_t& dirinfo) : m_path(dirinfo.m_path) {}
    dirinfo_t::dirinfo_t(const dirpath_t& dir) : m_path(dir) {}
    dirinfo_t::~dirinfo_t() {}

    bool             dirinfo_t::isRoot() const { return m_path.isRoot(); }
    bool             dirinfo_t::isRooted() const { return m_path.isRooted(); }
    bool             dirinfo_t::exists() const { return sExists(m_path); }
    bool             dirinfo_t::create() { return sCreate(m_path); }
    bool             dirinfo_t::remove() { return sDelete(m_path); }
    void             dirinfo_t::refresh() {}
    void             dirinfo_t::copy(const dirpath_t& toDirectory, bool overwrite) { sCopy(m_path, toDirectory, overwrite); }
    void             dirinfo_t::move(const dirpath_t& toDirectory) { sMove(m_path, toDirectory); }
    void             dirinfo_t::enumerate(enumerate_delegate_t& enumerator) { sEnumerate(m_path, enumerator); }
    dirpath_t const& dirinfo_t::getDirpath() const { return m_path; }
    bool             dirinfo_t::getRoot(dirinfo_t& outRootDirPath) const { outRootDirPath.m_path = m_path.root(); return true; }
    bool             dirinfo_t::getParent(dirinfo_t& outParentDirPath) const { outParentDirPath.m_path = m_path.parent(); return true; }
    bool             dirinfo_t::getTimes(filetimes_t& times) const { return sGetTime(m_path, times); }
    bool             dirinfo_t::setTimes(filetimes_t times) { return sSetTime(m_path, times); }
    bool             dirinfo_t::getAttrs(fileattrs_t& fattrs) const { return sGetAttrs(m_path, fattrs); }
    bool             dirinfo_t::setAttrs(fileattrs_t fattrs) { return sSetAttrs(m_path, fattrs); }

    dirinfo_t& dirinfo_t::operator=(const dirinfo_t& other)
    {
        if (this == &other)
            return *this;

        m_path = other.m_path;
        return *this;
    }

    dirinfo_t& dirinfo_t::operator=(const dirpath_t& other)
    {
        if (&m_path == &other)
            return *this;

        m_path = other;
        return *this;
    }

    bool dirinfo_t::operator==(const dirpath_t& other) const { return m_path == other; }
    bool dirinfo_t::operator!=(const dirpath_t& other) const { return m_path != other; }
    bool dirinfo_t::operator==(const dirinfo_t& other) const { return m_path == other.m_path; }
    bool dirinfo_t::operator!=(const dirinfo_t& other) const { return m_path != other.m_path; }

    // Static functions
    bool dirinfo_t::sCreate(const dirpath_t& dirpath)
    {
        filedevice_t* device;
        dirpath_t     syspath = dirpath.m_device->m_root->m_owner->resolve(dirpath, device);
        return (device != nullptr && device->createDir(syspath));
    }

    bool dirinfo_t::sDelete(const dirpath_t& dirpath)
    {
        filedevice_t* device;
        dirpath_t     syspath = dirpath.m_device->m_root->m_owner->resolve(dirpath, device);
        return (device != nullptr && device->deleteDir(syspath));
    }

    bool dirinfo_t::sExists(const dirpath_t& dirpath)
    {
        filedevice_t* device;
        dirpath_t     syspath = dirpath.m_device->m_root->m_owner->resolve(dirpath, device);
        return (device != nullptr && device->hasDir(syspath));
    }

    void dirinfo_t::sEnumerate(const dirpath_t& dirpath, enumerate_delegate_t& enumerator)
    {
        filedevice_t* device;
        dirpath_t     syspath = dirpath.m_device->m_root->m_owner->resolve(dirpath, device);
        if (device != nullptr)
            device->enumerate(syspath, enumerator);
    }

    bool dirinfo_t::sSetTime(const dirpath_t& dirpath, const filetimes_t& ftimes)
    {
        filedevice_t* device;
        dirpath_t     syspath = dirpath.m_device->m_root->m_owner->resolve(dirpath, device);
        if (device != nullptr)
            return device->setDirTime(syspath, ftimes);
        return false;
    }

    bool dirinfo_t::sGetTime(const dirpath_t& dirpath, filetimes_t& ftimes)
    {
        filedevice_t* device;
        dirpath_t     syspath = dirpath.m_device->m_root->m_owner->resolve(dirpath, device);
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
        dirpath_t     syspath = dirpath.m_device->m_root->m_owner->resolve(dirpath, device);
        if (device != nullptr)
            return device->setDirAttr(syspath, fattrs);
        return false;
    }

    bool dirinfo_t::sGetAttrs(const dirpath_t& dirpath, fileattrs_t& fattrs)
    {
        filedevice_t* device;
        dirpath_t     syspath = dirpath.m_device->m_root->m_owner->resolve(dirpath, device);
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
        dirpath_t     srcsyspath = srcdirpath.m_device->m_root->m_owner->resolve(srcdirpath, srcdevice);

        filedevice_t* dstdevice;
        dirpath_t     dstsyspath = dstdirpath.m_device->m_root->m_owner->resolve(dstdirpath, dstdevice);

        if (srcdevice != nullptr && dstdevice != nullptr)
            return srcdevice->copyDir(srcdirpath, dstdirpath, overwrite);

        return false;
    }

    bool dirinfo_t::sMove(const dirpath_t& srcdirpath, const dirpath_t& dstdirpath, bool overwrite)
    {
        filedevice_t* srcdevice;
        dirpath_t     srcsyspath = srcdirpath.m_device->m_root->m_owner->resolve(srcdirpath, srcdevice);

        filedevice_t* dstdevice;
        dirpath_t     dstsyspath = dstdirpath.m_device->m_root->m_owner->resolve(dstdirpath, dstdevice);

        if (srcdevice != nullptr && dstdevice != nullptr)
            return srcdevice->moveDir(srcdirpath, dstdirpath, overwrite);

        return false;
    }

}; // namespace xcore
