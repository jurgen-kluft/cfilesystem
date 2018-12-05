#include "xbase/x_target.h"
#include "xbase/x_debug.h"
#include "xbase/x_runes.h"

#include "xfilesystem/private/x_path.h"

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

    xpath::xpath() : m_alloc(nullptr), m_path() {}

    xpath::xpath(utf16::alloc* allocator)
        : m_alloc(allocator), m_path()
    {
        fix_slashes(m_path);
    }

    xpath::xpath(const xpath& path) : m_alloc(path.m_alloc), m_path()
    {
        copy(path.m_path, m_path, m_alloc, 16);
    }

    bool xpath::isEmpty() const { return m_path.is_empty(); }
    bool xpath::isRoot() const
    {
        runes rootpart = find(m_path, runez<4>(":\\"));
        if (rootpart.is_empty())
            return false;
        runes pathpart = selectUntilEndExcludeSelection(m_path, rootpart);
        return pathpart.is_empty();
    }

    bool xpath::isRooted() const
    {
        runes pos = find(m_path, runez<4>(":\\"));
        return !pos.is_empty();
    }

    bool xpath::isSubDirOf(const xpath& dirpath) const
    {

    }

    void xpath::set_filepath(utf16::runes& runes, utf16::alloc* allocator)
    {
        erase();
        m_alloc = allocator;
        m_path = runes;

        uchar32 slash = '\\';

        // Replace incorrect slashes with the correct one
        replace(m_path, (uchar32)'/', (uchar32)'\\');

        // Remove double slashes like '\\' or '//'

        // Remove slash at the beginning and end
        trim(m_path, slash);

    }
    
    void xpath::set_dirpath(utf16::runes& runes, utf16::alloc* allocator)
    {
        erase();
        m_alloc = allocator;
        m_path = runes;

        uchar32 slash = '\\';

        // Replace incorrect slashes with the correct one
        replace(m_path, (uchar32)'/', (uchar32)'\\');

        // Remove double slashes like '\\' or '//'

        // Remove slash at the beginning
        trimLeft(m_path, slash);

        // Ensure slash at the end
        if (!ends_with(m_path, '\\'))
        {
            runez<4> slashstr("\\");
            concatenate(m_path, slashstr, m_alloc, 16);
        }
    }

    void xpath::set_combine(const xpath& dirpath, const xpath& filepath)
    {
        erase();

        m_alloc = dirpath.m_alloc;
        if (filepath.isRooted())
        {
            runes relativefilepath = findSelectAfter(filepath.m_path, runez<4>(":\\"));
            concatenate(m_path, dirpath.m_path, relativefilepath, m_alloc, 16);
        }
        else
        {
            concatenate(m_path, dirpath.m_path, filepath.m_path, m_alloc, 16);
        }
    }

    void xpath::clear() { m_path.clear(); }
    void xpath::erase() { if (m_alloc!=nullptr) m_alloc->deallocate(m_path); }

    void xpath::makeRelative()
    {
        if (isRooted())
        {
            runes pos = findSelectUntilIncluded(m_path, runez<4>(":\\"));
            remove(m_path, pos);
        }
    }

    void xpath::makeRelativeTo(const xpath& root)
    {
        if (isRooted())
        {
            // We are rooted, now check if the incoming root path is
            // matching our current root path

        }
    }

    void xpath::setRootDir(const xpath& in_root_dirpath)
    {
        runes rootpart = findSelectUntilIncluded(m_path, runez<4>(":\\"));
        if (rootpart.is_empty() == false)
        {
            insert(m_path, in_root_dirpath.m_path, m_alloc, 16);
        }
        else
        {
            replaceSelection(m_path, rootpart, in_root_dirpath, m_alloc, 16);
        }
    }
     
    bool xpath::getRootDir(xpath& root) const
    {
        runes rootpart = findSelectUntilIncluded(m_path, runez<4>(":\\"));
        if (rootpart.is_empty() == false)
        {
            root.erase();
            copy(root.m_path, rootpart, root.m_alloc, 16);
            return true;
        }
        else
        {
            return false;
        }
    }

    bool xpath::getDir(xpath& outDirPath) const
    {
        if (isEmpty())
            return false;

        outDirPath.clear();
        outDirPath.m_alloc = m_alloc;

        // Select a string until and included the last '\'
        runes dirpart = findLastSelectUntilIncluded(m_path, '\\');
        if (dirpart.is_empty() == false)
        {
            copy(dirpart, outDirPath.m_path, outDirPath.m_alloc, 16);
            return true;
        }
        return false;
    }

    void xpath::getFilename(xpath& filename) const
    {
        runes filenamepart = findLastSelectAfter(m_path, '\\');
        if (!filenamepart.is_empty())
        {
            filename.clear();
            concatenate(filename.m_path, filenamepart, filename.m_alloc, 16);
        }
    }

    void xpath::getFilenameWithoutExtension(xpath& filename) const
    {
        filename.clear();
        runes filenamepart = findLastSelectAfter(m_path, '\\');
        if (!filenamepart.is_empty())
        {
            filenamepart       = findLastSelectUntil(filenamepart, '.');
            concatenate(filename.m_path, filenamepart, filename.m_alloc, 16);
        }
    }

    void xpath::getExtension(xpath& filename) const
    {
        filename.clear();
        runes filenamepart = findLastSelectAfter(m_path, '\\');
        runes filenameonly = findLastSelectUntil(filenamepart, '.');
        runes fileext = selectUntilEndExcludeSelection(filenamepart, filenameonly);
        concatenate(filename.m_path, fileext, filename.m_alloc, 16);
    }

    xpath& xpath::operator=(const xpath& path)
    {
        if (this == &path)
            return *this;

        if (m_alloc != nullptr)
        {
            m_alloc->deallocate(m_path);
        }
        m_alloc = path.m_alloc;
        copy(path.m_path, m_path, m_alloc, 16);
        return *this;
    }

    bool xpath::operator==(const xpath& rhs) const { return compare(m_path, rhs.m_path) == 0; }
    bool xpath::operator!=(const xpath& rhs) const { return compare(m_path, rhs.m_path) != 0; }

} // namespace xcore
