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
    dirpath_t::dirpath_t(filesystem_t::context_t* ctxt) : m_context(ctxt), m_path(ctxt)
    {
    }

    dirpath_t::dirpath_t(filesystem_t::context_t* ctxt, crunes_t const& path) : m_context(ctxt)
    {
        m_path.m_context= ctxt;
        copy(path, m_path.m_path, ctxt->m_stralloc);
    }

    dirpath_t::dirpath_t() : m_context(nullptr), m_path() {}
    dirpath_t::dirpath_t(const dirpath_t& dirpath) : m_context(dirpath.m_context), m_path(dirpath.m_path) {}
    dirpath_t::dirpath_t(const dirpath_t& rootdir, const dirpath_t& subpath) : m_context(rootdir.m_context), m_path(rootdir.m_path, subpath.m_path) {}

    dirpath_t::~dirpath_t() {}

    void dirpath_t::clear() { m_path.clear(); }
    bool dirpath_t::isEmpty() const { return m_path.isEmpty(); }
    bool dirpath_t::isRoot() const { return m_path.isRoot(); }
    bool dirpath_t::isRooted() const { return m_path.isRooted(); }
    bool dirpath_t::isSubDirOf(const dirpath_t& parent) const { return m_path.isSubDirOf(parent.m_path); }
    void dirpath_t::relative(dirpath_t& outRelative) const
    {
        outRelative = *this;
        outRelative.makeRelative();
    }

    void dirpath_t::makeRelative() { m_path.makeRelative(); }
    void dirpath_t::makeRelativeTo(const dirpath_t& parent) { m_path.makeRelativeTo(parent.m_path); }
    void dirpath_t::makeRelativeTo(const dirpath_t& parent, dirpath_t& sub) const
    {
        sub = *this;
        sub.makeRelativeTo(parent);
    }

    s32  dirpath_t::getLevels() const { return m_path.getLevels(); }
    bool dirpath_t::getLevel(s32 level, dirpath_t& name) const { return m_path.getLevel(level, name.m_path); }
    s32  dirpath_t::getLevelOf(const dirpath_t& name) const { return m_path.getLevelOf(name.m_path); }

    // e.g. dirpath_t d("K:\\parent\\folder\\sub\\folder\\"); d.split(2, parent, sub); parent=="K:\\parent\\folder\\;
    // sub=="sub\\folder\\";
    bool dirpath_t::split(s32 cnt, dirpath_t& parent, dirpath_t& subDir) const { return m_path.split(cnt, parent.m_path, subDir.m_path); }

    bool dirpath_t::getName(dirpath_t& outName) const { return false; }
    bool dirpath_t::hasName(const dirpath_t& inName) const { return false; }
    bool dirpath_t::getRoot(dirpath_t& outRootDirPath) const { return false; }
    bool dirpath_t::getParent(dirpath_t& outParentDirPath) const { return true; }
    void dirpath_t::setRoot(const dirpath_t& inRoot) {}

    void dirpath_t::toString(runes_t& dst) const
    {
        m_path.toString(dst);
    }

    filepath_t dirpath_t::operator+=(const filepath_t& other) { return filepath_t(*this, other); }
    dirpath_t& dirpath_t::operator+=(const dirpath_t& other)
    {
        m_path = path_t(m_path, other.m_path);
        return *this;
    }

    dirpath_t& dirpath_t::operator=(const filepath_t& fp)
    {
        // Copy the runes
        path_t const& path = filesys_t::get_path(fp);
        copy(path.m_path, m_path.m_path, m_path.m_context->m_stralloc, 16);
        return *this;
    }

    dirpath_t& dirpath_t::operator=(const dirpath_t& dp)
    {
        if (this == &dp)
            return *this;

        // Copy the runes
        path_t const& path = filesys_t::get_path(dp);
        copy(path.m_path, m_path.m_path, m_path.m_context->m_stralloc, 16);

        return *this;
    }

    bool dirpath_t::operator==(const dirpath_t& rhs) const { return rhs.m_path == m_path; }
    bool dirpath_t::operator!=(const dirpath_t& rhs) const { return rhs.m_path != m_path; }

    dirpath_t operator+(const dirpath_t& lhs, const dirpath_t& rhs) { return dirpath_t(lhs, rhs); }

}; // namespace xcore
