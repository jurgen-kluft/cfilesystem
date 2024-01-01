#include "ccore/c_target.h"
#include "ccore/c_debug.h"
#include "cbase/c_runes.h"

#include "cfilesystem/private/c_enumerations.h"
#include "cfilesystem/private/c_path.h"
#include "cfilesystem/c_dirpath.h"
#include "cfilesystem/c_filepath.h"

namespace ncore
{
    filepath_t::filepath_t() : m_dirpath(), m_filename(filesys_t::sNilName), m_extension(filesys_t::sNilName) {}
    filepath_t::filepath_t(const filepath_t& other) : m_dirpath(other.m_dirpath)
    {
        m_filename  = m_dirpath.m_device->m_root->m_paths->attach(other.m_filename);
        m_extension = m_dirpath.m_device->m_root->m_paths->attach(other.m_extension);
    }

    filepath_t::filepath_t(pathstr_t* filename, pathstr_t* extension) : m_dirpath()
    {
        m_filename  = m_dirpath.m_device->m_root->m_paths->attach(filename);
        m_extension = m_dirpath.m_device->m_root->m_paths->attach(extension);
    }

    filepath_t::filepath_t(pathdevice_t* device, pathnode_t* path, pathstr_t* filename, pathstr_t* extension) : m_dirpath(device, path)
    {
        m_filename  = m_dirpath.m_device->m_root->m_paths->attach(filename);
        m_extension = m_dirpath.m_device->m_root->m_paths->attach(extension);
    }

    filepath_t::filepath_t(dirpath_t const& dirpath, pathstr_t* filename, pathstr_t* extension) : m_dirpath(dirpath)
    {
        m_filename  = m_dirpath.m_device->m_root->m_paths->attach(filename);
        m_extension = m_dirpath.m_device->m_root->m_paths->attach(extension);
    }

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
        m_filename  = filesys_t::sNilName;
        m_extension = filesys_t::sNilName;
    }

    bool filepath_t::isRooted() const { return m_dirpath.isRooted(); }
    bool filepath_t::isEmpty() const { return m_dirpath.isEmpty() && m_filename == filesys_t::sNilName && m_extension == filesys_t::sNilName; }

    void filepath_t::makeRelativeTo(const dirpath_t& dirpath) { m_dirpath.makeRelativeTo(dirpath); }

    void filepath_t::makeAbsoluteTo(const dirpath_t& dirpath) { m_dirpath.makeAbsoluteTo(dirpath); }

    void filepath_t::setDirpath(dirpath_t const& dirpath) { m_dirpath = dirpath; }

    void filepath_t::setDevice(crunes_t const& devicename)
    {
        filesys_t* root = m_dirpath.m_device->m_root;

        fullpath_parser_utf32 parser;
        parser.parse(devicename);
        if (parser.has_device())
        {
            m_dirpath.m_device = root->register_device(parser.m_device);
        }
    }

    void filepath_t::setFilename(pathstr_t* filename)
    {
        if (filename != m_filename)
        {
            filesys_t* root = m_dirpath.m_device->m_root;
            root->release_filename(m_filename);
            m_filename = m_dirpath.m_device->m_root->m_paths->attach(filename);
        }
    }

    void filepath_t::setFilename(crunes_t const& filenamestr)
    {
        filesys_t* root          = m_dirpath.m_device->m_root;
        pathstr_t* out_filename  = nullptr;
        pathstr_t* out_extension = nullptr;
        root->register_filename(filenamestr, out_filename, out_extension);
        root->release_filename(m_filename);
        root->release_extension(m_extension);
        m_filename  = m_dirpath.m_device->m_root->m_paths->attach(out_filename);
        m_extension = m_dirpath.m_device->m_root->m_paths->attach(out_extension);
    }

    void filepath_t::setExtension(pathstr_t* extension)
    {
        if (extension != m_extension)
        {
            filesys_t* root = m_dirpath.m_device->m_root;
            root->release_extension(m_extension);
            m_extension = m_dirpath.m_device->m_root->m_paths->attach(extension);
        }
    }

    void filepath_t::setExtension(crunes_t const& extensionstr)
    {
        filesys_t* root = m_dirpath.m_device->m_root;
        pathstr_t* extension;
        root->register_name(extensionstr, extension);
        setExtension(extension);
    }

    dirpath_t filepath_t::root() const { return m_dirpath.root(); }
    dirpath_t filepath_t::dirpath() const { return m_dirpath; }

    filepath_t filepath_t::filename() const { return filepath_t(m_filename, m_extension); }
    filepath_t filepath_t::relative() const
    {
        filesys_t* root = m_dirpath.m_device->m_root;
        filepath_t fp(root->sNilDevice, m_dirpath.m_path, m_filename, m_extension);
        return fp;
    }

    pathnode_t* filepath_t::dirstr() const { return m_dirpath.m_path; }

    pathstr_t* filepath_t::filenamestr() const { return m_filename; }

    pathstr_t* filepath_t::extensionstr() const { return m_extension; }

    void filepath_t::split(s32 pivot, dirpath_t& left, filepath_t& right) const
    {
        // left.clear();
        // right.clear();
        // m_dirpath.split(pivot, left, right.m_dirpath);
        // left.m_device = m_dirpath.m_device->attach();
        // right.m_dirpath.m_device = m_dirpath.m_device->attach();
        // right.m_filename = m_dirpath.m_device->m_root->m_paths->attach(m_filename);
        // right.m_extension = m_dirpath.m_device->m_root->m_paths->attach(m_extension);
    }

    void filepath_t::truncate(filepath_t& left, pathnode_t*& folder) const
    {
        // left.clear();
        // folder = m_dirpath.basename();
        // filesys_t* root = m_dirpath.m_device->m_root;
        // root->get_split_path(m_dirpath.m_path, m_dirpath.m_path->m_len - 1, &left.m_dirpath.m_path, nullptr);
        // left.m_dirpath.m_device = m_dirpath.m_device->attach();
        // left.m_filename = m_dirpath.m_device->m_root->m_paths->attach(m_filename);
        // left.m_extension = m_dirpath.m_device->m_root->m_paths->attach(m_extension);
    }

    void filepath_t::truncate(pathnode_t*& folder, filepath_t& filepath) const
    {
        // filepath.clear();
        // folder = m_dirpath.rootname();
        // filesys_t* root = m_dirpath.m_device->m_root;
        // root->get_split_path(m_dirpath.m_path, m_dirpath.m_path->m_len - 1, &filepath.m_dirpath.m_path, nullptr);
        // filepath.m_dirpath.m_device = m_dirpath.m_device->attach();
        // filepath.m_filename = m_dirpath.m_device->m_root->m_paths->attach(m_filename);
        // filepath.m_extension = m_dirpath.m_device->m_root->m_paths->attach(m_extension);
    }

    void filepath_t::combine(pathnode_t* folder, filepath_t const& filepath)
    {
    }

    void filepath_t::combine(filepath_t const& filepath, pathnode_t* folder)
    {
    }

    void filepath_t::down(crunes_t const& folder)
    {
        m_dirpath.down(folder);
    }

    void filepath_t::up()
    {
    }

    void filepath_t::to_string(runes_t& str) const
    {
        m_dirpath.to_string(str);
        crunes_t filenamestr(m_filename->str(), 0, m_filename->get_len(), m_filename->get_len());
        ncore::concatenate(str, filenamestr);
        crunes_t extensionstr(m_extension->str(), 0, m_extension->get_len(), m_extension->get_len());
        ncore::concatenate(str, extensionstr);
    }

    s32 filepath_t::to_strlen() const
    {
        s32 len = m_dirpath.to_strlen();
        len += m_filename->get_len();
        len += m_extension->get_len();
        return len;
    }

    s32 filepath_t::compare(const filepath_t& right) const
    {
        s32 const fe = m_dirpath.m_device->m_root->m_paths->compare(m_filename, right.m_filename);
        if (fe != 0)
            return fe;
        s32 const ce = m_dirpath.m_device->m_root->m_paths->compare(m_extension, right.m_extension);
        if (ce != 0)
            return ce;
        return m_dirpath.compare(right.m_dirpath);
    }

    filepath_t& filepath_t::operator=(const filepath_t& fp)
    {
        m_dirpath       = fp.m_dirpath;
        filesys_t* root = m_dirpath.m_device->m_root;
        root->release_filename(m_filename);
        root->release_extension(m_extension);
        m_filename  = fp.m_dirpath.m_device->m_root->m_paths->attach(m_filename);
        m_extension = fp.m_dirpath.m_device->m_root->m_paths->attach(m_extension);
        return *this;
    }

    bool operator==(const filepath_t& left, const filepath_t& right) { return left.compare(right) == 0; }

    bool operator!=(const filepath_t& left, const filepath_t& right) { return left.compare(right) != 0; }

    filepath_t operator+(const dirpath_t& dirpath, const filepath_t& filepath) { return filepath_t(dirpath, filepath.filenamestr(), filepath.extensionstr()); }

} // namespace ncore
