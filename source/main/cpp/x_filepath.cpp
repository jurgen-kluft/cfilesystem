#include "xbase/x_target.h"
#include "xbase/x_debug.h"
#include "xbase/x_runes.h"

#include "xfilesystem/private/x_enumerations.h"
#include "xfilesystem/private/x_path.h"
#include "xfilesystem/x_dirpath.h"
#include "xfilesystem/x_filepath.h"

namespace xcore
{
    using namespace utf32;

    filepath_t::filepath_t() : mParent(nullptr), mPath() {}
    filepath_t::filepath_t(filesys_t* fsys, path_t& path) : mParent(fsys), mPath()
    {
        mPath.m_alloc = path.m_alloc;
        mPath.m_path  = path.m_path;
        path.m_alloc  = fsys->m_stralloc;
    }

    filepath_t::filepath_t(const filepath_t& filepath) : mParent(filepath.mParent), mPath() { mPath = filepath.mPath; }
    filepath_t::filepath_t(const dirpath_t& dirpath, const filepath_t& filepath) : mParent(filepath.mParent), mPath() { mPath.combine(dirpath.mPath, filepath.mPath); }
    filepath_t::~filepath_t() {}

    void filepath_t::clear() { mPath.clear(); }
    bool filepath_t::isEmpty() const { return mPath.isEmpty(); }
    bool filepath_t::isRooted() const { return mPath.isRooted(); }

    void filepath_t::makeRelative() { mPath.makeRelative(); }
    void filepath_t::makeRelativeTo(const dirpath_t& root) { mPath.makeRelativeTo(root.mPath); }
    void filepath_t::makeAbsoluteTo(const dirpath_t& root) { mPath.setRootDir(root.mPath); }
    bool filepath_t::getRoot(dirpath_t& root) const { return mPath.getRootDir(root.mPath); }
    bool filepath_t::getDirname(dirpath_t& outDirPath) const { return mPath.getDirname(outDirPath.mPath); }
    void filepath_t::getFilename(filepath_t& filename) const { mPath.getFilename(filename.mPath); }
    void filepath_t::getFilenameWithoutExtension(filepath_t& filename) const { mPath.getFilenameWithoutExtension(filename.mPath); }
    void filepath_t::getExtension(filepath_t& filename) const { mPath.getExtension(filename.mPath); }

    void filepath_t::up() { mPath.up(); }
    void filepath_t::down(dirpath_t const& p) { mPath.down(p.mPath); }

    filepath_t& filepath_t::operator=(const filepath_t& path)
    {
        if (this == &path)
            return *this;
        mPath = path.mPath;
        return *this;
    }

    bool filepath_t::operator==(const filepath_t& rhs) const { return mPath == rhs.mPath; }
    bool filepath_t::operator!=(const filepath_t& rhs) const { return mPath != rhs.mPath; }

} // namespace xcore
