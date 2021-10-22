#include "xbase/x_target.h"
#include "xbase/x_debug.h"
#include "xbase/x_runes.h"

#include "xfilesystem/x_filepath.h"
#include "xfilesystem/x_dirpath.h"
#include "xfilesystem/private/x_filesystem.h"

namespace xcore
{
    //==============================================================================
    // dirpath_t: "Device:\\Folder\Folder\"
    //==============================================================================

    dirpath_t::dirpath_t() : m_device(filesysroot_t::sNilDevice)
    {
        m_device->attach();
        m_path = filesysroot_t::sNilPath;
    }
    dirpath_t::dirpath_t(dirpath_t const& other)
    {
        m_device = m_device->attach();
        m_path = other.m_path->attach();
    }
    dirpath_t::dirpath_t(pathdevice_t* device)
    {
        m_device = device->attach();
        m_path = filesysroot_t::sNilPath;
    }
    dirpath_t::~dirpath_t()
    {
        filesysroot_t* root = m_device->m_root;
        root->release_device(m_device);
        root->release_path(m_path);
    }

    void dirpath_t::clear()
    {
        filesysroot_t* root = m_device->m_root;
        root->release_device(m_device);
        root->release_path(m_path);
        m_device = filesysroot_t::sNilDevice;
        m_path = filesysroot_t::sNilPath;
    }

    bool dirpath_t::isEmpty() const { return m_device == filesysroot_t::sNilDevice && m_path == filesysroot_t::sNilPath; }

    bool dirpath_t::isRoot() const
    {
        return m_device != filesysroot_t::sNilDevice && m_path != filesysroot_t::sNilPath;
    }

    bool dirpath_t::isRooted() const
    {
        return m_device != filesysroot_t::sNilDevice;
    }

    void dirpath_t::makeRelativeTo(const dirpath_t& dirpath)
    {
        // @TODO
    }
    void dirpath_t::makeAbsoluteTo(const dirpath_t& dirpath)
    {
        // @TODO
    }

    dirpath_t dirpath_t::device() const
    {
        return dirpath_t(m_device); 
    }

    dirpath_t dirpath_t::root() const 
    { 
        dirpath_t dp(m_device); 
        filesysroot_t* root = m_device->m_root;
        root->get_split_path(m_path, 1, &dp.m_path, nullptr);
        return dp;
    }

    dirpath_t dirpath_t::parent() const
    {
        filesysroot_t* root = m_device->m_root;
        dirpath_t dp(m_device);
        dp.m_path = root->get_parent_path(m_path);
        dp.m_path = dp.m_path->attach();
        return dp;
    }

    pathname_t* dirpath_t::basename() const
    {
        return m_path->get_name();
    }

    pathname_t* dirpath_t::rootname() const
    {
        if (m_path->m_len > 0)
        {
            return m_path->m_path[0];
        }
        return filesysroot_t::sNilName;
    }

    pathname_t* dirpath_t::devname() const
    {
        return m_device->m_name;
    }

    void dirpath_t::split(s32 pivot, dirpath_t left, dirpath_t right) const
    {
        if (pivot == 0)
        {
            filesysroot_t* root = m_device->m_root;
            root->release_device(left.m_device);
            root->release_path(left.m_path);
            left.m_device = m_device->attach();
            left.m_path = m_path->attach();

            root->release_device(right.m_device);
            root->release_path(right.m_path);
            right.m_device = m_device->attach();
            right.m_path = filesysroot_t::sNilPath;
        }
        else if (pivot == m_path->m_len)
        {
            filesysroot_t* root = m_device->m_root;
            root->release_device(left.m_device);
            root->release_path(left.m_path);
            left.m_device = m_device->attach();
            left.m_path = filesysroot_t::sNilPath;

            root->release_device(right.m_device);
            root->release_path(right.m_path);
            right.m_device = m_device->attach();
            right.m_path = m_path->attach();
        }
        else if (pivot > 0 && pivot < m_path->m_len)
        {
            filesysroot_t* root = m_device->m_root;
            root->release_device(left.m_device);
            root->release_path(left.m_path);
            root->release_device(right.m_device);
            root->release_path(right.m_path);
            left.m_device = m_device->attach();
            right.m_device = m_device->attach();
            root->get_split_path(m_path, pivot, &left.m_path, &right.m_path);
            left.m_path = left.m_path->attach();
            right.m_path = right.m_path->attach();
        }
    }

    void dirpath_t::truncate(dirpath_t& dirpath, pathname_t*& folder) const
    {
        filesysroot_t* root = m_device->m_root;
        root->release_device(dirpath.m_device);
        root->release_path(dirpath.m_path);
        dirpath.m_device = m_device->attach();
        if (m_path->m_len <= 1)
        {
            dirpath.m_path = filesysroot_t::sNilPath;
            folder = filesysroot_t::sNilName;
        }
        else
        {
            root->get_split_path(m_path, m_path->m_len - 1, &dirpath.m_path, nullptr);
            dirpath.m_path = dirpath.m_path->attach();
            folder = m_path->m_path[m_path->m_len - 1]->incref();
        }
    }

    void dirpath_t::truncate(pathname_t*& folder, dirpath_t& dirpath) const
    {
        filesysroot_t* root = m_device->m_root;
        root->release_device(dirpath.m_device);
        root->release_path(dirpath.m_path);
        dirpath.m_device = m_device->attach();
        if (m_path->m_len <= 1)
        {
            dirpath.m_path = filesysroot_t::sNilPath;
            folder = filesysroot_t::sNilName;
        }
        else
        {
            root->get_split_path(m_path, 1, nullptr, &dirpath.m_path);
            dirpath.m_path = dirpath.m_path->attach();
            folder = m_path->m_path[0]->incref();
        }
    }

    void dirpath_t::combine(pathname_t* folder, dirpath_t const& dirpath)
    {
        filesysroot_t* root = m_device->m_root;
        root->release_device(m_device);
        root->release_path(m_path);
        m_device = dirpath.m_device->attach();
        m_path = m_path->prepend(folder, root->m_allocator);
        m_path = m_path->attach();
    }

    void dirpath_t::combine(dirpath_t const& dirpath, pathname_t* folder)
    {
        filesysroot_t* root = m_device->m_root;
        root->release_device(m_device);
        root->release_path(m_path);
        filesysroot_t* root = m_device->m_root;
        m_device = dirpath.m_device->attach();
        m_path = m_path->append(folder, root->m_allocator);
        m_path = m_path->attach();
    }


    void dirpath_t::to_string(runes_t& str) const
    {
        filesysroot_t* root = m_device->m_root;
        crunes_t devicestr(m_device->m_path->m_name, m_device->m_path->m_len);
        xcore::concatenate(str, devicestr);
        for (s32 i = 0; i < m_path->m_len; i++)
        {
            pathname_t* pname = m_path->m_path[i];
            crunes_t namestr(pname->m_name, pname->m_len);
            xcore::concatenate(str, namestr);
            str += (utf32::rune)'\\';
        }
    }

    bool dirpath_t::operator==(const dirpath_t& other) const
    {
        if (other.m_device == m_device)
        {
            return other.m_path->compare(m_path) == 0;
        }
        return false;
    }

    bool dirpath_t::operator!=(const dirpath_t& other) const
    {
        if (other.m_device != m_device)
            return true;

        if (other.m_path->compare(m_path) != 0)
            return true;
        
        return false;
    }

    
}; // namespace xcore
