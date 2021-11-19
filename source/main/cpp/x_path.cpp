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
    //  - From filesys_t* you can ask for the root directory of a device
    //    - tdirpath_t appdir = root->device_root("appdir");
    //  - So now with an existing tdirpath_t dir, you could do the following:
    //    - tdirpath_t bins = appdir->down("bin") // even if this folder doesn't exist, it will be 'added'
    //    - tfilepath_t coolexe = bins->file("cool.exe");
    //    - pathname_t* datafilename; pathname_t* dataextension; root->filename("data.txt", datafilename, dataextension);
    //    - tfilepath_t datafilepath = bins->file(datafilename, dataextension);
    //    - stream_t datastream = datafilepath->open();
    //      datastream.close();

    struct path_t;
    struct pathname_t;
    struct pathdevice_t;

    class filesys_t;



    void fullpath_parser_utf32::parse(const crunes_t& fullpath)
    {
        utf32::rune slash_chars[] = {'\\', '\0'};
        crunes_t    slash(slash_chars);
        utf32::rune devicesep_chars[] = {':', '\\', '\0'};
        crunes_t    devicesep(devicesep_chars);

        m_device          = findSelectUntilIncluded(fullpath, devicesep);
        crunes_t filepath = selectAfterExclude(fullpath, m_device);
        m_path            = findLastSelectUntilIncluded(filepath, slash);
        m_filename        = selectAfterExclude(fullpath, m_path);
        m_filename     = findLastSelectUntil(m_filename, '.');
        m_extension    = selectAfterExclude(fullpath, m_filename);
        m_first_folder = findSelectUntil(m_path, slash);
        m_last_folder  = findLastSelectUntil(m_path, slash);
        m_last_folder  = selectAfterExclude(m_path, m_last_folder);
        trimLeft(m_last_folder, '\\');
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
    pathname_t::pathname_t() : m_hash(0), m_next(nullptr), m_refs(0), m_len(0)
    {
        m_name[0] = utf32::TERMINATOR;
        m_name[1] = utf32::TERMINATOR;
    }
    pathname_t::pathname_t(s32 strlen) : m_hash(0), m_next(nullptr), m_refs(0), m_len(strlen)
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
        if (m_len == other->m_len)
        {
            crunes_t name(m_name, m_len);
            crunes_t othername(other->m_name, other->m_len);
            return xcore::compare(name, othername);
        }
        return false;
    }

    s32 pathname_t::compare(crunes_t const& name) const { return xcore::compare(crunes_t(m_name, m_len), name); }

    pathname_t* pathname_t::attach() { m_refs++; return this;  }

    bool pathname_t::detach()
    {
        if (m_refs > 0)
        {
            m_refs -= 1;
            return m_refs == 0;
        }
        return false;
    }

    void pathname_t::release(alloc_t* allocator) 
    {
        allocator->deallocate(this);
    }

    pathname_t* pathname_t::construct(alloc_t* allocator, u64 hname, crunes_t const& name)
    {
        u32 const strlen = name.size();
        u32 const strcap = strlen - 1;

        void*    name_mem = allocator->allocate(sizeof(pathname_t) + (sizeof(utf32::rune) * strcap), sizeof(void*));
        pathname_t* pname = new (name_mem) pathname_t(strlen);

        pname->m_hash = hname;
        pname->m_next = nullptr;
        pname->m_refs = 0;
        runes_t dststr(pname->m_name, pname->m_name + strlen);
        copy(name, dststr);

        return pname;
    }

    void    pathname_t::to_string(runes_t& out_str) const
    {
        crunes_t namestr(m_name, m_name + m_len);
        xcore::concatenate(out_str, namestr);
    }


    //
    // pathname_table_t implementations
    // 
    void pathname_table_t::init(alloc_t* allocator, s32 cap)
    {
        m_len = 0;
        m_cap = cap;
        m_table = (pathname_t**)allocator->allocate(sizeof(pathname_t*) * cap);
        for (s32 i = 0; i < m_cap; i++)
            m_table[i] = nullptr;
    }

    void pathname_table_t::release(alloc_t* allocator)
    {
        allocator->deallocate(m_table);
        m_table = nullptr;
        m_len = 0;
        m_cap = 0;
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
                if (*iter==name || (name->m_hash == (*iter)->m_hash && name->compare(*iter) == 0))
                {
                    // already exists
                    return;
                }
                else
                {
                    name->m_next = (*iter);
                    *iter = name;
                    return;
                }
            }
            iter = &((*iter)->m_next);
        }
        *iter = name;
    }

    bool pathname_table_t::remove(pathname_t* name)
    {
        u32 const index = hash_to_index(name->m_hash);
        pathname_t** iter = &m_table[index];
        while (*iter != nullptr)
        {
            if (*iter == name)
            {
                *iter = name->m_next;
                return true;
            }
            iter = &((*iter)->m_next);
        }
        return false;
    }

    u32  pathname_table_t::hash_to_index(u64 hash) const
    {
        u32 const index = (u32)(hash & (m_cap - 1));
        return index;
    }

    //
    // filesys_t functions
    //
    pathdevice_t filesys_t::sNilDevice;
    pathname_t   filesys_t::sNilName;
    path_t       filesys_t::sNilPath;

    void filesys_t::init(alloc_t* allocator)
    {
        m_allocator = allocator;

        m_filename_table.init(m_allocator, 65536);
        m_extension_table.init(m_allocator, 8192);

        sNilName.m_hash = 0;
        sNilName.m_len = 0;
        sNilName.m_name[0] = utf32::TERMINATOR;
        sNilName.m_next = nullptr;
        sNilName.m_refs = 0;

        sNilPath.m_cap = 1;
        sNilPath.m_len = 0;
        sNilPath.m_path[0] = nullptr;
        sNilPath.m_ref = 0;

        sNilDevice.m_root = this;
        sNilDevice.m_alias = &sNilName;
        sNilDevice.m_deviceName = &sNilName;
        sNilDevice.m_devicePath = &sNilPath;
        sNilDevice.m_redirector = nullptr;
        sNilDevice.m_fileDevice = nullptr;
    }

    void filesys_t::exit(alloc_t* allocator)
    {
        for (s32 i = 0; i < m_num_devices; ++i)
        {
            release_name(m_tdevice[i].m_alias);
            release_name(m_tdevice[i].m_deviceName);
            release_path(m_tdevice[i].m_devicePath);
            m_tdevice[i].m_root = nullptr;
            m_tdevice[i].m_alias = nullptr;
            m_tdevice[i].m_deviceName = nullptr;
            m_tdevice[i].m_devicePath = nullptr;
            m_tdevice[i].m_fileDevice = nullptr;
            m_tdevice[i].m_redirector = nullptr;
        }
        m_num_devices = 0;

        m_filename_table.release(allocator);
        m_extension_table.release(allocator);
    }

    void filesys_t::release_name(pathname_t* name) 
    {
        if (name == &sNilName)
            return;

        if (name->detach())
        {
            m_filename_table.remove(name);
            name->release(m_allocator);
        }
    }

    void filesys_t::release_filename(pathname_t* name) 
    {
        release_name(name);
    }

    void filesys_t::release_extension(pathname_t* name) 
    {
        if (name == &sNilName)
            return;

        if (name->detach())
        {
            m_extension_table.remove(name);
            name->release(m_allocator);
        }
    }

    void filesys_t::release_path(path_t* path)
    {
        if (path == &sNilPath)
            return;

        if (path->detach(this))
        { 
            path_t::destruct(m_allocator, path);
        }
    }

    void filesys_t::release_device(pathdevice_t* dev) {}

    pathname_t* filesys_t::find_name(crunes_t const& namestr) const
    {
        const u64 hname = calchash(namestr);
        pathname_t* name = m_filename_table.find(hname);
        while (name != nullptr && xcore::compare(namestr, crunes_t(name->m_name, name->m_len)) != 0)
        {
            name = m_filename_table.next(hname, name);
        }
        return name;
    }

    pathname_t* filesys_t::register_name(crunes_t const& namestr) 
    { 
        pathname_t* name = find_name(namestr);
        if (name == nullptr)
        {
            const u64 hname = calchash(namestr);
            name = pathname_t::construct(m_allocator, hname, namestr);
            m_filename_table.insert(name);
        }
        return name;
    }

    pathname_t* filesys_t::get_empty_name() const
    {
        return &sNilName;
    }

    bool filesys_t::register_directory(crunes_t const& directory, pathname_t*& out_devicename, path_t*& out_path)
    {
        fullpath_parser_utf32 parser;
        parser.parse(directory);

        if (parser.has_device())
        {
            out_devicename = register_name(parser.m_device);
        }
        else
        {
            out_devicename = &sNilName;
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
            folder = parser.iterate_folder();
            out_path->m_path[out_path->m_len++] = register_dirname(folder);
            while (parser.next_folder(folder) && out_path->m_len < out_path->m_cap)
            {
                out_path->m_path[out_path->m_len++] = register_dirname(folder);
            }
            for (c = 0; c < out_path->m_len; c++)
            {
                out_path->m_path[c]->attach();
            }
        }
        else
        {
            out_path = &sNilPath;
        }

        return true;
    }

    bool filesys_t::register_directory(path_t** paths_to_concatenate, s32 paths_len, path_t*& out_path)
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

    bool filesys_t::register_filename(crunes_t const& fullfilename, pathname_t*& out_filename, pathname_t*& out_extension)
    {
        // split filename into name+extension
        crunes_t filename = findLastSelectUntil(fullfilename, ".");
        crunes_t extension = selectAfterExclude(fullfilename, filename);
        out_filename = register_name(filename);
        out_extension = register_extension(extension);
        return false;
    }

    bool filesys_t::register_fullfilepath(crunes_t const& fullfilepath, pathname_t*& out_devicename, path_t*& out_path, pathname_t*& out_filename, pathname_t*& out_extension)
    {
        fullpath_parser_utf32 parser;
        parser.parse(fullfilepath);

        if (parser.has_device())
        {
            out_devicename = register_name(parser.m_device);
        }
        else
        {
            out_devicename = &sNilName;
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
            folder = parser.iterate_folder();
            c = 0;
            out_path->m_path[c++] = register_dirname(folder);
            while (parser.next_folder(folder))
            {
                out_path->m_path[c++] = register_dirname(folder);
            }
        }
        else
        {
            out_path = &sNilPath;
        }

        if (parser.has_filename())
        { 
            out_filename = register_name(parser.m_filename);
        }
        else {
            out_filename = &sNilName;
        }

        if (parser.has_extension())
        { 
            out_extension = register_extension(parser.m_extension);
        }
        else {
            out_extension = &sNilName;
        }

        return true;

    }

    pathname_t* filesys_t::register_dirname(crunes_t const& fulldirname)
    {
        return register_name(fulldirname);
    }

    pathname_t* filesys_t::register_extension(crunes_t const& extension) 
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

    pathdevice_t* filesys_t::register_device(crunes_t const& device)
    {
        pathname_t* devicename = register_name(device);
        return register_device(devicename);
    }

    pathdevice_t* filesys_t::find_device(pathname_t* devicename) const
    {
        for (s32 i = 0; i < m_num_devices; ++i)
        {
            if (m_tdevice[i].m_deviceName == devicename)
            {
                return (pathdevice_t*)&m_tdevice[i];
            }
        }
        return nullptr;
    }

    pathdevice_t* filesys_t::register_device(pathname_t* devicename)
    {
        pathdevice_t* device = find_device(devicename);
        if (device == nullptr)
        {
            if (m_num_devices < 64)
            {
                m_tdevice[m_num_devices].m_root = this;
                m_tdevice[m_num_devices].m_alias = &sNilName;
                m_tdevice[m_num_devices].m_deviceName = devicename;
                m_tdevice[m_num_devices].m_devicePath = &sNilPath;
                m_tdevice[m_num_devices].m_redirector = nullptr;
                m_tdevice[m_num_devices].m_fileDevice = nullptr;
                device = &m_tdevice[m_num_devices];
                m_num_devices++;
            }
            else 
            {
                device = &sNilDevice;
            }
        }
        return device;
    }

    path_t* filesys_t::get_parent_path(path_t* path)
    {
        if (path->m_len == 0)
            return &sNilPath;
        if (path->m_len == 1)
            return &sNilPath;

        s32 depth = path->m_len - 1;
        path_t* out_path = path_t::construct(m_allocator, depth);
        path_t::copy_array(path->m_path, 0, depth, out_path->m_path, 0);
        return out_path;
    }

    void filesys_t::get_expand_path(path_t* path, pathname_t* folder, path_t*& out_path)
    {
        s32 depth = path->m_len + 1;
        out_path = path_t::construct(m_allocator, depth);
        path_t::copy_array(path->m_path, 0, depth-1, out_path->m_path, 0);
        out_path->m_path[path->m_len] = folder;
    }

    void filesys_t::get_expand_path(pathname_t* folder, path_t* path, path_t*& out_path)
    {
        s32 depth = path->m_len + 1;
        out_path = path_t::construct(m_allocator, depth);
        path_t::copy_array(path->m_path, 0, depth-1, out_path->m_path, 1);
        out_path->m_path[0] = folder;
    }

    void filesys_t::get_expand_path(path_t* left, s32 lstart, s32 llen, path_t* right, s32 rstart, s32 rlen, path_t*& out_path)
    {
        s32 depth = llen + rlen;
        out_path = path_t::construct(m_allocator, depth);
        path_t::copy_array(left->m_path, lstart, llen, out_path->m_path, 0);
        path_t::copy_array(right->m_path, rstart, rlen, out_path->m_path, llen);
    }

    void filesys_t::get_split_path(path_t* path, s32 pivot, path_t** left, path_t** right)
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

    bool path_t::detach(filesys_t* root)
    {
        if (m_ref > 0)
        {
            m_ref -= 1;
            if (m_ref == 0)
            {
                for (s32 i = 0; i < m_len; i++)
                {
                    pathname_t* dirname = m_path[i];
                    root->release_name(dirname);
                }
                return true;
            }
        }
        return false;
    }

    pathname_t* path_t::get_name() const
    {
        if (m_len == 0) 
            return &filesys_t::sNilName;
        return m_path[m_len - 1];
    }

    path_t* path_t::prepend(pathname_t* folder, alloc_t* allocator)
    {
        path_t* path = construct(allocator, folder->m_len + 1);
        copy_array(m_path, 0, m_len, path->m_path, 1);
        path->m_path[0] = folder->attach();
        return path;
    }

    path_t* path_t::append(pathname_t* folder, alloc_t* allocator)
    {
        path_t* path = construct(allocator, folder->m_len + 1);
        copy_array(m_path, 0, m_len, path->m_path, 0);
        path->m_path[m_len - 1] = folder->attach();
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

    void    path_t::to_string(runes_t& str) const
    {
        const char* slash = "\\";
        crunes_t slashstr(slash, slash + 1);
        for (s32 i = 0; i < m_len; i++)
        {
            pathname_t* dirname = m_path[i];
            crunes_t dirstr(dirname->m_name, dirname->m_name + dirname->m_len);
            xcore::concatenate(str, dirstr);
            xcore::concatenate(str, slashstr);
        }
    }

    path_t* path_t::construct(alloc_t* allocator, s32 cap)
    {
        ASSERT(cap > 0);
        cap = ((cap + 3) & ~3);
        s32 const allocsize = sizeof(path_t) + (sizeof(pathname_t*) * (cap - 1));
        path_t* path = (path_t*)allocator->allocate(allocsize);
        path->m_cap = cap;
        path->m_len = 0;
        path->m_ref = 0;
        path->m_path[0] = nullptr;
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
    void pathdevice_t::init(filesys_t* owner)
    {
        m_root = owner;
        m_alias = &owner->sNilName;
        m_deviceName = &owner->sNilName;
        m_devicePath = &owner->sNilPath;
        m_redirector = nullptr;
        m_fileDevice = x_NullFileDevice();
    }

    pathdevice_t* pathdevice_t::construct(alloc_t* allocator, filesys_t* owner)
    {
        void*      device_mem = allocator->allocate(sizeof(pathdevice_t), sizeof(void*));
        pathdevice_t* device  = static_cast<pathdevice_t*>(device_mem);
        device->init(owner);
        return device;
    }

    void pathdevice_t::destruct(alloc_t* allocator, pathdevice_t*& device)
    {
        allocator->deallocate(device);
        device = nullptr;
    }

    pathdevice_t* pathdevice_t::attach()
    {
        m_alias->attach();
        m_deviceName->attach();
        m_devicePath->attach();
        if (m_redirector!=nullptr)
            m_redirector->attach();
        return this;
    }

    bool pathdevice_t::detach(filesys_t* root)
    {
        root->release_name(m_alias);
        root->release_name(m_deviceName);
        root->release_path(m_devicePath);
        if (m_redirector != nullptr)
            m_redirector->detach(root);

        m_alias = &root->sNilName;
        m_deviceName = &root->sNilName;
        m_devicePath = &root->sNilPath;
        m_redirector = nullptr;
        m_fileDevice = nullptr;
        
        return false;
    }

    s32 pathdevice_t::compare(pathdevice_t* device) const
    {
        if (m_deviceName == device->m_deviceName)
            return 0;
        else if (m_deviceName < device->m_deviceName)
            return -1;
        return 1;
    }


    void pathdevice_t::to_string(runes_t& str) const
    {
        s32 i = 0;
        pathdevice_t const* device = this;
        pathdevice_t const* devices[32];
        do
        {
            devices[i++] = device;
            device = device->m_redirector;
        } while (device != nullptr && i < 32);

        device = devices[--i];

        // should be the root device (has filedevice), so first emit the device name.
        // this device should not have any device path.
        device->m_deviceName->to_string(str);

        // the rest of the devices are aliases and should be appending their paths
        while (--i >= 0)
        {
            device = devices[i];
            device->m_devicePath->to_string(str);
        }
    }

}; // namespace xcore