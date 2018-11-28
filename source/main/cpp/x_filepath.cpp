#include "xbase/x_target.h"
#include "xbase/x_debug.h"
#include "xstring/x_string.h"

#include "xfilesystem/private/x_enumerator.h"
#include "xfilesystem/x_dirpath.h"
#include "xfilesystem/x_filepath.h"

namespace xcore
{
    namespace xfilesystem_utilities
    {
        void fixSlashes(xstring &str, uchar32 slash)
        {
            // Replace incorrect slashes with the correct one
            if (slash == '\\')
                replace(str, '/', '\\');
            else
                replace(str, '\\', '/');

            // Remove double slashes like '\\' or '//'

            // Remove slashes at the start and end of this string
            trimDelimiters(str, rightSlash, rightSlash);
        }
    }    // namespace xfilesystem_utilities

    xfilepath::xfilepath() : mParent(nullptr), mString() {}
    xfilepath::xfilepath(xfilesystem *parent, xstring const &str) : mParent(parent), mString(str)
    {
        fixSlashes(mString);
    }

    xfilepath::xfilepath(const xfilepath &filepath) : mParent(filepath.mParent), mString()
    {
        mString += copy(filepath.mString);
    }

    xfilepath::xfilepath(const xdirpath &dirpath, const xfilepath &filepath) : mParent(filepath.mParent), mString()
    {
        if (!filepath.isRooted())
        {
            mString = copy(dirpath.mString);
            mString = mString + xstring("\\") + filepath.mString;
        }
        fixSlashes();
    }

    xfilepath::~xfilepath() {}

    void xfilepath::clear() { mString.clear(); }

    bool xfilepath::isEmpty() const { return mString.isEmpty(); }
    bool xfilepath::isRooted() const
    {
        xstring fnd = find(mString, ":\\");
        return !fnd.is_empty();
    }

    void xfilepath::makeRelative(const xdirpath &root)
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

    void xfilepath::getName(xstring &outName) const
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

    void xfilepath::getExtension(xstring &outExtension) const
    {
        s32 pos = mString.rfind(".");
        if (pos >= 0)
            outExtension = mString.c_str() + pos;
        else
            outExtension = "";
    }

    void xfilepath::getDirname(xdirpath &outDirPath) const
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

    bool xfilepath::getRoot(xdirpath &outRootDirPath) const
    {
        xdirpath d;
        getDirPath(d);
        return d.getRoot(outRootDirPath);
    }

    bool xfilepath::getParent(xdirpath &outParentDirPath) const
    {
        xdirpath d;
        getDirPath(d);
        return d.getParent(outParentDirPath);
    }

    void xfilepath::getSubDir(const char *subDir, xdirpath &outSubDirPath) const
    {
        xdirpath d;
        getDirPath(d);
        d.getSubDir(subDir, outSubDirPath);
    }

    xfilepath &xfilepath::operator=(const xfilepath &path)
    {
        if (this == &path)
            return *this;
        mParent = path.mParent;
        mString = copy(path.mString);
        return *this;
    }

    bool xfilepath::operator==(const xfilepath &rhs) const { return compare(mString, rhs.mString) == 0; }
    bool xfilepath::operator!=(const xfilepath &rhs) const { return compare(mString, rhs.mString) != 0; }
};
