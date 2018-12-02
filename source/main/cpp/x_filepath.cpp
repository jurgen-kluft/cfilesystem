#include "xbase/x_target.h"
#include "xbase/x_debug.h"
#include "xbase/x_runes.h"

#include "xfilesystem/private/x_enumerations.h"
#include "xfilesystem/x_dirpath.h"
#include "xfilesystem/x_filepath.h"

namespace xcore
{
    using namespace utf16;

    static void fix_slashes(utf16::runes& str)
    {
        uchar32 slash = '\\';

        // Replace incorrect slashes with the correct one
        if (slash == '\\')
            replace(str, (uchar32)'/', (uchar32)'\\');
        else
            replace(str, (uchar32)'\\', (uchar32)'/');

        // Remove double slashes like '\\' or '//'

        // Remove slashes at the start and end of this string
        trimDelimiters(str, slash, slash);
    }

    xfilepath::xfilepath() : mParent(nullptr), mRunes() {}
    xfilepath::xfilepath(xfilesystem* parent, utf16::alloc* allocator, utf16::runes& str)
        : mParent(parent), mAlloc(allocator), mRunes()
    {
        copy(str, mRunes, mAlloc, 16);
        fix_slashes(mRunes);
    }

    xfilepath::xfilepath(const xfilepath& filepath) : mParent(filepath.mParent), mAlloc(filepath.mAlloc), mRunes()
    {
        copy(filepath, mRunes, mAlloc, 16);
    }

    xfilepath::xfilepath(const xdirpath& dirpath, const xfilepath& filepath) : mParent(filepath.mParent), mRunes()
    {
        if (!filepath.isRooted())
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

    void xfilepath::makeRelative(const xdirpath& root)
    {
        runes devicepart = selectUntil(mRunes, runez<4>(":\\"));
        remove(mRunes, devicepart);
        insert(mRunes, root);
    }

    void xfilepath::getFilename(xfilepath& filename) const
    {
        runes filenamepart = rselectUntil(mRunes, '\\');
        filename.mRunes.clear();
        concatenate(filename.mRunes, filenamepart, filename.mAlloc, 16);
    }

    void xfilepath::getFilenameWithoutExtension(xfilepath& filename) const
    {
        runes filenamepart = rselectUntil(mRunes, '\\');
        filenamepart       = selectUntilLast(filenamepart, '.');
        filename.mRunes.clear();
        concatenate(filename.mRunes, filenamepart, filename.mAlloc, 16);
    }

    void xfilepath::getDirname(xdirpath& outDirPath) const
    {
        outDirPath.mRunes.clear();

        // Select a string until and included the last '\'
        runes dirpart = selectUntilIncludedLast(mRunes, '\\');
        if (dirpart.is_empty() == false)
        {
            copy(outDirPath.mRunes, dirpart, outDirPath.mAlloc, 16);
        }
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
        copy(mRunes, path.mRunes, mAlloc, 16);
        return *this;
    }

    bool xfilepath::operator==(const xfilepath& rhs) const { return compare(mRunes, rhs.mRunes) == 0; }
    bool xfilepath::operator!=(const xfilepath& rhs) const { return compare(mRunes, rhs.mRunes) != 0; }

} // namespace xcore
