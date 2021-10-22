#include "xbase/x_target.h"
#include "xbase/x_debug.h"
#include "xbase/x_runes.h"

#include "xfilesystem/private/x_enumerations.h"
#include "xfilesystem/private/x_path.h"
#include "xfilesystem/x_dirpath.h"
#include "xfilesystem/x_filepath.h"

namespace xcore
{

    filepath_t::filepath_t() : m_dirpath(), m_filename(filesysroot_t::sNilName), m_extension(filesysroot_t::sNilName) {}
    filepath_t::filepath_t(pathname_t* filename, pathname_t* extension) : m_dirpath(), m_filename(filename), m_extension(extension) {}
    filepath_t::filepath_t(dirpath_t dirpath, pathname_t* filename, pathname_t* extension) : m_dirpath(dirpath), m_filename(filename), m_extension(extension) {}

    filepath_t::~filepath_t()
    {
        filesysroot_t* root = m_dirpath.m_device->m_root;
        root->release_filename(m_filename);
        root->release_extension(m_extension);
    }

    void filepath_t::clear()
    {
        m_dirpath.clear();
        filesysroot_t* root = m_dirpath.m_device->m_root;
        root->release_filename(m_filename);
        root->release_extension(m_extension);
        m_filename = filesysroot_t::sNilName;
        m_extension = filesysroot_t::sNilName;
    }

    bool filepath_t::isRooted() const { return m_dirpath.isRooted(); }
    bool filepath_t::isEmpty() const { return m_dirpath.isEmpty() && m_filename == filesysroot_t::sNilName && m_extension == filesysroot_t::sNilName; }

    void filepath_t::setDirpath(dirpath_t const& dirpath)
    {
        m_dirpath = dirpath;
    }

    void filepath_t::setFilename(pathname_t* filename) { filename->incref(); m_filename->release(m_dirpath.m_device->m_root->m_allocator); m_filename = filename; }
    void filepath_t::setFilename(crunes_t const& filenamestr)
    {
        filesysroot_t* root = m_dirpath.m_device->m_root;
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
        filesysroot_t* root = m_dirpath.m_device->m_root;
        pathname_t* out_extension = root->register_extension(extensionstr);
        root->release_extension(m_extension);
        m_extension = out_extension->incref();
    }

    dirpath_t filepath_t::root() const { return m_dirpath.root(); }
    dirpath_t filepath_t::dirpath() const { return m_dirpath; }
    pathname_t*   filepath_t::dirname() const { return m_dirpath.basename(); }
    pathname_t*   filepath_t::filename() const { return m_filename; }
    pathname_t*   filepath_t::extension() const { return m_extension; }

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
        filesysroot_t* root = m_dirpath.m_device->m_root;
        root->get_split_path(m_dirpath.m_path, m_dirpath.m_path->m_len - 1, &left.m_dirpath.m_path, nullptr);
        left.m_dirpath.m_device = m_dirpath.m_device->attach();
        left.m_filename = m_filename->incref();
        left.m_extension = m_extension->incref();
    }

    void filepath_t::truncate(pathname_t*& folder, filepath_t& filepath) const
    {
        dirpath.clear();
        folder = m_dirpath.rootname();
        filesysroot_t* root = m_dirpath.m_device->m_root;
        root->get_split_path(m_dirpath.m_path, m_dirpath.m_path->m_len - 1, &dirpath.m_dirpath.m_path, nullptr);
        dirpath.m_dirpath.m_device = m_dirpath.m_device->attach();
        dirpath.m_filename = m_filename->incref();
        dirpath.m_extension = m_extension->incref();
    }

    void filepath_t::combine(pathname_t* folder, filepath_t const& filepath)
    {

    }

    void filepath_t::combine(filepath_t const& filepath, pathname_t* folder)
    {

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

} // namespace xcore
