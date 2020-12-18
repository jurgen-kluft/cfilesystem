#include "xbase/x_target.h"
#include "xbase/x_debug.h"
#include "xbase/x_runes.h"

#include "xfilesystem/private/x_path.h"
#include "xfilesystem/x_filepath.h"
#include "xfilesystem/x_dirpath.h"
#include "xfilesystem/x_enumerator.h"
#include "xfilesystem/private/x_devicemanager.h"

namespace xcore
{
    static utf32::rune sSlash                 = '\\';
    static utf32::rune sSemiColumnSlashStr[3] = {':', '\\', 0};
    static crunes_t    sSemiColumnSlash(sSemiColumnSlashStr, sSemiColumnSlashStr + 2);

    static void fix_slashes(runes_t& str)
    {
        // Replace incorrect slashes with the correct one
        findReplace(str, (uchar32)'/', sSlash);

        // Remove double slashes like '\\' or '//'

        // Remove slashes at the start and end of this string
        trimDelimiters(str, sSlash, sSlash);
    }

    path_t::path_t() : m_alloc(nullptr), m_path() {}

    path_t::path_t(runes_alloc_t* allocator) : m_alloc(allocator), m_path() { fix_slashes(m_path); }

    path_t::path_t(runes_alloc_t* allocator, const crunes_t& path) : m_alloc(allocator)
    {
        copy(path, m_path, m_alloc, 16);
        fix_slashes(m_path);
    }

    path_t::path_t(const path_t& path) : m_alloc(path.m_alloc), m_path()
    {
        copy(path.m_path, m_path, m_alloc, 16);
        fix_slashes(m_path);
    }

    path_t::path_t(const path_t& lhspath, const path_t& rhspath) : m_alloc(lhspath.m_alloc), m_path()
    {
        // Combine both paths into a new path
        concatenate(m_path, lhspath.m_path, rhspath.m_path, m_alloc, 16);
    }

    path_t::~path_t()
    {
        if (m_alloc != nullptr)
        {
            m_alloc->deallocate(m_path);
        }
    }

    path_t path_t::resolve(filesys_t* fs, filedevice_t*& outdevice) const
    {
        runes_t rootpart = find(m_path, sSemiColumnSlash);
        if (rootpart.is_empty())
        {
            outdevice = nullptr;
        }
        else
        {
            path_t path;
            outdevice = fs->m_devman->find_device(*this, path);
            if (outdevice != nullptr)
            {
                return path;
            }
        }
        return path_t();
    }

    void path_t::clear() { m_path.clear(); }

    void path_t::erase()
    {
        clear();
        if (m_alloc != nullptr)
            m_alloc->deallocate(m_path);
    }

    bool path_t::isEmpty() const { return m_path.is_empty(); }
    bool path_t::isRoot() const
    {
        runes_t rootpart = find(m_path, sSemiColumnSlash);
        if (rootpart.is_empty())
            return false;
        // Now determine if we are an actual root, which means that we
        // do not have any folders after our device statement.
        runes_t pathpart = selectAfterExclude(m_path, rootpart);
        return pathpart.is_empty();
    }

    bool path_t::isRooted() const
    {
        runes_t pos = find(m_path, sSemiColumnSlash);
        return !pos.is_empty();
    }

    bool path_t::isSubDirOf(const path_t& parent) const
    {
        // example:
        // parent = "E:\data\files\music\"
        // this = "files\music\rnb\"
        // overlap = "files\music\"
        // remainder = "rnb\"
        if (isRooted())
        {
            return false;
        }

        if (parent.isRooted())
        {
            runes_t overlap = selectOverlap(parent.m_path, m_path);
            return !overlap.is_empty();
        }
        return false;
    }

    void path_t::set_filepath(runes_t& runes_t, runes_alloc_t* allocator)
    {
        erase();
        m_alloc = allocator;
        m_path  = runes_t;

        // Replace incorrect slashes with the correct one
        findReplace(m_path, (uchar32)'/', sSlash);

        // Remove double slashes like '\\' or '//'

        // Remove slash at the beginning and end
        trim(m_path, sSlash);
    }

    void path_t::set_dirpath(runes_t& runes_t, runes_alloc_t* allocator)
    {
        erase();
        m_alloc = allocator;
        m_path  = runes_t;

        // Replace incorrect slashes with the correct one
        findReplace(m_path, (uchar32)'/', sSlash);

        // Remove double slashes like '\\' or '//'

        // Remove slash at the beginning
        trimLeft(m_path, sSlash);

        // Ensure slash at the end
        if (!ends_with(m_path, sSlash))
        {
            crunes_t slashstr(sSemiColumnSlashStr + 1, sSemiColumnSlashStr + 2);
            concatenate(m_path, slashstr, m_alloc, 16);
        }
    }

    void path_t::combine(const path_t& dirpath, const path_t& otherpath)
    {
        erase();
        m_alloc = dirpath.m_alloc;
        if (otherpath.isRooted())
        {
            runes_t relativefilepath = findSelectAfter(otherpath.m_path, sSemiColumnSlash);
            concatenate(m_path, dirpath.m_path, relativefilepath, m_alloc, 16);
        }
        else
        {
            concatenate(m_path, dirpath.m_path, otherpath.m_path, m_alloc, 16);
        }
    }

    void path_t::copy_dirpath(runes_t& runes_t) { copy(runes_t, m_path, m_alloc, 16); }

    class enumerate_runes
    {
    public:
        virtual bool operator()(s32 level, const runes_t& folder) = 0;
    };

    static void enumerate_fn(const runes_t& dirpath, enumerate_runes& enumerator, bool right_to_left)
    {
        s32     level = 0;
        runes_t path  = findSelectAfter(dirpath, sSemiColumnSlash);
        if (right_to_left == false)
        {
            runes_t dir = selectBetween(path, sSlash, sSlash);
            while (dir.is_empty() == false)
            {
                if (enumerator(level, dir) == false)
                {
                    break;
                }
                level++;
                dir = selectNextBetween(path, dir, sSlash, sSlash);
            }
        }
        else
        {
            runes_t dir = selectBetweenLast(path, sSlash, sSlash);
            while (dir.is_empty() == false)
            {
                if (enumerator(level, dir) == false)
                {
                    break;
                }
                level++;
                dir = selectPreviousBetween(path, dir, sSlash, sSlash);
            }
        }
    }

    class folder_counting_enumerator : public enumerate_runes
    {
    public:
        s32 mLevels;
        folder_counting_enumerator() : mLevels(0) {}
        virtual bool operator()(s32 level, const runes_t& folder)
        {
            mLevels++;
            return true;
        }
    };

    s32 path_t::getLevels() const
    {
        folder_counting_enumerator e;
        enumerate_fn(m_path, e, false);
        return e.mLevels;
    }

    bool path_t::getLevel(s32 level, path_t& outpath) const
    {
        // Return the path at level @level
        runez_t<ascii::rune, 4> device(":\\");

        runes_t path = findSelectAfter(m_path, device);
        if (path.is_empty())
            return false;
        runes_t dir = selectBetween(m_path, sSlash, sSlash);
        while (level > 0 && !dir.is_empty())
        {
            dir = selectNextBetween(m_path, dir, sSlash, sSlash);
            level--;
        }
        if (!dir.is_empty())
        {
            runes_t path = selectAfterInclude(m_path, dir);
            copy(path, outpath.m_path, outpath.m_alloc, 16);
            return true;
        }
        return false;
    }

    class folder_search_enumerator : public enumerate_delegate_t
    {
    public:
        runes_t mFolderName;
        bool    mFound;
        s32     mLevel;

        folder_search_enumerator(const runes_t& folder) : mFolderName(folder), mFound(false), mLevel(-1) {}

        virtual bool operator()(s32 level, const runes_t& folder)
        {
            mFound = compare(mFolderName, folder) == 0;
            mLevel = level;
            return mFound;
        }
    };

    s32 path_t::getLevelOf(const path_t& parent) const
    {
        // PARENT:   c:\disk
        // THIS:     c:\disk\child
        // RETURN 0
        if (starts_with(m_path, parent.m_path)) {}

        //@TODO: Implement this!

        return 0;
    }

    bool path_t::split(s32 level, path_t& parent_dirpath, path_t& relative_filepath) const
    {
        // Split the path at level @level
        runes_t path = findSelectAfter(m_path, sSemiColumnSlash);
        if (path.is_empty())
            return false;

        runes_t dir = selectBetween(m_path, sSlash, sSlash);
        while (level > 0 && !dir.is_empty())
        {
            dir = selectNextBetween(m_path, dir, sSlash, sSlash);
            level--;
        }

        if (!dir.is_empty())
        {
            runes_t parent_path = selectBeforeExclude(m_path, dir);
            copy(parent_path, parent_dirpath.m_path, parent_dirpath.m_alloc, 16);
            runes_t relative_path = selectAfterInclude(m_path, dir);
            copy(relative_path, relative_filepath.m_path, relative_filepath.m_alloc, 16);
            return true;
        }

        return false;
    }

    void path_t::makeRelative()
    {
        if (isRooted())
        {
            runes_t pos = findSelectUntilIncluded(m_path, sSemiColumnSlash);
            removeSelection(m_path, pos);
        }
    }

    void path_t::makeRelativeTo(const path_t& parent)
    {
        makeRelative();

        // PARENT:   "a\b\c\d\"
        // THIS:     "c\d\e\f\"
        // RESULT:   "e\f\"

        runes_t parentpath = parent.m_path;
        if (parent.isRooted())
        {
            parentpath = findSelectAfter(parentpath, sSemiColumnSlash);
        }

        runes_t overlap = selectOverlap(parentpath, m_path);
        if (overlap.is_empty() == false)
        {
            runes_t remainder = selectAfterExclude(parentpath, overlap);
            keepOnlySelection(m_path, remainder);
        }
    }

    void path_t::setRootDir(const path_t& in_root_dirpath)
    {
        if (!isRooted())
        {
            runes_t rootpart = findSelectUntilIncluded(m_path, sSemiColumnSlash);
            if (rootpart.is_empty() == false)
            {
                insert(m_path, in_root_dirpath.m_path, m_alloc, 16);
            }
            else
            {
                replaceSelection(m_path, rootpart, in_root_dirpath.m_path, m_alloc, 16);
            }
        }
    }

    bool path_t::getRootDir(path_t& root) const
    {
        if (isRooted())
        {
            runes_t rootpart = findSelectUntilIncluded(m_path, sSemiColumnSlash);
            if (rootpart.is_empty() == false)
            {
                root.erase();
                copy(root.m_path, rootpart, root.m_alloc, 16);
                return true;
            }
        }
        return false;
    }

    bool path_t::getDirname(path_t& outDirPath) const
    {
        if (isEmpty())
            return false;

        outDirPath.clear();
        outDirPath.m_alloc = m_alloc;

        // Select a string until and included the last '\'
        runes_t dirpart = findLastSelectUntilIncluded(m_path, sSlash);
        if (dirpart.is_empty() == false)
        {
            copy(dirpart, outDirPath.m_path, outDirPath.m_alloc, 16);
            return true;
        }
        return false;
    }

    bool path_t::up() { return false; }

    bool path_t::down(path_t const& runes_t) { return false; }

    void path_t::getFilename(path_t& filename) const
    {
        runes_t filenamepart = findLastSelectAfter(m_path, sSlash);
        if (!filenamepart.is_empty())
        {
            filename.clear();
            concatenate(filename.m_path, filenamepart, filename.m_alloc, 16);
        }
    }

    void path_t::getFilenameWithoutExtension(path_t& filename) const
    {
        filename.clear();
        runes_t filenamepart = findLastSelectAfter(m_path, sSlash);
        if (!filenamepart.is_empty())
        {
            filenamepart = findLastSelectUntil(filenamepart, '.');
            concatenate(filename.m_path, filenamepart, filename.m_alloc, 16);
        }
    }

    void path_t::getExtension(path_t& filename) const
    {
        filename.clear();
        runes_t filenamepart = findLastSelectAfter(m_path, sSlash);
        runes_t filenameonly = findLastSelectUntil(filenamepart, '.');
        runes_t fileext      = selectAfterExclude(filenamepart, filenameonly);
        concatenate(filename.m_path, fileext, filename.m_alloc, 16);
    }

    path_t& path_t::operator=(const runes_t& path)
    {
        if (m_alloc != nullptr)
        {
            erase();
            copy(path, m_path, m_alloc, 16);
        }
        return *this;
    }

    path_t& path_t::operator=(const path_t& path)
    {
        if (this == &path)
            return *this;

        if (m_alloc != nullptr)
        {
            if (path.m_alloc == m_alloc)
            {
                copy(path.m_path, m_path, m_alloc, 16);
            }
            else
            { // Allocator changed
                m_alloc->deallocate(m_path);
                m_alloc = path.m_alloc;
                copy(path.m_path, m_path, m_alloc, 16);
            }
        }
        else
        {
            m_alloc = path.m_alloc;
            copy(path.m_path, m_path, m_alloc, 16);
        }
        return *this;
    }

    path_t& path_t::operator+=(const runes_t& r)
    {
        concatenate(m_path, r, m_alloc, 16);
        return *this;
    }

    bool path_t::operator==(const path_t& rhs) const { return compare(m_path, rhs.m_path) == 0; }
    bool path_t::operator!=(const path_t& rhs) const { return compare(m_path, rhs.m_path) != 0; }

    void path_t::as_utf16(path_t const& p, path_t& dst)
    {
    }
    
    void path_t::as_utf16(filepath_t const& fp, path_t& dst)
    {
    }

    void path_t::as_utf16(filepath_t const& fp, filepath_t& dst)
    {
    }
    
    void path_t::as_utf16(dirpath_t const& dp, path_t& dst)
    {
    }

    void path_t::as_utf16(dirpath_t const& dp, dirpath_t& dst)
    {
    }


} // namespace xcore
