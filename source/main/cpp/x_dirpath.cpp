#include "xbase/x_target.h"
#include "xbase/x_debug.h"
#include "xbase/x_runes.h"

#include "xfilesystem/x_filepath.h"
#include "xfilesystem/x_dirpath.h"

//==============================================================================
namespace xcore
{
    //==============================================================================
    // xdirpath: Device:\\Folder\Folder\ 
	//==============================================================================

    xdirpath::xdirpath(xfilesystem* fs, xpath& path) : mParent(fs)
	{
		mPath.m_alloc = path.m_alloc;
		mPath.m_path = path.m_path;
		path.m_alloc = nullptr;
		path.m_path = xpath::__runes();
	}

    xdirpath::xdirpath() : mParent(nullptr), mPath() {}
    xdirpath::xdirpath(const xdirpath& dirpath) : mParent(dirpath.mParent), mPath(dirpath.mPath)
    {
    }
    xdirpath::xdirpath(const xdirpath& rootdir, const xdirpath& subpath) : mParent(rootdir.mParent), mPath(rootdir.mPath, subpath.mPath)
    {
    }
    xdirpath::~xdirpath()
    {
    }

    void xdirpath::clear() { mPath.clear(); }

    bool xdirpath::isEmpty() const { return mPath.isEmpty(); }

    bool xdirpath::isRoot() const
    {
        return mPath.isRoot();
    }

    bool xdirpath::isRooted() const
    {
        return mPath.isRooted();
    }

    bool xdirpath::isSubDirOf(const xdirpath& parent) const
    {
		return mPath.isSubDirOf(parent.mPath);
    }

    void xdirpath::relative(xdirpath& outRelative) const
    {
        outRelative = *this;
        outRelative.makeRelative();
    }

    void xdirpath::makeRelative()
    {
		mPath.makeRelative();
    }

    void xdirpath::makeRelativeTo(const xdirpath& parent)
    {
        mPath.makeRelativeTo(parent.mPath);
    }

    void xdirpath::makeRelativeTo(const xdirpath& parent, xdirpath& sub) const
    {
        sub = *this;
        sub.makeRelativeTo(parent);
    }

	s32  xdirpath::getLevels() const
	{
		return mPath.getLevels();
	}
	
	bool xdirpath::getLevel(s32 level, xdirpath& name) const
	{
		return mPath.getLevel(level, name.mPath);
	}

	s32  xdirpath::getLevelOf(const xdirpath& name) const
	{
		return mPath.getLevelOf(name.mPath);
	}

    // e.g. xdirpath d("K:\\parent\\folder\\sub\\folder\\"); d.split(2, parent, sub); parent=="K:\\parent\\folder\\;
    // sub=="sub\\folder\\";
    bool xdirpath::split(s32 cnt, xdirpath& parent, xdirpath& subDir) const
    {
        return mPath.split(cnt, parent.mPath, subDir.mPath);
    }

    bool xdirpath::getName(xfilepath& outName) const { return false; }

    bool xdirpath::hasName(const xfilepath& inName) const { return false; }

    bool xdirpath::getRoot(xdirpath& outRootDirPath) const { return false; }

    bool xdirpath::getParent(xdirpath& outParentDirPath) const { return true; }

    void xdirpath::setRoot(const xdirpath& inRoot) {}

    bool xdirpath::getRoot(xdirpath& outRoot) const { return false; }

    xfilepath xdirpath::operator+=(const xfilepath& other) { return xfilepath(*this, other); }
    xdirpath& xdirpath::operator+=(const xdirpath& other) 
	{
		mPath = xpath(mPath, other.mPath); 
	}

    xdirpath& xdirpath::operator=(const xfilepath& fp)
    {
        fp.getDirname(*this);
        return *this;
    }

    xdirpath& xdirpath::operator=(const xdirpath& fp)
    {
        if (this == &fp)
            return *this;

        return *this;
    }

    bool xdirpath::operator==(const xdirpath& rhs) const { return rhs.mPath == mPath; }
    bool xdirpath::operator!=(const xdirpath& rhs) const { return rhs.mPath != mPath; }

    xdirpath operator+(const xdirpath& lhs, const xdirpath& rhs) { return xdirpath(lhs, rhs); }
    xfilepath operator+(const xdirpath& dp, const xfilepath& fp) { return xfilepath(dp, fp); }

}; // namespace xcore
