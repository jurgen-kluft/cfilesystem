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

    xfilepath::xfilepath()
        : mFileSystem(nullptr)
        , mPath()
    {
    }
    xfilepath::xfilepath(xfilesystem* parent, xpath& path)
        : mFileSystem(parent)
        , mPath()
    {
        mPath.m_alloc = path.m_alloc;
        mPath.m_path  = path.m_path;
        path.m_alloc  = nullptr;
        path.m_path   = xpath::__runes();
    }

    xfilepath::xfilepath(const xfilepath& filepath)
        : mFileSystem(filepath.mFileSystem)
        , mPath()
    {
        mPath = filepath.mPath;
    }

    xfilepath::xfilepath(const xdirpath& dirpath, const xfilepath& filepath)
        : mFileSystem(filepath.mFileSystem)
        , mPath()
    {
        mPath.combine(dirpath.mPath, filepath.mPath);
    }

    xfilepath::~xfilepath() {}

    void xfilepath::clear() { mPath.clear(); }

    bool xfilepath::isEmpty() const { return mPath.isEmpty(); }
    bool xfilepath::isRooted() const { return mPath.isRooted(); }

    void xfilepath::makeRelative() { mPath.makeRelative(); }

    void xfilepath::makeRelativeTo(const xdirpath& root) { mPath.makeRelativeTo(root.mPath); }

    bool xfilepath::getRoot(xdirpath& root) const { return mPath.getRootDir(root.mPath); }

    bool xfilepath::getDirname(xdirpath& outDirPath) const { return mPath.getDirname(outDirPath.mPath); }

    void xfilepath::getFilename(xfilepath& filename) const { mPath.getFilename(filename.mPath); }

    void xfilepath::getFilenameWithoutExtension(xfilepath& filename) const { mPath.getFilenameWithoutExtension(filename.mPath); }

    void xfilepath::getExtension(xfilepath& filename) const { mPath.getExtension(filename.mPath); }

    xfilepath& xfilepath::operator=(const xfilepath& path)
    {
        if (this == &path)
            return *this;
        mPath = path.mPath;
        return *this;
    }

    bool xfilepath::operator==(const xfilepath& rhs) const { return mPath == rhs.mPath; }
    bool xfilepath::operator!=(const xfilepath& rhs) const { return mPath != rhs.mPath; }

} // namespace xcore
