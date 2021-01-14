#include "xbase/x_target.h"
#include "xbase/x_debug.h"
#include "xbase/x_runes.h"

#include "xfilesystem/private/x_enumerations.h"
#include "xfilesystem/private/x_path.h"
#include "xfilesystem/x_dirpath.h"
#include "xfilesystem/x_filepath.h"

namespace xcore
{
    filepath_t::filepath_t() : m_context(nullptr), m_path() {}
    filepath_t::filepath_t(filesystem_t::context_t* ctxt) : m_context(ctxt), m_path(ctxt)
    {
    }
    filepath_t::filepath_t(filesystem_t::context_t* ctxt, crunes_t const& path) : m_context(ctxt), m_path()
    {
        m_path.m_context = ctxt;
        copy(path, m_path.m_path, ctxt->m_stralloc);
    }

    filepath_t::filepath_t(const filepath_t& filepath) : m_context(filepath.m_context), m_path() { m_path = filepath.m_path; }
    filepath_t::filepath_t(const dirpath_t& dirpath, const filepath_t& filepath) : m_context(filepath.m_context), m_path() { m_path.combine(dirpath.m_path, filepath.m_path); }
    filepath_t::~filepath_t() {}

    void filepath_t::clear() { m_path.clear(); }
    bool filepath_t::isEmpty() const { return m_path.isEmpty(); }
    bool filepath_t::isRooted() const { return m_path.isRooted(); }

    void filepath_t::makeRelative() { m_path.makeRelative(); }
    void filepath_t::makeRelativeTo(const dirpath_t& root) { m_path.makeRelativeTo(root.m_path); }
    void filepath_t::makeAbsoluteTo(const dirpath_t& root) { m_path.setRootDir(root.m_path); }
    bool filepath_t::getRoot(dirpath_t& root) const { return m_path.getRootDir(root.m_path); }
    bool filepath_t::getDirname(dirpath_t& outDirPath) const { return m_path.getDirname(outDirPath.m_path); }
    void filepath_t::getFilename(filepath_t& filename) const { m_path.getFilename(filename.m_path); }
    void filepath_t::getFilenameWithoutExtension(filepath_t& filename) const { m_path.getFilenameWithoutExtension(filename.m_path); }
    void filepath_t::getExtension(filepath_t& filename) const { m_path.getExtension(filename.m_path); }

    void filepath_t::up() { m_path.up(); }
    void filepath_t::down(dirpath_t const& p) { m_path.down(p.m_path); }

    void filepath_t::toString(runes_t& dst) const
    {
        return m_path.toString(dst);
    }

    filepath_t& filepath_t::operator=(const filepath_t& path)
    {
        if (this == &path)
            return *this;
        m_path = path.m_path;
        return *this;
    }

    bool filepath_t::operator==(const filepath_t& rhs) const { return m_path == rhs.m_path; }
    bool filepath_t::operator!=(const filepath_t& rhs) const { return m_path != rhs.m_path; }

} // namespace xcore
