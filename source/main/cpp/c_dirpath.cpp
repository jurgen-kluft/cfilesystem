#include "ccore/c_target.h"
#include "ccore/c_debug.h"
#include "cbase/c_integer.h"
#include "cbase/c_runes.h"

#include "cfilesystem/c_filepath.h"
#include "cfilesystem/c_dirpath.h"
#include "cfilesystem/private/c_filesystem.h"

namespace ncore
{
    //==============================================================================
    // dirpath_t: "Device:\\Folder\Folder\"
    //==============================================================================

    dirpath_t::dirpath_t() : m_device(filesys_t::sNilDevice), m_path(filesys_t::sNilNode) {}

    dirpath_t::dirpath_t(dirpath_t const& other)
    {
        m_device = other.m_device->attach();
        m_path   = other.m_device->m_root->m_paths->attach(other.m_path);
    }
    dirpath_t::dirpath_t(pathdevice_t* device)
    {
        m_device = device->attach();
        m_path   = filesys_t::sNilNode;
    }
    dirpath_t::dirpath_t(pathdevice_t* device, pathnode_t* path)
    {
        m_device = m_device->m_root->m_paths->attach(device);
        m_path   = m_device->m_root->m_paths->attach(path);
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
        m_device = filesys_t::sNilDevice;
        m_path   = filesys_t::sNilNode;
    }

    bool dirpath_t::isEmpty() const { return m_device == filesys_t::sNilDevice && m_path == filesys_t::sNilNode; }
    bool dirpath_t::isRoot() const { return m_device != filesys_t::sNilDevice && m_path != filesys_t::sNilNode; }
    bool dirpath_t::isRooted() const { return m_device != filesys_t::sNilDevice; }

    void dirpath_t::makeRelativeTo(const dirpath_t& dirpath)
    {
        // try and find an overlap of folder
        //   this    = a b c d [e f]
        //   dirpath = [e f] g h i j
        //   overlap = [e f]
        //   result  = g h i j

        // TODO
    }

    void dirpath_t::makeAbsoluteTo(const dirpath_t& dirpath)
    {
        //   this    = a b c d [e f]
        //   dirpath = [e f] g h i j k
        //   overlap = [e f]
        //   result  = a b c d [e f] g h i j k

        // TODO
    }

    void dirpath_t::getSubDir(const dirpath_t& parentpath, const dirpath_t& path, dirpath_t& out_subpath)
    {
        //   parent  = [a b c d e f]
        //   path    = [a b c d e f] g h i j
        //   overlap = [a b c d e f]
        //   subpath = g h i j

        // TODO
    }

    dirpath_t dirpath_t::device() const { return dirpath_t(m_device); }

    dirpath_t dirpath_t::root() const
    {
        dirpath_t   dp(m_device);
        filesys_t*  root = m_device->m_root;
        pathnode_t* left = nullptr;
        dp.m_path        = m_device->m_root->m_paths->attach(left);
        return dp;
    }

    dirpath_t dirpath_t::parent() const
    {
        filesys_t* root = m_device->m_root;
        dirpath_t  dp(m_device);
        dp.m_path = root->get_parent_path(m_path);
        dp.m_path = m_device->m_root->m_paths->attach(dp.m_path);
        return dp;
    }

    pathstr_t* dirpath_t::basename() const { return m_path->m_name; }
    pathstr_t* dirpath_t::rootname() const 
    { 
        pathnode_t* iter = m_path;
        while (iter->m_parent != nullptr)
            iter = iter->m_parent;
        return iter->m_name;
    }
    pathstr_t*  dirpath_t::devname() const { return m_device->m_deviceName; }

    s32 dirpath_t::getLevels() const
    {
        pathnode_t* it     = m_path;
        s32         levels = 0;
        while (it != nullptr)
        {
            levels++;
            it = it->m_parent;
        }
        return levels;
    }

    void dirpath_t::down(crunes_t const& folder)
    {
        filesys_t* root       = m_device->m_root;
        pathstr_t* folder_str = root->m_paths->findOrInsert(folder);
        m_path                = root->m_paths->findOrInsert(m_path, folder_str);
    }

    void dirpath_t::up()
    {
        filesys_t* root = m_device->m_root;
        pathnode_t* current = m_path;
        m_path = m_path->m_parent;
        if (m_path == nullptr)
            m_path = root->sNilNode;
        root->release_path(current);
    }

    s32 dirpath_t::compare(const dirpath_t& other) const
    {
        s32 const de = other.m_device->compare(m_device);
        if (de != 0)
            return de;
        s32 const pe = other.m_device->m_root->m_paths->compare(m_path, other.m_path);
        return pe;
    }

    void dirpath_t::to_string(runes_t& str) const
    {
        filesys_t* root = m_device->m_root;
        m_device->to_string(str);
        root->m_paths->to_string(m_path, str);
    }

    s32 dirpath_t::to_strlen() const
    {
        filesys_t* root = m_device->m_root;
        s32        len  = m_device->to_strlen();
        len += root->m_paths->to_strlen(m_path);
        return len;
    }

    dirpath_t& dirpath_t::operator=(dirpath_t const& other)
    {
        filesys_t* root = m_device->m_root;

        root->release_device(m_device);
        root->release_path(m_path);

        m_device = root->m_paths->attach(other.m_device);
        m_path   = root->m_paths->attach(other.m_path);
        return *this;
    }

    dirpath_t operator+(const dirpath_t& left, const dirpath_t& right)
    {
        dirpath_t dp(left);

        return dp;
    }

}; // namespace ncore
