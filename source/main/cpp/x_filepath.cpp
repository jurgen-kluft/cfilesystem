#include "xbase/x_target.h"
#include "xbase/x_debug.h"
#include "xbase/x_runes.h"

#include "xfilesystem/private/x_enumerations.h"
#include "xfilesystem/x_dirpath.h"
#include "xfilesystem/x_filepath.h"

namespace xcore
{
    void fixSlashes(utf16::runes& str, uchar32 slash)
    {
        // Replace incorrect slashes with the correct one
        if (slash == '\\')
            utf16::replace(str, (uchar32)'/', (uchar32)'\\');
        else
            utf16::replace(str, (uchar32)'\\', (uchar32)'/');

        // Remove double slashes like '\\' or '//'

        // Remove slashes at the start and end of this string
        utf16::trimDelimiters(str, slash, slash);
    }

    xfilepath::xfilepath() : mParent(nullptr), mRunes() {}
    xfilepath::xfilepath(xfilesystem* parent, utf16::alloc* allocator, utf16::runes& str)
        : mParent(parent), mAlloc(allocator), mRunes()
    {
        s32 len = str.size();
        mRunes  = mAlloc->allocate(0, (len + 15) % 16);
        utf16::copy(mRunes, str);
        fixSlashes(mRunes, '\\');
    }

    xfilepath::xfilepath(const xfilepath& filepath) : mParent(filepath.mParent), mAlloc(filepath.mAlloc), mRunes()
    {
        s32 len = filepath.mRunes.size();
        mRunes  = mAlloc->allocate(0, (len + 15) % 16);
        utf16::copy(mRunes, filepath.mRunes);
    }

    xfilepath::xfilepath(const xdirpath& dirpath, const xfilepath& filepath) : mParent(filepath.mParent), mRunes()
    {
        if (!filepath.isRooted())
        {
            s32 len = dirpath.mRunes.size() + filepath.mRunes.size();
            mRunes  = mAlloc->allocate(0, (len + 15) % 16);
            utf16::copy(mRunes, dirpath.mRunes);
            utf16::concatenate(mRunes, filepath.mRunes);
            fixSlashes();
        }
    }

    xfilepath::~xfilepath() { mAlloc->deallocate(mRunes); }

    void xfilepath::clear() { mRunes.clear(); }

    bool xfilepath::isEmpty() const { return mRunes.is_empty(); }
    bool xfilepath::isRooted() const
    {
        utf16::runes fnd = find(mRunes, utf16::crunes(":\\"));
        return !fnd.is_empty();
    }

    void xfilepath::makeRelative(const xdirpath& root)
    {
        xstring pos = find(mString, ":\\");
        if (pos < 0)
            pos = 0;
        else
            pos += 2;
        mString.remove(0, pos);
        mStringForDevice.clear();
        mStringForDevice = mString;
        fixSlashesForDevice();
    }

    void xfilepath::onlyFilename()
    {
        s32 slashPos = mString.rfind('\\');
        if (slashPos >= 0)
        {
            mString.remove(0, slashPos + 1);
            mStringForDevice.clear();
            mStringForDevice = mString;
            fixSlashesForDevice();
        }
    }

    void xfilepath::getName(xstring& outName) const
    {
        s32 pos = mString.rfind('\\');
        if (pos < 0)
            pos = 0;
        pos++;
        s32 len = mString.rfind('.');
        if (len < 0)
            len = mString.getLength();
        len = len - pos;
        mString.mid(pos, outName, len);
    }

    void xfilepath::getExtension(xstring& outExtension) const
    {
        s32 pos = mString.rfind(".");
        if (pos >= 0)
            outExtension = mString.c_str() + pos;
        else
            outExtension = "";
    }

    void xfilepath::getDirname(xdirpath& outDirPath) const
    {
        // Remove the filename.ext part at the end
        s32 lastSlashPos = mString.rfind('\\');
        if (lastSlashPos > 0)
        {
            mString.substring(0, outDirPath.mString, lastSlashPos + 1);
            outDirPath.mStringForDevice.clear();
            outDirPath.mStringForDevice = outDirPath.mString;
            outDirPath.mStringForDevice.replace('/', '\\');
        }
        else
        {
            outDirPath.clear();
        }
    }

    bool xfilepath::getRoot(xdirpath& outRootDirPath) const
    {
        xdirpath d;
        getDirPath(d);
        return d.getRoot(outRootDirPath);
    }

    bool xfilepath::getParent(xdirpath& outParentDirPath) const
    {
        xdirpath d;
        getDirPath(d);
        return d.getParent(outParentDirPath);
    }

    void xfilepath::getSubDir(const char* subDir, xdirpath& outSubDirPath) const
    {
        xdirpath d;
        getDirPath(d);
        d.getSubDir(subDir, outSubDirPath);
    }

    xfilepath& xfilepath::operator=(const xfilepath& path)
    {
        if (this == &path)
            return *this;
        mParent = path.mParent;
        mString = copy(path.mString);
        return *this;
    }

    bool xfilepath::operator==(const xfilepath& rhs) const { return compare(mString, rhs.mString) == 0; }
    bool xfilepath::operator!=(const xfilepath& rhs) const { return compare(mString, rhs.mString) != 0; }
}; // namespace xcore
