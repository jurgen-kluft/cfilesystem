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

    dirpath_t::dirpath_t(filesys_t* fs, path_t& path) : mParent(fs)
    {
        mPath.m_alloc = path.m_alloc;
        mPath.m_path  = path.m_path;
        path.m_alloc  = nullptr;
        path.m_path   = runes_t();
    }

    dirpath_t::dirpath_t() : mParent(nullptr), mPath() {}
    dirpath_t::dirpath_t(const dirpath_t& dirpath) : mParent(dirpath.mParent), mPath(dirpath.mPath) {}
    dirpath_t::dirpath_t(const dirpath_t& rootdir, const dirpath_t& subpath) : mParent(rootdir.mParent), mPath(rootdir.mPath, subpath.mPath) {}

    dirpath_t::~dirpath_t() {}

    void dirpath_t::clear() { mPath.clear(); }
    bool dirpath_t::isEmpty() const { return mPath.isEmpty(); }
    bool dirpath_t::isRoot() const { return mPath.isRoot(); }
    bool dirpath_t::isRooted() const { return mPath.isRooted(); }
    bool dirpath_t::isSubDirOf(const dirpath_t& parent) const { return mPath.isSubDirOf(parent.mPath); }
    void dirpath_t::relative(dirpath_t& outRelative) const
    {
        outRelative = *this;
        outRelative.makeRelative();
    }

    void dirpath_t::makeRelative() { mPath.makeRelative(); }
    void dirpath_t::makeRelativeTo(const dirpath_t& parent) { mPath.makeRelativeTo(parent.mPath); }
    void dirpath_t::makeRelativeTo(const dirpath_t& parent, dirpath_t& sub) const
    {
        sub = *this;
        sub.makeRelativeTo(parent);
    }

    s32  dirpath_t::getLevels() const { return mPath.getLevels(); }
    bool dirpath_t::getLevel(s32 level, dirpath_t& name) const { return mPath.getLevel(level, name.mPath); }
    s32  dirpath_t::getLevelOf(const dirpath_t& name) const { return mPath.getLevelOf(name.mPath); }

    // e.g. dirpath_t d("K:\\parent\\folder\\sub\\folder\\"); d.split(2, parent, sub); parent=="K:\\parent\\folder\\;
    // sub=="sub\\folder\\";
    bool dirpath_t::split(s32 cnt, dirpath_t& parent, dirpath_t& subDir) const { return mPath.split(cnt, parent.mPath, subDir.mPath); }

    bool dirpath_t::getName(dirpath_t& outName) const { return false; }
    bool dirpath_t::hasName(const dirpath_t& inName) const { return false; }
    bool dirpath_t::getRoot(dirpath_t& outRootDirPath) const { return false; }
    bool dirpath_t::getParent(dirpath_t& outParentDirPath) const { return true; }
    void dirpath_t::setRoot(const dirpath_t& inRoot) {}

    filepath_t dirpath_t::operator+=(const filepath_t& other) { return filepath_t(*this, other); }
    dirpath_t& dirpath_t::operator+=(const dirpath_t& other)
    {
        mPath = path_t(mPath, other.mPath);
        return *this;
    }

    dirpath_t& dirpath_t::operator=(const filepath_t& fp)
    {
        // Copy the runes
        path_t const& path = filesys_t::get_path(fp);
        copy(path.m_path, mPath.m_path, mPath.m_alloc, 16);
        return *this;
    }

    dirpath_t& dirpath_t::operator=(const dirpath_t& dp)
    {
        if (this == &dp)
            return *this;

        // Copy the runes
        path_t const& path = filesys_t::get_path(dp);
        copy(path.m_path, mPath.m_path, mPath.m_alloc, 16);

        return *this;
    }

    bool dirpath_t::operator==(const dirpath_t& rhs) const { return rhs.mPath == mPath; }
    bool dirpath_t::operator!=(const dirpath_t& rhs) const { return rhs.mPath != mPath; }

    dirpath_t operator+(const dirpath_t& lhs, const dirpath_t& rhs) { return dirpath_t(lhs, rhs); }

}; // namespace xcore
