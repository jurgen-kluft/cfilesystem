#include "xbase/x_allocator.h"
#include "xbase/x_binary_search.h"
#include "xbase/x_debug.h"
#include "xbase/x_hash.h"
#include "xbase/x_runes.h"
#include "xbase/x_target.h"

#include "xfilesystem/private/x_filedevice.h"
#include "xfilesystem/private/x_filesystem.h"
#include "xfilesystem/x_dirpath.h"
#include "xfilesystem/x_filepath.h"

namespace xcore
{
    //  What are the benefits of using a table like below to manage filepaths and dirpaths?
    //  - Sharing of strings
    //  - Easy manipulation of dirpath, can easily go to parent or child directory, likely without doing any allocations
    //  - You can prime the table which then results in no allocations when you are using existing filepaths and dirpaths
    //  - Combining dirpath with filepath becomes very easy
    //  - No need to deal with different types of slashes
    //
    //  Use cases:
    //  - From filesysroot_t* you can ask for the root directory of a device
    //    - tdirpath_t appdir = root->device_root("appdir");
    //  - So now with an existing tdirpath_t dir, you could do the following:
    //    - tdirpath_t bins = appdir->down("bin") // even if this folder doesn't exist, it will be 'added'
    //    - tfilepath_t coolexe = bins->file("cool.exe");
    //    - pathname_t* datafilename; pathname_t* dataextension; root->filename("data.txt", datafilename, dataextension);
    //    - tfilepath_t datafilepath = bins->file(datafilename, dataextension);
    //    - stream_t datastream = datafilepath->open();
    //      datastream.close();

    struct pathname_t;
    struct pathdevice_t;

    class filesysroot_t;
    class path_t;



    void fullpath_parser_utf32::parse(const crunes_t& fullpath)
    {
        utf32::rune slash_chars[] = {'\\', '\0'};
        crunes_t    slash(slash_chars);
        utf32::rune devicesep_chars[] = {':', '\\', '\0'};
        crunes_t    devicesep(devicesep_chars);

        m_device          = findSelectUntilIncluded(fullpath, devicesep);
        crunes_t filepath = selectAfterExclude(fullpath, m_device);
        m_path            = findLastSelectUntil(filepath, slash);
        m_filename        = selectAfterExclude(fullpath, m_path);
        trimLeft(m_filename, '\\');
        m_filename     = findLastSelectUntil(m_filename, '.');
        m_extension    = selectAfterExclude(fullpath, m_filename);
        m_first_folder = findSelectUntil(m_path, slash);
        m_last_folder  = findLastSelectUntil(m_path, slash);
        m_last_folder  = selectAfterExclude(m_path, m_last_folder);
        trimLeft(m_last_folder, '\\');
        trimRight(m_device, devicesep);
    }

    bool fullpath_parser_utf32::next_folder(crunes_t& folder) const
    {
        // example: projects\binary_reader\bin\ 
        folder = selectAfterExclude(m_path, folder);
        trimLeft(folder, '\\');
        folder = findSelectUntil(folder, '\\');
        return !folder.is_empty();
    }

    bool fullpath_parser_utf32::prev_folder(crunes_t& folder) const
    {
        // example: projects\binary_reader\bin\ 
        folder = selectBeforeExclude(m_path, folder);
        if (folder.is_empty())
            return false;
        trimRight(folder, '\\');
        crunes_t prevfolder = findLastSelectAfter(folder, '\\');
        if (!prevfolder.is_empty())
        {
            folder = prevfolder;
        }
        return true;
    }



    // 
    // pathname_t implementations
    // 
    pathname_t::pathname_t(s32 strlen) : m_next(nullptr), m_id(-1), m_refs(0), m_len(strlen)
    {
        m_name[0]      = utf32::TERMINATOR;
        m_name[strlen] = utf32::TERMINATOR;
    }

    bool pathname_t::isEmpty() const { return m_len == 0; }

    s32 pathname_t::compare(const pathname_t* name, const pathname_t* other)
    {
        crunes_t cname(name->m_name, name->m_len);
        crunes_t cother(other->m_name, other->m_len);
        return xcore::compare(cname, cother);
    }

    s32 pathname_t::compare(const pathname_t* other) const
    {
        crunes_t name(m_name, m_len);
        crunes_t othername(other->m_name, other->m_len);
        return xcore::compare(name, othername);
    }

    s32 pathname_t::compare(crunes_t const& name) const { return xcore::compare(crunes_t(m_name, m_len), name); }

    pathname_t* pathname_t::incref() { m_refs++; return this;  }
    pathname_t* pathname_t::release(alloc_t* allocator) 
    {
        if (pathname_t::release(this))
        {
            allocator->deallocate(this);
            return nullptr;
        }
        return this;
    }

    bool pathname_t::release(pathname_t* name)
    {
        if (name->m_refs > 0)
        {
            name->m_refs -= 1;
            return name->m_refs == 0;
        }
        return false;
    }

    pathname_t* pathname_t::construct(alloc_t* allocator, u64 hname, crunes_t const& name)
    {
        u32 const strlen = name.size();
        u32 const strcap = strlen;

        void*    name_mem = allocator->allocate(sizeof(pathname_t) + (sizeof(utf32::rune) * strcap), sizeof(void*));
        pathname_t* pname    = new (name_mem) pathname_t(strcap);

        pname->m_hash = hname;
        pname->m_next = nullptr;
        pname->m_refs = 0;
        runes_t dststr(pname->m_name, pname->m_name, pname->m_name + strlen);
        copy(name, dststr);

        return pname;
    }



    // mechanism: copy-on-write


    //
    // pathname_table_t implementations
    // 
    void pathname_table_t::initialize(alloc_t* allocator, s32 cap)
    {
        m_len = 0;
        m_cap = cap;
        m_table = (pathname_t**)allocator->allocate(sizeof(pathname_t*) * cap);
        for (s32 i = 0; i < m_cap; i++)
            m_table[i] = nullptr;
    }

    pathname_t* pathname_table_t::find(u64 hash) const
    {
        u32 index = hash_to_index(hash);
        return m_table[index];
    }

    pathname_t* pathname_table_t::next(u64 hash, pathname_t* prev) const
    {
        return prev->m_next;
    }

    void pathname_table_t::insert(pathname_t* name)
    {
        u32 const index = hash_to_index(name->m_hash);
        pathname_t** iter = &m_table[index];
        while (*iter!=nullptr)
        {
            if (name->m_hash <= (*iter)->m_hash)
            {
                name->m_next = (*iter);
                *iter = name;
                return;
            }
            iter = &((*iter)->m_next);
        }
        *iter = name;
    }

    bool pathname_table_t::remove(pathname_t* name)
    {
        u32 const index = hash_to_index(name->m_hash);
        if (name == m_table[index])
        {
            m_table[index] = name->m_next;
        }
        else
        {
            pathname_t* iter = m_table[index];
            while (iter != nullptr)
            {
                if (iter->m_next == name)
                {
                    iter->m_next = name->m_next;
                    return true;
                }
                iter = iter->m_next;
            }
        }
        return false;
    }

    u32  pathname_table_t::hash_to_index(u64 hash) const
    {
        u32 index = (u32)(hash & (m_len - 1));
        return index;
    }

    //
    // filesysroot_t functions
    //
    void filesysroot_t::initialize(alloc_t* allocator)
    {
        m_allocator = allocator;

        m_filename_table.initialize(m_allocator, 65536);
        m_extension_table.initialize(m_allocator, 8192);
    }

    void filesysroot_t::release_name(pathname_t* name) 
    {
        if (name == sNilName)
            return;

        if (pathname_t::release(name))
        {
            m_filename_table.remove(name);
            name = name->release(m_allocator);
        }
    }

    void filesysroot_t::release_filename(pathname_t* name) 
    {
        release_name(name);
    }

    void filesysroot_t::release_extension(pathname_t* name) 
    {
        if (name == sNilName)
            return;

        if (pathname_t::release(name))
        {
            m_extension_table.remove(name);
            name = name->release(m_allocator);
        }
    }

    void filesysroot_t::release_path(path_t* path)
    {
        if (path == sNilPath)
            return;

        if (path->detach(this))
        { 
            path_t::destruct(m_allocator, path);
        }
    }

    void filesysroot_t::release_device(pathdevice_t* dev) {}

    pathname_t* filesysroot_t::register_name(crunes_t const& namestr) 
    { 
        const u64 hname = calchash(namestr);
        pathname_t* name = m_filename_table.find(hname);
        while (name != nullptr && xcore::compare(namestr, name->m_name) != 0)
        {
            name = m_filename_table.next(hname, name);
        }
        if (name == nullptr)
        {
            name = pathname_t::construct(m_allocator, hname, namestr);
            m_filename_table.insert(name);
        }
        return name;
    }

    pathname_t* filesysroot_t::get_empty_name() const
    {
        return sNilName;
    }

    bool filesysroot_t::register_directory(crunes_t const& directory, pathname_t*& out_devicename, path_t*& out_path)
    {
        fullpath_parser_utf32 parser;
        parser.parse(directory);

        if (parser.has_device())
        {
            out_devicename = register_name(parser.m_device);
        }
        else
        {
            out_devicename = sNilName;
        }

        if (parser.has_path())
        {
            crunes_t folder = parser.iterate_folder();
            s32 c = 1;
            while (parser.next_folder(folder))
            {
                c += 1;
            }

            out_path = path_t::construct(m_allocator, c);
            crunes_t folder = parser.iterate_folder();
            c = 0;
            out_path->m_path[c++] = register_dirname(folder);
            while (parser.next_folder(folder))
            {
                out_path->m_path[c++] = register_dirname(folder);
            }
        }
        else
        {
            out_path = sNilPath;
        }

        return true;
    }

    bool filesysroot_t::register_directory(path_t** paths_to_concatenate, s32 paths_len, path_t*& out_path)
    {
        s32 depth = 0;
        for (s32 i = 0; i < paths_len; i++)
        {
            depth += paths_to_concatenate[i]->m_len;
        }
        out_path = path_t::construct(m_allocator, depth);
        
        depth = 0;
        for (s32 i = 0; i < paths_len; i++)
        {
            path_t::copy_array(paths_to_concatenate[i]->m_path, 0, paths_to_concatenate[i]->m_len, out_path->m_path, depth);
            depth += paths_to_concatenate[i]->m_len;
        }
        return true;
    }

    bool filesysroot_t::register_filename(crunes_t const& fullfilename, pathname_t*& out_filename, pathname_t*& out_extension)
    {
        // split filename into name+extension
        crunes_t filename = findLastSelectUntil(fullfilename, ".");
        crunes_t extension = selectAfterExclude(fullfilename, filename);
        out_filename = register_name(filename);
        out_extension = register_extension(extension);
        return false;
    }

    bool filesysroot_t::register_fullfilepath(crunes_t const& fullfilepath, pathname_t*& out_devicename, path_t*& out_path, pathname_t*& out_filename, pathname_t*& out_extension)
    {

    }

    pathname_t* filesysroot_t::register_dirname(crunes_t const& fulldirname)
    {
        return register_name(fulldirname);
    }

    pathname_t* filesysroot_t::register_extension(crunes_t const& extension) 
    { 
        const u64 hname = calchash(extension);
        pathname_t* name = m_extension_table.find(hname);
        while (name != nullptr && xcore::compare(extension, name->m_name) != 0)
        {
            name = m_extension_table.next(hname, name);
        }
        if (name == nullptr)
        {
            name = pathname_t::construct(m_allocator, hname, extension);
            m_extension_table.insert(name);
        }
        return name;
    }

    pathdevice_t* filesysroot_t::register_device(crunes_t const& device)
    {
        for (s32 i = 0; i < m_num_devices; ++i)
        {
            if (xcore::compare(device, m_tdevice[i].m_name->m_name) == 0)
            {
                return &m_tdevice[i];
            }
        }
        return sNilDevice;
    }

    pathdevice_t* filesysroot_t::register_device(pathname_t* device)
    {
        for (s32 i = 0; i < m_num_devices; ++i)
        {
            if (device == m_tdevice[i].m_name)
            {
                return &m_tdevice[i];
            }
        }
        return sNilDevice;
    }

    path_t* filesysroot_t::get_parent_path(path_t* path)
    {
        if (path->m_len == 0)
            return sNilPath;
        if (path->m_len == 1)
            return sNilPath;

        s32 depth = path->m_len - 1;
        path_t* out_path = path_t::construct(m_allocator, depth);
        path_t::copy_array(path->m_path, 0, depth, out_path->m_path, 0);
        return out_path;
    }

    void filesysroot_t::get_split_path(path_t* path, s32 pivot, path_t** left, path_t** right)
    {
        if (pivot == 0)
        {
            if (left != nullptr)
            {
                *left = path;
            }
            if (right != nullptr)
            {
                *right = nullptr;
            }
        }
        else if (pivot >= path->m_len)
        {
            if (left != nullptr)
            {
                *left = nullptr;
            }
            if (right != nullptr)
            {
                *right = path;
            }
        }
        else
        {
            if (left != nullptr)
            {
                s32 len = pivot;
                path_t* newpath = path_t::construct(m_allocator, len);
                path_t::copy_array(path->m_path, 0, len, newpath->m_path, 0);
                *left = newpath;
            }
            if (right != nullptr)
            {
                s32 len = path->m_len - pivot;
                path_t* newpath = path_t::construct(m_allocator, len);
                path_t::copy_array(path->m_path, 0, len, newpath->m_path, 0);
                *right = newpath;
            }
        }
    }

    //
    // path_t implementations
    // 

    path_t* path_t::attach() 
    { 
        m_ref++; 
        return this; 
    }

    bool path_t::detach(filesysroot_t* root)
    {
        if (m_ref > 0)
        {
            m_ref -= 1;
            if (m_ref == 0)
            {
                for (s32 i = 0; i < m_len; i++)
                {
                    pathname_t* dirname = m_path[i];
                    dirname->release(root->m_allocator);
                }
                return true;
            }
        }
        return false;
    }

    pathname_t* path_t::get_name() const
    {
        if (m_len == 0) 
            return filesysroot_t::sNilName;
        return m_path[m_len - 1];
    }

    path_t* path_t::prepend(pathname_t* folder, alloc_t* allocator)
    {
        path_t* path = construct(allocator, folder->m_len + 1);
        copy_array(m_path, 0, m_len, path->m_path, 1);
        path->m_path[0] = folder->incref();
        return path;
    }

    path_t* path_t::append(pathname_t* folder, alloc_t* allocator)
    {
        path_t* path = construct(allocator, folder->m_len + 1);
        copy_array(m_path, 0, m_len, path->m_path, 0);
        pathname_t* f = 
            path->m_path[m_len - 1] = folder->incref();
        return path;
    }

    s32 path_t::compare(path_t* other) const
    {
        if (other == this)
            return 0;

        if (m_len < other->m_len)
            return -1;
        if (m_len > other->m_len)
            return 1;
        for (s32 i = 0; i < m_len; i++)
        {
            s32 c = m_path[i]->compare(other->m_path[i]);
            if (c != 0)
                return c;
        }
        return 0;
    }

    path_t* path_t::construct(alloc_t* allocator, s32 cap)
    {
        ASSERT(cap > 0);
        s32 const allocsize = sizeof(path_t) + (((cap + 3) & ~3) - 1);
        path_t* path = (path_t*)allocator->allocate(allocsize);
        path->m_cap = (((cap + 3) & ~3) - 1);
        path->m_len = 0;
        path->m_ref = 0;
        path->m_path[0] = '\0';
        path->m_path[1] = '\0';
        return path;
    }

    void path_t::destruct(alloc_t* allocator, path_t* path)
    {
        allocator->deallocate(path);
    }

    void path_t::copy_array(pathname_t** from, u32 from_start, u32 from_len, pathname_t** to, u32 to_start)
    {
        pathname_t* const* src = from + from_start;
        pathname_t* const* end = src + from_len;
        pathname_t** dst = to + to_start;
        while (src < end)
            *dst++ = *src++;
    }


    //
    // pathdevice_t implementations
    // 
    void pathdevice_t::init(filesysroot_t* owner)
    {
        m_name = nullptr;
        m_root = owner;
        m_path = nullptr;
        m_fd   = x_NullFileDevice();
    }

    pathdevice_t* pathdevice_t::construct(alloc_t* allocator, filesysroot_t* owner)
    {
        // Allocate a pathdevice_t
        void*      device_mem = allocator->allocate(sizeof(pathdevice_t), sizeof(void*));
        pathdevice_t* device     = static_cast<pathdevice_t*>(device_mem);
        device->init(owner);
        return device;
    }
    void pathdevice_t::destruct(alloc_t* allocator, pathdevice_t*& device)
    {
        allocator->deallocate(device);
        device = nullptr;
    }

}; // namespace xcore