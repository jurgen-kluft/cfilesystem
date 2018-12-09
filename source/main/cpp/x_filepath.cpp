#include "xbase/x_target.h"
#include "xbase/x_debug.h"
#include "xbase/x_runes.h"

#include "xfilesystem/private/x_enumerations.h"
#include "xfilesystem/x_dirpath.h"
#include "xfilesystem/x_filepath.h"

namespace xcore
{
    using namespace utf32;

    static void fix_slashes(utf16::runes& str)
    {
        uchar32 slash = '\\';

        // Replace incorrect slashes with the correct one
        if (slash == '\\')
            findReplace(str, (uchar32)'/', (uchar32)'\\');
        else
            replace(str, (uchar32)'\\', (uchar32)'/');

        // Remove double slashes like '\\' or '//'

        // Remove slashes at the start and end of this string
        trimDelimiters(str, slash, slash);
    }

    xfilepath::xfilepath() : mFileSystem(nullptr), mRunes() {}
    xfilepath::xfilepath(xfilesystem* parent, utf16::alloc* allocator, utf16::runes& str)
        : mFileSystem(parent), mAlloc(allocator), mRunes()
    {
        copy(str, mRunes, mAlloc, 16);
        fix_slashes(mRunes);
    }

    xfilepath::xfilepath(const xfilepath& filepath) : mFileSystem(filepath.mFileSystem), mAlloc(filepath.mAlloc), mRunes()
    {
        copy(filepath.mRunes, mRunes, mAlloc, 16);
    }

    xfilepath::xfilepath(const xdirpath& dirpath, const xfilepath& filepath) : mFileSystem(filepath.mFileSystem), mRunes()
    {
        if (filepath.isRooted())
        {
            runes relativefilepath = findSelectAfter(filepath.mRunes, runez<4>(":\\"));
            concatenate(mRunes, dirpath.mRunes, relativefilepath, mAlloc, 16);
            fix_slashes(mRunes);
        }
        else
        {
            concatenate(mRunes, dirpath.mRunes, filepath.mRunes, mAlloc, 16);
            fix_slashes(mRunes);
        }
    }

    xfilepath::~xfilepath() { mAlloc->deallocate(mRunes); }

    void xfilepath::clear() { mRunes.clear(); }

    bool xfilepath::isEmpty() const { return mRunes.is_empty(); }
    bool xfilepath::isRooted() const
    {
        runes pos = find(mRunes, runez<4>(":\\"));
        return !pos.is_empty();
    }

    void xfilepath::makeRelative()
    {
        if (isRooted())
        {
            runes pos = findSelectUntilIncluded(mRunes, runez<4>(":\\"));
            remove(mRunes, pos);
        }
    }

    void xfilepath::makeRelative(const xdirpath& root)
    {
        runes devicepart = findSelectUntil(mRunes, runez<4>(":\\"));
        remove(mRunes, devicepart);
        insert(mRunes, root.mRunes);
    }

    void xfilepath::makeRelative(xdirpath& dir, xfilepath& filename) const {}

    bool xfilepath::getRoot(xdirpath& root) const
    {
        runes rootpart = findSelectUntilIncluded(mRunes, runez<4>(":\\"));
        if (rootpart.is_empty() == false)
        {
            copy(root.mRunes, rootpart, root.mAlloc, 16);
            return true;
        }
        else
        {
            return false;
        }
    }

    bool xfilepath::getDirname(xdirpath& outDirPath) const
    {
        if (isEmpty())
            return false;

        outDirPath.mRunes.clear();
        if (outDirPath.mAlloc == nullptr)
            outDirPath.mAlloc = mAlloc;

        // Select a string until and included the last '\'
        runes dirpart = findLastSelectUntilIncluded(mRunes, '\\');
        if (dirpart.is_empty() == false)
        {
            copy(dirpart, outDirPath.mRunes, outDirPath.mAlloc, 16);
        }
        else
        {
            copy(mRunes, outDirPath.mRunes, outDirPath.mAlloc, 16);
        }

        return true;
    }

    void xfilepath::getFilename(xfilepath& filename) const
    {
        runes filenamepart = findLastSelectAfter(mRunes, '\\');
        filename.mRunes.clear();
        concatenate(filename.mRunes, filenamepart, filename.mAlloc, 16);
    }

    void xfilepath::getFilenameWithoutExtension(xfilepath& filename) const
    {
        runes filenamepart = findLastSelectAfter(mRunes, '\\');
        filenamepart       = findLastSelectUntil(filenamepart, '.');
        filename.mRunes.clear();
        concatenate(filename.mRunes, filenamepart, filename.mAlloc, 16);
    }

    void xfilepath::getExtension(xfilepath& filename) const
    {
        runes filenamepart = findLastSelectAfter(mRunes, '\\');
        filenamepart       = findLastSelectUntil(filenamepart, '.');
        filename.mRunes.clear();
        concatenate(filename.mRunes, filenamepart, filename.mAlloc, 16);
    }

    xfilepath& xfilepath::operator=(const xfilepath& path)
    {
        if (this == &path)
            return *this;

        if (mAlloc != nullptr)
        {
            mAlloc->deallocate(mRunes);
        }
        mAlloc = path.mAlloc;
        copy(path.mRunes, mRunes, mAlloc, 16);
        return *this;
    }

    bool xfilepath::operator==(const xfilepath& rhs) const { return compare(mRunes, rhs.mRunes) == 0; }
    bool xfilepath::operator!=(const xfilepath& rhs) const { return compare(mRunes, rhs.mRunes) != 0; }

} // namespace xcore
