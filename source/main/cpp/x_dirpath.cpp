#include "xbase/x_target.h"
#include "xbase/x_debug.h"
#include "xbase/x_runes.h"

#include "xfilesystem/x_filepath.h"
#include "xfilesystem/x_dirpath.h"

//==============================================================================
namespace xcore
{
	using namespace utf16;

	//==============================================================================
	// xdirpath: Device:\\Folder\Folder\ 
	//==============================================================================

	static void fix_slashes(utf16::runes& str)
	{
		// Replace incorrect slashes with the correct one

		// Remove double slashes like '\\' or '//'

		// Make sure there is NO slash at the start of the path

		// Make sure there is a slash at the end of the path
	}

	xdirpath::xdirpath(xfilesystem* fs, alloc* allocator) : mParent(fs), mAlloc(allocator) {}
	xdirpath::xdirpath(xfilesystem* fs, alloc* allocator, runes const& str) : mParent(fs), mAlloc(allocator)
	{
		utf16::copy(str, mRunes, mAlloc, 16);
		fix_slashes(mRunes);
	}
	xdirpath::xdirpath() : mParent(nullptr), mAlloc(nullptr), mRunes() {}
	xdirpath::xdirpath(const xdirpath& dirpath) : mParent(dirpath.mParent), mAlloc(dirpath.mAlloc) { copy(dirpath.mRunes, mRunes, mAlloc, 16); }
	xdirpath::xdirpath(const xdirpath& rootdir, const xdirpath& subdir) : mParent(rootdir.mParent), mAlloc(rootdir.mAlloc)
	{
		xdirpath relativesubdir(subdir);
		relativesubdir.makeRelative();

	}
	xdirpath::~xdirpath()
	{
		if (mAlloc != nullptr)
			mAlloc->deallocate(mRunes);
	}

	void xdirpath::clear() { mRunes.clear(); }

	bool xdirpath::isEmpty() const { return mRunes.is_empty(); }

	class enumerate_runes
	{
	public:
		virtual bool operator()(s32 level, const runes& folder) = 0;
	};

	static void enumerate_fn(const runes& dirpath, enumerate_runes& enumerator, bool right_to_left)
	{
		runez<4> device(":\\");

		s32   level = 0;
		runes path  = findSelectAfter(dirpath, device);
		if (right_to_left == false)
		{
			runes dir = selectBetween(path, '\\', '\\');
			while (dir.is_empty() == false)
			{
				if (enumerator(level, dir) == false)
				{
					break;
				}
				level++;
				dir = selectNextBetween(path, dir, '\\', '\\');
			}
		}
		else
		{
			runes dir = selectBetweenLast(path, '\\', '\\');
			while (dir.is_empty() == false)
			{
				if (enumerator(level, dir) == false)
				{
					break;
				}
				level++;
				dir = selectPreviousBetween(path, dir, '\\', '\\');
			}
		}
	}

	class folder_counting_enumerator : public enumerate_runes
	{
	public:
		s32 mLevels;
		folder_counting_enumerator() : mLevels(0) {}
		virtual bool operator()(s32 level, const runes& folder)
		{
			mLevels++;
			return true;
		}
	};

	s32 xdirpath::getLevels() const
	{
		folder_counting_enumerator e;
		enumerate_fn(mRunes, e, false);
		return e.mLevels;
	}

	bool xdirpath::getLevel(s32 level, xdirpath& outpath) const
	{
		// Return the path at level @level
		runez<4> device(":\\");

		s32   level = 0;
		runes path  = findSelectAfter(mRunes, device);
		if (path.is_empty())
			return false;
		runes dir = selectBetween(mRunes, '\\', '\\');
		while (level > 0 && !dir.is_empty())
		{
			dir = selectNextBetween(mRunes, dir, '\\', '\\');
			level--;
		}
		if (!dir.is_empty())
		{
			runes path = selectUntilEndIncluded(mRunes, dir);
			copy(path, outpath.mRunes, outpath.mAlloc, 16);
			return true;
		}
		return false;
	}

	class folder_search_enumerator : public enumerate_delegate
	{
	public:
		runes mFolderName;
		bool  mFound;
		s32   mLevel;

		folder_search_enumerator(const runes& folder) : mFolderName(folder), mFound(false), mLevel(-1) {}

		virtual bool operator()(s32 level, const runes& folder)
		{
			mFound = (mFolderName == folder);
			mLevel = level;
			return mFound;
		}
	};

	s32 xdirpath::getLevelOf(const xdirpath& parent) const
	{
		// PARENT:   c:\disk
		// THIS:     c:\disk\child
		// RETURN 0
		if (starts_with(mRunes, parent.mRunes))
		{
		}
	}

	bool xdirpath::isRoot() const
	{
		runez<4> device(":\\");
		runes	path = findSelectAfter(mRunes, device);
		if (path.is_empty())
			return false;

		// Now determine if we are an actual root, which means that we
		// do not have any folders after our device statement.
		runes path = findSelectAfter(mRunes, device);
		return path.size() == 0;
	}

	bool xdirpath::isRooted() const
	{
		runez<4> device(":\\");
		runes	path = findSelectAfter(mRunes, device);
		return (!path.is_empty());
	}

	bool xdirpath::isSubDirOf(const xdirpath& parent) const
	{
		// example:
		// parent = E:\data\files\music\ 
		// this = files\music\rnb\ 
		// overlap = files\music\ 
		// remainder = rnb\ 
		if (isRooted())
		{
			return false;
		}

		if (parent.isRooted())
		{
			runes remainder;
			runes overlap = selectOverlap(parent.mRunes, mRunes, remainder);
			return (!overlap.is_empty()) && !remainder.is_empty();
		}
		return false;
	}

	void xdirpath::relative(xdirpath& outRelative) const
	{
		outRelative = *this;
		outRelative.makeRelative();
	}

	void xdirpath::makeRelative()
	{
		runez<4> device(":\\");
		runes	devicepart = findSelectUntilIncluded(mRunes, device);
		if (!devicepart.is_empty())
		{
			remove(mRunes, devicepart);
		}
	}

	void xdirpath::makeRelativeTo(const xdirpath& parent)
	{
		makeRelative();

		// PARENT:   a\b\c\d\ 
        // THIS:     c\d\e\f\ 
		// RESULT:   e\f\ 
        runez<4> device(":\\");
		runes parentpath = parent.mRunes;
		if (parent.isRooted())
		{
			parentpath = findSelectAfter(parentpath, device);
		}

		runes remainder;
		runes overlap = selectOverlap(parentpath, mRunes, remainder);
		keep(mRunes, remainder);
	}

	void xdirpath::makeRelativeTo(const xdirpath& parent, xdirpath& sub) const
	{
		sub = *this;
		sub.makeRelativeTo(parent);
	}

	// e.g. xdirpath d("K:\\parent\\folder\\sub\\folder\\"); d.split(2, parent, sub); parent=="K:\\parent\\folder\\;
	// sub=="sub\\folder\\";
	bool xdirpath::split(s32 cnt, xdirpath& parent, xdirpath& subDir) const
	{
		// Return the path at level @level
		runez<4> device(":\\");

		s32   level = 0;
		runes path  = findSelectAfter(mRunes, device);
		if (path.is_empty())
			return false;
		runes dir = selectBetween(mRunes, '\\', '\\');
		while (level > 0 && !dir.is_empty())
		{
			dir = selectNextBetween(mRunes, dir, '\\', '\\');
			level--;
		}
		if (!dir.is_empty())
		{
			runes path = selectUntilEndIncluded(mRunes, dir);
			copy(path, parent.mRunes, parent.mAlloc, 16);
			return true;
		}

		return false;
	}

	bool xdirpath::getName(xfilepath& outName) const { return false; }

	bool xdirpath::hasName(const xfilepath& inName) const { return false; }

	bool xdirpath::getRoot(xdirpath& outRootDirPath) const { return false; }

	bool xdirpath::getParent(xdirpath& outParentDirPath) const { return true; }

	bool xdirpath::getSubDir(const xfilepath& subDir, xdirpath& outSubDirPath) const { return true; }

	void xdirpath::setRoot(const xdirpath& inRoot) {}

	bool xdirpath::getRoot(xdirpath& outRoot) const { return false; }

	xfilepath xdirpath::operator+=(const xfilepath& fp) { return xfilepath(*this, fp); }

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

	bool xdirpath::operator==(const xdirpath& rhs) const { return compare(rhs.mRunes, mRunes) == 0; }
	bool xdirpath::operator!=(const xdirpath& rhs) const { return compare(rhs.mRunes, mRunes) != 0; }

	xfilepath operator+(const xdirpath& dp, const xfilepath& fp) { return xfilepath(dp, fp); }

}; // namespace xcore
