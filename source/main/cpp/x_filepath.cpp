#include "xbase/x_target.h"
#include "xbase/x_debug.h"
#include "xbase/x_runes.h"

#include "xfilesystem/private/x_enumerations.h"
#include "xfilesystem/private/x_path.h"
#include "xfilesystem/x_dirpath.h"
#include "xfilesystem/x_filepath.h"

namespace xcore
{

    filepath_t::filepath_t() : m_dirpath(), m_filename(filesys_t::sNilName), m_extension(filesys_t::sNilName) {}
    filepath_t::filepath_t(const filepath_t& other)
        : m_dirpath(other.m_dirpath)
    {
        m_filename = other.m_filename->incref();
        m_extension = other.m_extension->incref();
    }
    filepath_t::filepath_t(pathname_t* filename, pathname_t* extension) : m_dirpath(), m_filename(filename), m_extension(extension) {}
    filepath_t::filepath_t(pathdevice_t* device, path_t* path, pathname_t* filename, pathname_t* extension)
        : m_dirpath(device, path), m_filename(filename), m_extension(extension) {}
    filepath_t::filepath_t(dirpath_t const& dirpath, pathname_t* filename, pathname_t* extension) : m_dirpath(dirpath), m_filename(filename), m_extension(extension) {}

    filepath_t::~filepath_t()
    {
        filesys_t* root = m_dirpath.m_device->m_root;
        root->release_filename(m_filename);
        root->release_extension(m_extension);
    }

    void filepath_t::clear()
    {
        m_dirpath.clear();
        filesys_t* root = m_dirpath.m_device->m_root;
        root->release_filename(m_filename);
        root->release_extension(m_extension);
        m_filename = filesys_t::sNilName;
        m_extension = filesys_t::sNilName;
    }

    bool filepath_t::isRooted() const { return m_dirpath.isRooted(); }
    bool filepath_t::isEmpty() const { return m_dirpath.isEmpty() && m_filename == filesys_t::sNilName && m_extension == filesys_t::sNilName; }

    void filepath_t::makeRelativeTo(const dirpath_t& dirpath)
    {
        m_dirpath.makeRelativeTo(dirpath);
    }

    void filepath_t::makeAbsoluteTo(const dirpath_t& dirpath)
    {
        m_dirpath.makeAbsoluteTo(dirpath);
    }

    void filepath_t::setDirpath(dirpath_t const& dirpath)
    {
        m_dirpath = dirpath;
    }

    void filepath_t::setFilename(pathname_t* filename) { filename->incref(); m_filename->release(m_dirpath.m_device->m_root->m_allocator); m_filename = filename; }
    void filepath_t::setFilename(crunes_t const& filenamestr)
    {
        filesys_t* root = m_dirpath.m_device->m_root;
        pathname_t* out_filename = nullptr;
        pathname_t* out_extension = nullptr;
        root->register_filename(filenamestr, out_filename, out_extension);
        root->release_filename(m_filename);
        root->release_extension(m_extension);
        m_filename = out_filename->incref();
        m_extension = out_extension->incref();

    }
    void filepath_t::setExtension(pathname_t* extension) { extension->incref(); m_extension->release(m_dirpath.m_device->m_root->m_allocator); m_extension = extension; }
    void filepath_t::setExtension(crunes_t const& extensionstr)
    {
        filesys_t* root = m_dirpath.m_device->m_root;
        pathname_t* out_extension = root->register_extension(extensionstr);
        root->release_extension(m_extension);
        m_extension = out_extension->incref();
    }

    dirpath_t filepath_t::root() const { return m_dirpath.root(); }
    dirpath_t filepath_t::base() const { return m_dirpath.base(); }
    dirpath_t filepath_t::dirpath() const { return m_dirpath; }

    filepath_t   filepath_t::filename() const { return filepath_t(m_filename, m_extension); }
    filepath_t  filepath_t::relative() const
    {
        filesys_t* root = m_dirpath.m_device->m_root;
        filepath_t fp(root->sNilDevice, m_dirpath.m_path, m_filename, m_extension);
        return fp;
    }

    pathname_t* filepath_t::dirstr() const
    {
        return m_dirpath.m_path->get_name();
    }

    pathname_t* filepath_t::filenamestr() const
    {
        return m_filename;
    }

    pathname_t* filepath_t::extensionstr() const
    {
        return m_filename;
    }

    void filepath_t::split(s32 pivot, dirpath_t& left, filepath_t& right) const
    {
        left.clear();
        right.clear();
        m_dirpath.split(pivot, left, right.m_dirpath);
        left.m_device = m_dirpath.m_device->attach();
        right.m_dirpath.m_device = m_dirpath.m_device->attach();
        right.m_filename = m_filename->incref();
        right.m_extension = m_extension->incref();
    }

    void filepath_t::truncate(filepath_t& left, pathname_t*& folder) const
    {
        left.clear();
        folder = m_dirpath.basename();
        filesys_t* root = m_dirpath.m_device->m_root;
        root->get_split_path(m_dirpath.m_path, m_dirpath.m_path->m_len - 1, &left.m_dirpath.m_path, nullptr);
        left.m_dirpath.m_device = m_dirpath.m_device->attach();
        left.m_filename = m_filename->incref();
        left.m_extension = m_extension->incref();
    }

    void filepath_t::truncate(pathname_t*& folder, filepath_t& filepath) const
    {
        filepath.clear();
        folder = m_dirpath.rootname();
        filesys_t* root = m_dirpath.m_device->m_root;
        root->get_split_path(m_dirpath.m_path, m_dirpath.m_path->m_len - 1, &filepath.m_dirpath.m_path, nullptr);
        filepath.m_dirpath.m_device = m_dirpath.m_device->attach();
        filepath.m_filename = m_filename->incref();
        filepath.m_extension = m_extension->incref();
    }

    void filepath_t::combine(pathname_t* folder, filepath_t const& filepath)
    {
        path_t* newpath = nullptr;
        filesys_t* root = m_dirpath.m_device->m_root;
        root->get_expand_path(folder, m_dirpath.m_path, newpath);
        root->release_path(m_dirpath.m_path);
        m_dirpath.m_path = newpath->attach();
        root->release_filename(m_filename);
        root->release_extension(m_extension);
        m_filename = filepath.m_filename->incref();
        m_extension = filepath.m_extension->incref();
    }

    void filepath_t::combine(filepath_t const& filepath, pathname_t* folder)
    {
        filesys_t* root = m_dirpath.m_device->m_root;
        root->release_device(m_dirpath.m_device);
        root->release_filename(m_filename);
        root->release_extension(m_extension);
        m_filename = filepath.m_filename->incref();
        m_extension = filepath.m_extension->incref();
        down(folder);
    }

    void filepath_t::down(pathname_t* folder)
    {
        path_t* newpath = nullptr;
        filesys_t* root = m_dirpath.m_device->m_root;
        root->get_expand_path(m_dirpath.m_path, folder, newpath);
        root->release_path(m_dirpath.m_path);
        m_dirpath.m_path = newpath->attach();
    }

    void filepath_t::up()
    {
        filesys_t* root = m_dirpath.m_device->m_root;
        path_t* newpath = root->get_parent_path(m_dirpath.m_path);
        root->release_path(m_dirpath.m_path);
        m_dirpath.m_path = newpath->attach();
    }

    void filepath_t::to_string(runes_t& str) const
    {
        m_dirpath.to_string(str);
        crunes_t filenamestr(m_filename->m_name, m_filename->m_len);
        xcore::concatenate(str, filenamestr);
        str += (utf32::rune)'.';
        crunes_t extensionstr(m_extension->m_name, m_extension->m_len);
        xcore::concatenate(str, extensionstr);
    }

    s32 filepath_t::compare(const filepath_t& right) const
    {
        s32 const fe = (m_filename->compare(right.m_filename));
        if (fe != 0)
            return fe;
        s32 const ce = (m_extension->compare(right.m_extension));
        if (ce != 0)
            return ce;
        s32 const de = m_dirpath.compare(right.m_dirpath);
        return de;
    }

    bool operator==(const filepath_t& left, const filepath_t& right)
    {
        return left.compare(right) == 0;
    }

    bool operator!=(const filepath_t& left, const filepath_t& right)
    {
        return left.compare(right) != 0;
    }

    filepath_t operator+(const dirpath_t& dirpath, const filepath_t& filepath)
    {
        return filepath_t(dirpath, filepath.filenamestr(), filepath.extensionstr());
    }

} // namespace xcore
