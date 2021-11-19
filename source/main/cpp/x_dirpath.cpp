#include "xbase/x_target.h"
#include "xbase/x_debug.h"
#include "xbase/x_integer.h"
#include "xbase/x_runes.h"

#include "xfilesystem/x_filepath.h"
#include "xfilesystem/x_dirpath.h"
#include "xfilesystem/private/x_filesystem.h"

namespace xcore
{
    //==============================================================================
    // dirpath_t: "Device:\\Folder\Folder\"
    //==============================================================================

    dirpath_t::dirpath_t() : m_device(&filesys_t::sNilDevice), m_path(&filesys_t::sNilPath)
    {}

    dirpath_t::dirpath_t(dirpath_t const& other)
    {
        m_device = other.m_device->attach();
        m_path = other.m_path->attach();
    }
    dirpath_t::dirpath_t(pathdevice_t* device)
    {
        m_device = device->attach();
        m_path = &filesys_t::sNilPath;
    }
    dirpath_t::dirpath_t(pathdevice_t* device, path_t* path)
    {
        m_device = device->attach();
        m_path = path->attach();
    }

    dirpath_t::~dirpath_t()
    {
        filesys_t* root = m_device->m_root;
        root->release_device(m_device);
        root->release_path(m_path);
    }

    void dirpath_t::clear()
    {
        filesys_t* root = m_device->m_root;
        root->release_device(m_device);
        root->release_path(m_path);
        m_device = &filesys_t::sNilDevice;
        m_path = &filesys_t::sNilPath;
    }

    bool dirpath_t::isEmpty() const { return m_device == &filesys_t::sNilDevice && m_path == &filesys_t::sNilPath; }

    bool dirpath_t::isRoot() const
    {
        return m_device != &filesys_t::sNilDevice && m_path != &filesys_t::sNilPath;
    }

    bool dirpath_t::isRooted() const
    {
        return m_device != &filesys_t::sNilDevice;
    }

    void dirpath_t::makeRelativeTo(const dirpath_t& dirpath)
    {
        // try and find an overlap of folder
        //   this    = a b c d [e f]
        //   dirpath = [e f] g h i j
        //   overlap = [e f]
        //   result  = g h i j
        s32 max = xmin(m_path->m_len, dirpath.m_path->m_len);
        for (s32 c=max; c>0; --c)
        {
            s32 sstart = dirpath.m_path->m_len - c;
            s32 i = 0;
            for ( ; i<c; ++i)
            {
                if (dirpath.m_path->m_path[sstart-i] != m_path->m_path[i])
                {
                    break;
                }
            }
            if (i == c)
            {
                // strip of the top 'c' folders
                filesys_t* root = m_device->m_root;
                path_t* right = nullptr;
                root->get_split_path(m_path, c, nullptr, &right);
                if (right != m_path)
                {
                    root->release_path(m_path);
                }
                m_path = right->attach();
                return;
            }
        }
    }

    void dirpath_t::makeAbsoluteTo(const dirpath_t& dirpath)
    {
        //   this    = a b c d [e f]
        //   dirpath = [e f] g h i j k
        //   overlap = [e f]
        //   result  = a b c d [e f] g h i j k
        s32 max = xmin(m_path->m_len, dirpath.m_path->m_len);
        s32 c = max;
        for ( ; c>0; --c)
        {
            s32 sstart = dirpath.m_path->m_len - c;
            s32 i = 0;
            for ( ; i<c; ++i)
            {
                if (dirpath.m_path->m_path[sstart-i] != m_path->m_path[i])
                {
                    break;
                }
            }
            if (i == c)
            {
                break;
            }
        }

        // there was no overlap so just append, dirpath + this
        filesys_t* root = m_device->m_root;
        path_t* path = nullptr;
        root->get_expand_path(dirpath.m_path, c, dirpath.m_path->m_len - c, m_path, 0, m_path->m_len - c, path);
        if (path != m_path)
        {
            root->release_path(m_path);
        }
        m_path = path->attach();
    }

    void dirpath_t::getSubDir(const dirpath_t& parentpath, const dirpath_t& path, dirpath_t& out_subpath)
    {
        //   parent  = [a b c d e f]
        //   path    = [a b c d e f] g h i j
        //   overlap = [a b c d e f]
        //   subpath = g h i j
        s32 max = xmin(path.m_path->m_len, parentpath.m_path->m_len);
        for (s32 c=max; c>0; --c)
        {
            s32 sstart = parentpath.m_path->m_len - c;
            s32 i = 0;
            for ( ; i<c; ++i)
            {
                if (parentpath.m_path->m_path[sstart-i] != path.m_path->m_path[i])
                {
                    break;
                }
            }
            if (i == c)
            {
                // only take the sub folders
                filesys_t* root = path.m_device->m_root;
                path_t* right = nullptr;
                root->get_split_path(path.m_path, c, nullptr, &right);
                out_subpath = dirpath_t(&root->sNilDevice, right);
                return;
            }
        }
    }

    dirpath_t dirpath_t::device() const
    {
        return dirpath_t(m_device); 
    }

    dirpath_t dirpath_t::root() const 
    { 
        dirpath_t dp(m_device); 
        filesys_t* root = m_device->m_root;
        path_t* left = nullptr;
        root->get_split_path(m_path, 1, &left, nullptr);
        dp.m_path = left->attach();
        return dp;
    }

    dirpath_t dirpath_t::parent() const
    {
        filesys_t* root = m_device->m_root;
        dirpath_t dp(m_device);
        dp.m_path = root->get_parent_path(m_path);
        dp.m_path = dp.m_path->attach();
        return dp;
    }

    dirpath_t dirpath_t::relative() const
    {
        filesys_t* root = m_device->m_root;
        dirpath_t dp(&root->sNilDevice);
        dp.m_path = m_path->attach();
        return dp;
    }

    dirpath_t dirpath_t::base() const
    {
        filesys_t* root = m_device->m_root;
        dirpath_t dp(m_device);
        path_t* right = &root->sNilPath;
        if (m_path->m_len > 0)
        {
            root->get_split_path(m_path, m_path->m_len - 1, nullptr, &right);
        }
        dp.m_path = right->attach();
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
        return &filesys_t::sNilName;
    }

    pathname_t* dirpath_t::devname() const
    {
        return m_device->m_deviceName;
    }

    s32 dirpath_t::getLevels() const
    {
        return m_path->m_len;
    }

    s32 dirpath_t::getLevelOf(dirpath_t const& parent) const
    {
        if (parent.m_device == m_device)
        {
            if (m_path->m_len >= parent.m_path->m_len)
            {
                s32 d = parent.m_path->m_len;
                for (s32 i = 0; i < d; i++)
                {
                    if (parent.m_path->m_path[i] == m_path->m_path[i])
                    {
                        return -1;
                    }
                }
                return m_path->m_len - parent.m_path->m_len;
            }
        }
        return -1;
    }

    void dirpath_t::split(s32 pivot, dirpath_t& left, dirpath_t& right) const
    {
        if (pivot == 0)
        {
            filesys_t* root = m_device->m_root;
            root->release_device(left.m_device);
            root->release_path(left.m_path);
            left.m_device = m_device->attach();
            left.m_path = m_path->attach();

            root->release_device(right.m_device);
            root->release_path(right.m_path);
            right.m_device = m_device->attach();
            right.m_path = &filesys_t::sNilPath;
        }
        else if (pivot == m_path->m_len)
        {
            filesys_t* root = m_device->m_root;
            root->release_device(left.m_device);
            root->release_path(left.m_path);
            left.m_device = m_device->attach();
            left.m_path = &filesys_t::sNilPath;

            root->release_device(right.m_device);
            root->release_path(right.m_path);
            right.m_device = m_device->attach();
            right.m_path = m_path->attach();
        }
        else if (pivot > 0 && pivot < m_path->m_len)
        {
            filesys_t* root = m_device->m_root;
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
        filesys_t* root = m_device->m_root;
        root->release_device(dirpath.m_device);
        root->release_path(dirpath.m_path);
        dirpath.m_device = m_device->attach();
        if (m_path->m_len <= 1)
        {
            dirpath.m_path = &filesys_t::sNilPath;
            folder = &filesys_t::sNilName;
        }
        else
        {
            root->get_split_path(m_path, m_path->m_len - 1, &dirpath.m_path, nullptr);
            dirpath.m_path = dirpath.m_path->attach();
            folder = m_path->m_path[m_path->m_len - 1]->attach();
        }
    }

    void dirpath_t::truncate(pathname_t*& folder, dirpath_t& dirpath) const
    {
        filesys_t* root = m_device->m_root;
        root->release_device(dirpath.m_device);
        root->release_path(dirpath.m_path);
        dirpath.m_device = m_device->attach();
        if (m_path->m_len <= 1)
        {
            dirpath.m_path = &filesys_t::sNilPath;
            folder = &filesys_t::sNilName;
        }
        else
        {
            root->get_split_path(m_path, 1, nullptr, &dirpath.m_path);
            dirpath.m_path = dirpath.m_path->attach();
            folder = m_path->m_path[0]->attach();
        }
    }

    void dirpath_t::combine(pathname_t* folder, dirpath_t const& dirpath)
    {
        filesys_t* root = m_device->m_root;
        root->release_device(m_device);
        root->release_path(m_path);
        m_device = dirpath.m_device->attach();
        m_path = dirpath.m_path->prepend(folder, root->m_allocator);
        m_path = m_path->attach();
    }

    void dirpath_t::combine(dirpath_t const& dirpath, pathname_t* folder)
    {
        filesys_t* root = m_device->m_root;
        root->release_device(m_device);
        root->release_path(m_path);
        m_device = dirpath.m_device->attach();
        m_path = dirpath.m_path->append(folder, root->m_allocator);
        m_path = m_path->attach();
    }

    void dirpath_t::down(pathname_t* folder)
    {
        filesys_t* root = m_device->m_root;
        path_t* newpath = m_path->append(folder, root->m_allocator);
        root->release_path(m_path);
        m_path = newpath->attach();
    }

    void dirpath_t::up()
    {
        filesys_t* root = m_device->m_root;
        path_t* newpath = root->get_parent_path(m_path);
        root->release_path(m_path);
        m_path = newpath->attach();
    }

    s32 dirpath_t::compare(const dirpath_t& other) const
    {
        s32 const de = other.m_device->compare(m_device);
        if (de != 0)
            return de;
        s32 const pe = m_path->compare(other.m_path);
        return pe;
    }

    void dirpath_t::to_string(runes_t& str) const
    {
        filesys_t* root = m_device->m_root;
        m_device->to_string(str);
        for (s32 i = 0; i < m_path->m_len; i++)
        {
            pathname_t* pname = m_path->m_path[i];
            crunes_t namestr(pname->m_name, pname->m_len);
            xcore::concatenate(str, namestr);
            str += (utf32::rune)'\\';
        }
    }

    dirpath_t& dirpath_t::operator=(dirpath_t const& other)
    {
        filesys_t* root = m_device->m_root;
        
        root->release_device(m_device);
        root->release_path(m_path);

        m_device = other.m_device->attach();
        m_path = other.m_path->attach();

        return *this;
    }

    dirpath_t operator+(const dirpath_t& left, const dirpath_t& right)
    {
        dirpath_t dp(left);

        return dp;
    }

}; // namespace xcore
