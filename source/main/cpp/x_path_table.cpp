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
    //  - From troot_t* you can ask for the root directory of a device
    //    - tdirpath_t appdir = root->device_root("appdir");
    //  - So now with an existing tdirpath_t dir, you could do the following:
    //    - tdirpath_t bins = appdir->down("bin") // even if this folder doesn't exist, it will be 'added'
    //    - tfilepath_t coolexe = bins->file("cool.exe");
    //    - tname_t* datafilename; tname_t* dataextension; root->filename("data.txt", datafilename, dataextension);
    //    - tfilepath_t datafilepath = bins->file(datafilename, dataextension);
    //    - stream_t datastream = datafilepath->open();
    //      datastream.close();

    struct tname_t;
    struct tdevice_t;

    class troot_t;
    class tpath_t;

    class xpath_parser_utf32
    {
    public:
        crunes_t m_device;
        crunes_t m_path;
        crunes_t m_filename;
        crunes_t m_extension;
        crunes_t m_first_folder;
        crunes_t m_last_folder;

        void parse(const runes_t& fullpath)
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

        bool has_device() const { return !m_device.is_empty(); }
        bool has_path() const { return !m_path.is_empty(); }
        bool has_filename() const { return !m_filename.is_empty(); }
        bool has_extension() const { return !m_extension.is_empty(); }

        crunes_t iterate_folder() const { return m_first_folder; }
        bool     next_folder(crunes_t& folder) const;
        crunes_t last_folder() const { return m_last_folder; }
        bool     prev_folder(crunes_t& folder) const;
    };

    bool xpath_parser_utf32::next_folder(crunes_t& folder) const
    {
        // example: projects\binary_reader\bin\ 
		folder = selectAfterExclude(m_path, folder);
        trimLeft(folder, '\\');
        folder = findSelectUntil(folder, '\\');
        return !folder.is_empty();
    }

    bool xpath_parser_utf32::prev_folder(crunes_t& folder) const
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

    class tname_t
    {
    public:
        u64      m_hash;
        tname_t* m_next;
        u32      m_id;
        s32      m_refs;
        s32      m_len;
        utf32::rune m_name[1];

        tname_t(s32 strlen);

        bool isEmpty() const;

        static s32 compare(const tname_t* name, const tname_t* other);
        s32 compare(const tname_t* other) const;
        s32 compare(crunes_t const& name) const;

        bool operator==(const tname_t& other) const { return compare(&other) == 0; }
        bool operator!=(const tname_t& other) const { return compare(&other) != 0; }

        tname_t* incref();
        tname_t* release(alloc_t* allocator);
        static bool release(tname_t* name);
        static tname_t* construct(alloc_t* allocator, u64 hname, crunes_t const& name);

        XCORE_CLASS_PLACEMENT_NEW_DELETE
    };

    // 
    // tname_t implementations
    // 
    tname_t::tname_t(s32 strlen) : m_next(nullptr), m_id(-1), m_refs(0), m_len(strlen)
    {
        m_name[0]      = utf32::TERMINATOR;
        m_name[strlen] = utf32::TERMINATOR;
    }

    bool tname_t::isEmpty() const { return m_len == 0; }

    s32 tname_t::compare(const tname_t* name, const tname_t* other)
    {
        crunes_t cname(name->m_name, name->m_len);
        crunes_t cother(other->m_name, other->m_len);
        return xcore::compare(cname, cother);
    }

    s32 tname_t::compare(const tname_t* other) const
    {
        crunes_t name(m_name, m_len);
        crunes_t othername(other->m_name, other->m_len);
        return xcore::compare(name, othername);
    }

    s32 tname_t::compare(crunes_t const& name) const { return xcore::compare(crunes_t(m_name, m_len), name); }

    tname_t* tname_t::incref() { m_refs++; return this;  }
    tname_t* tname_t::release(alloc_t* allocator) 
    {
        if (tname_t::release(this))
        {
            allocator->deallocate(this);
            return nullptr;
        }
        return this;
    }
    
    bool tname_t::release(tname_t* name)
    {
        if (name->m_refs > 0)
        {
            name->m_refs -= 1;
            return name->m_refs == 0;
        }
        return false;
    }

    tname_t* tname_t::construct(alloc_t* allocator, u64 hname, crunes_t const& name)
    {
        u32 const strlen = name.size();
        u32 const strcap = strlen;

        void*    name_mem = allocator->allocate(sizeof(tname_t) + (sizeof(utf32::rune) * strcap), sizeof(void*));
        tname_t* pname    = new (name_mem) tname_t(strcap);

        pname->m_hash = hname;
        pname->m_next = nullptr;
        pname->m_refs = 0;
        runes_t dststr(pname->m_name, pname->m_name, pname->m_name + strlen);
        copy(name, dststr);

        return pname;
    }

    struct tdevice_t
    {
        troot_t*      m_root; // If alias, tdevice_t ("work") | nullptr
        filedevice_t* m_fd;   // nullptr                      | xfiledevice("e")
        tname_t*      m_name; // "codedir"                    | "E"
        tname_t*      m_path; // "codedir:\\xfilesystem\\"    | "E:\\dev.go\\src\\github.com\\jurgen-kluft\\"

        void       init(troot_t* owner);

        tdevice_t* attach() { return this; }
        tdevice_t* release(alloc_t* allocator) { return this; }

        static tdevice_t* construct(alloc_t* allocator, troot_t* owner);
        static void       destruct(alloc_t* allocator, tdevice_t*& device);
    };

    // mechanism: copy-on-write
    struct tname_table_t
    {
        void     initialize(alloc_t* allocator, s32 cap=65536);

        tname_t* find(u64 hash) const;
        tname_t* next(u64 hash, tname_t* prev) const;
        void     insert(tname_t* name);
        bool     remove(tname_t* name);
        u32      hash_to_index(u64 hash) const;

        s32       m_len;
        s32       m_cap;
        tname_t** m_table;
    };

    //
    // tname_table_t implementations
    // 
    void tname_table_t::initialize(alloc_t* allocator, s32 cap)
    {
        m_len = 0;
        m_cap = cap;
        m_table = (tname_t**)allocator->allocate(sizeof(tname_t*) * cap);
        for (s32 i = 0; i < m_cap; i++)
            m_table[i] = nullptr;
    }

    tname_t* tname_table_t::find(u64 hash) const
    {
        u32 index = hash_to_index(hash);
        return m_table[index];
    }

    tname_t* tname_table_t::next(u64 hash, tname_t* prev) const
    {
        return prev->m_next;
    }

    void tname_table_t::insert(tname_t* name)
    {
        u32 const index = hash_to_index(name->m_hash);
        tname_t** iter = &m_table[index];
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

    bool tname_table_t::remove(tname_t* name)
    {
        u32 const index = hash_to_index(name->m_hash);
        if (name == m_table[index])
        {
            m_table[index] = name->m_next;
        }
        else
        {
            tname_t* iter = m_table[index];
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

    u32  tname_table_t::hash_to_index(u64 hash) const
    {
        u32 index = (u32)(hash & (m_len - 1));
        return index;
    }


    // reference counted path array, to reduce copying.
    // mechanism: copy-on-write
    struct tpath_t
    {
        s32 m_ref;
        s16 m_len;
        s16 m_cap;
        tname_t* m_path[1];

        tpath_t() : m_ref(0), m_len(0), m_cap(1) { }

        tpath_t* attach();
        bool detach(troot_t* root);

        tname_t* get_name() const;
        tpath_t* prepend(tname_t* folder, alloc_t* allocator);
        tpath_t* append(tname_t* folder, alloc_t* allocator);

        static tpath_t* construct(alloc_t* allocator, s32 len);
        static void destruct(alloc_t* allocator, tpath_t* path);
        static void copy_array(tname_t* const* from, u32 from_start, u32 from_len, tname_t** dst, u32 dst_start);
    };


    // API prototype
    // root table
    class troot_t
    {
    public:
        troot_t(alloc_t* allocator) : m_allocator(allocator) {}

        alloc_t*            m_allocator;
        s32                 m_num_devices;
        s32                 m_max_devices;
        tdevice_t           m_tdevice[64];

        tname_table_t       m_filename_table;
        tname_table_t       m_extension_table;

        static tdevice_t* sNilDevice;
        static tname_t*   sNilName;
        static tpath_t*   sNilPath;
        static troot_t*   sNilRoot;

        void initialize(alloc_t* allocator);

        void release_filename(tname_t* name);
        void release_extension(tname_t* name);
        void release_path(tpath_t* path);
        void release_device(tdevice_t* dev);

        tname_t* get_empty_name() const;

        tname_t* register_name(crunes_t const& namestr);
        bool register_filename(crunes_t const& fullfilename, tname_t*& out_filename, tname_t*& out_extension);
        tname_t* register_dirname(crunes_t const& fulldirname);
        tname_t* register_extension(crunes_t const& extension);
        tdevice_t* register_device(crunes_t const& device);
    };

    //
    // troot_t functions
    //
    void troot_t::initialize(alloc_t* allocator)
    {
        m_allocator = allocator;

        m_filename_table.initialize(m_allocator, 65536);
        m_extension_table.initialize(m_allocator, 8192);
    }

    void troot_t::release_filename(tname_t* name) 
    {
        if (name == sNilName)
            return;

        if (tname_t::release(name))
        {
            m_filename_table.remove(name);
            name = name->release(m_allocator);
        }
    }

    void troot_t::release_extension(tname_t* name) 
    {
        if (name == sNilName)
            return;

        if (tname_t::release(name))
        {
            m_extension_table.remove(name);
            name = name->release(m_allocator);
        }
    }

    void troot_t::release_path(tpath_t* path)
    {
        if (path == sNilPath)
            return;

        if (path->detach(this))
        { 
            tpath_t::destruct(m_allocator, path);
        }
    }

    void troot_t::release_device(tdevice_t* dev) {}

    tname_t* troot_t::get_empty_name() const
    {
        return sNilName;
    }

    bool troot_t::register_filename(crunes_t const& fullfilename, tname_t*& out_filename, tname_t*& out_extension)
    {
        // split filename into name+extension
        crunes_t filename = findLastSelectUntil(fullfilename, ".");
        crunes_t extension = selectAfterExclude(fullfilename, filename);
        out_filename = register_name(filename);
        out_extension = register_extension(extension);
        return false;
    }

    tname_t* troot_t::register_dirname(crunes_t const& fulldirname)
    {
        return register_name(fulldirname);
    }

    tname_t* troot_t::register_name(crunes_t const& namestr) 
    { 
        const u64 hname = calchash(namestr);
        tname_t* name = m_filename_table.find(hname);
        while (name != nullptr && xcore::compare(namestr, name->m_name) != 0)
        {
            name = m_filename_table.next(hname, name);
        }
        if (name == nullptr)
        {
            name = tname_t::construct(m_allocator, hname, namestr);
            m_filename_table.insert(name);
        }
        return name;
    }

    tname_t* troot_t::register_extension(crunes_t const& extension) 
    { 
        const u64 hname = calchash(extension);
        tname_t* name = m_extension_table.find(hname);
        while (name != nullptr && xcore::compare(extension, name->m_name) != 0)
        {
            name = m_extension_table.next(hname, name);
        }
        if (name == nullptr)
        {
            name = tname_t::construct(m_allocator, hname, extension);
            m_extension_table.insert(name);
        }
        return name;
    }

    tdevice_t* troot_t::register_device(crunes_t const& device)
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


    //
    // tpath_t implementations
    // 

    tpath_t* tpath_t::attach() 
    { 
        m_ref++; 
        return this; 
    }

    bool tpath_t::detach(troot_t* root)
    {
        if (m_ref > 0)
        {
            m_ref -= 1;
            if (m_ref == 0)
            {
                for (s32 i = 0; i < m_len; i++)
                {
                    tname_t* dirname = m_path[i];
                    dirname->release(root->m_allocator);
                }
                return true;
            }
        }
        return false;
    }

    tname_t* tpath_t::get_name() const
    {
        if (m_len == 0) 
            return troot_t::sNilName;
        return m_path[m_len - 1];
    }

    tpath_t* tpath_t::prepend(tname_t* folder, alloc_t* allocator)
    {
        tpath_t* path = construct(allocator, folder->m_len + 1);
        copy_array(m_path, 0, m_len, path->m_path, 1);
        path->m_path[0] = folder->incref();
        return path;
    }

    tpath_t* tpath_t::append(tname_t* folder, alloc_t* allocator)
    {
        tpath_t* path = construct(allocator, folder->m_len + 1);
        copy_array(m_path, 0, m_len, path->m_path, 0);
        tname_t* f = 
        path->m_path[m_len - 1] = folder->incref();
        return path;
    }

    tpath_t* tpath_t::construct(alloc_t* allocator, s32 cap)
    {
        ASSERT(cap > 0);
        s32 const allocsize = sizeof(tpath_t) + (((cap + 3) & ~3) - 1);
        tpath_t* path = (tpath_t*)allocator->allocate(allocsize);
        path->m_cap = (((cap + 3) & ~3) - 1);
        path->m_len = 0;
        path->m_ref = 0;
        path->m_path[0] = '\0';
        path->m_path[1] = '\0';
        return path;
    }

    void tpath_t::destruct(alloc_t* allocator, tpath_t* path)
    {
        allocator->deallocate(path);
    }

    void tpath_t::copy_array(tname_t* const* from, u32 from_start, u32 from_len, tname_t** to, u32 to_start)
    {
        tname_t* const* src = from + from_start;
        tname_t* const* end = src + from_len;
        tname_t** dst = to + to_start;
        while (src < end)
            *dst++ = *src++;
    }


    //
    // tdevice_t implementations
    // 
    void tdevice_t::init(troot_t* owner)
    {
        m_name = nullptr;
        m_root = owner;
        m_path = nullptr;
        m_fd   = x_NullFileDevice();
    }

    tdevice_t* tdevice_t::construct(alloc_t* allocator, troot_t* owner)
    {
        // Allocate a tdevice_t
        void*      device_mem = allocator->allocate(sizeof(tdevice_t), sizeof(void*));
        tdevice_t* device     = static_cast<tdevice_t*>(device_mem);
        device->init(owner);
        return device;
    }
    void tdevice_t::destruct(alloc_t* allocator, tdevice_t*& device)
    {
        allocator->deallocate(device);
        device = nullptr;
    }

    class tdirpath_t
    {
    protected:
        tdevice_t* m_device;
        tpath_t* m_path;
        friend class tfilepath_t;

    public:
        tdirpath_t();
        tdirpath_t(tdirpath_t const& other);
        tdirpath_t(tdevice_t* device);
        ~tdirpath_t();

        void clear();
        bool isEmpty() const;

        void makeRelativeTo(const tdirpath_t& dirpath);
        void makeAbsoluteTo(const tdirpath_t& dirpath);

        tname_t* getname() const;

        tdirpath_t root() const;

        tdirpath_t prepend(tname_t* folder);
        tdirpath_t prepend(crunes_t const& folder);
        tdirpath_t append(tname_t* folder);
        tdirpath_t append(crunes_t const& folder);

        void to_string(runes_t& str) const;
    };

    class tfilepath_t
    {
        tdirpath_t m_dirpath;
        tname_t*   m_filename;  // Should always have a filename
        tname_t*   m_extension; // textension_t global("") if ext == ""

    public:
        tfilepath_t();
        tfilepath_t(tname_t* filename, tname_t* extension);
        tfilepath_t(tdirpath_t dirpath, tname_t* filename, tname_t* extension);
        ~tfilepath_t();

        void clear();
        bool isEmpty() const;

        void setFilename(tname_t* filename);
        void setFilename(crunes_t const& filename);
        void setExtension(tname_t* extension);
        void setExtension(crunes_t const& extension);

        tdirpath_t  root() const;
        tdirpath_t  dirpath() const;
        tname_t*    dirname() const;
        tname_t*    filename() const;
        tname_t*    extension() const;

        tfilepath_t prepend(tname_t* folder);
        tfilepath_t prepend(crunes_t const& folder);
        tfilepath_t append(tname_t* folder);
        tfilepath_t append(crunes_t const& folder);

        void to_string(runes_t& str) const;
    };

    tdirpath_t::tdirpath_t() : m_device(troot_t::sNilDevice)
    {
        m_device->attach();
        m_path = troot_t::sNilPath;
    }
    tdirpath_t::tdirpath_t(tdirpath_t const& other)
    {
        m_device = m_device->attach();
        m_path = other.m_path->attach();
    }
    tdirpath_t::tdirpath_t(tdevice_t* device)
    {
        m_device = device->attach();
        m_path = troot_t::sNilPath;
    }
    tdirpath_t::~tdirpath_t()
    {
        troot_t* root = m_device->m_root;
        root->release_device(m_device);
        root->release_path(m_path);
    }

    void tdirpath_t::clear()
    {
        troot_t* root = m_device->m_root;
        root->release_device(m_device);
        root->release_path(m_path);
        m_device  = troot_t::sNilDevice;
        m_path = troot_t::sNilPath;
    }

    bool tdirpath_t::isEmpty() const { return m_device == troot_t::sNilDevice && m_path == troot_t::sNilPath; }

    void tdirpath_t::makeRelativeTo(const tdirpath_t& dirpath) 
    {
        // @TODO
    }
    void tdirpath_t::makeAbsoluteTo(const tdirpath_t& dirpath)
    {
        // @TODO
    }

    tdirpath_t tdirpath_t::root() const { return tdirpath_t(m_device); }

    tname_t*   tdirpath_t::getname() const 
    { 
        troot_t* root = m_device->m_root;
        return m_path->get_name();
    }

    tdirpath_t tdirpath_t::prepend(tname_t* folder)
    {   
        tdirpath_t d;
        troot_t* root = m_device->m_root;
        d.m_device = m_device;
        d.m_path = m_path->prepend(folder, root->m_allocator);
        return d;
    }

    tdirpath_t tdirpath_t::prepend(crunes_t const& folder)
    {   
        troot_t* root = m_device->m_root;
        tname_t* name = root->register_dirname(folder);
        return prepend(name);
    }

    tdirpath_t tdirpath_t::append(tname_t* folder)
    {
        tdirpath_t d;
        troot_t* root = m_device->m_root;
        d.m_device = m_device;
        d.m_path = m_path->append(folder, root->m_allocator);
        return d;
    }

    tdirpath_t tdirpath_t::append(crunes_t const& folder)
    {
        troot_t* root = m_device->m_root;
        tname_t* name = root->register_dirname(folder);
        return append(name);
    }

    void tdirpath_t::to_string(runes_t& str) const
    {
        troot_t* root = m_device->m_root;
        xcore::concatenate(str, m_device->m_path->m_name);       
        for (s32 i = 0; i < m_path->m_len; i++)
        {
            tname_t* pname = m_path->m_path[i];
            xcore::concatenate(str, pname->m_name);
            str += (utf32::rune)'\\';
        }
    }

    tfilepath_t::tfilepath_t() : m_dirpath(), m_filename(troot_t::sNilName), m_extension(troot_t::sNilName) {}
    tfilepath_t::tfilepath_t(tname_t* filename, tname_t* extension) : m_dirpath(), m_filename(filename), m_extension(extension) {}
    tfilepath_t::tfilepath_t(tdirpath_t dirpath, tname_t* filename, tname_t* extension) : m_dirpath(dirpath), m_filename(filename), m_extension(extension) {}

    tfilepath_t::~tfilepath_t()
    {
        troot_t::sNilRoot->release_filename(m_filename);
        troot_t::sNilRoot->release_extension(m_extension);
    }

    void tfilepath_t::clear()
    {
        m_dirpath.clear();
        troot_t* root = m_dirpath.m_device->m_root;
        root->release_filename(m_filename);
        root->release_extension(m_extension);
        m_filename = troot_t::sNilName;
        m_extension = troot_t::sNilName;
    }

    bool tfilepath_t::isEmpty() const { return m_dirpath.isEmpty() && m_filename == troot_t::sNilName && m_extension == troot_t::sNilName; }

    void tfilepath_t::setFilename(tname_t* filename) { filename->incref(); m_filename->release(m_dirpath.m_device->m_root->m_allocator); m_filename = filename; }
    void tfilepath_t::setFilename(crunes_t const& filenamestr)
    {
        troot_t* root = m_dirpath.m_device->m_root;
        tname_t* out_filename = nullptr;
        tname_t* out_extension = nullptr;
        root->register_filename(filenamestr, out_filename, out_extension);
        root->release_filename(m_filename);
        root->release_extension(m_extension);
        m_filename = out_filename->incref();
        m_extension = out_extension->incref();

    }
    void tfilepath_t::setExtension(tname_t* extension) { extension->incref(); m_extension->release(m_dirpath.m_device->m_root->m_allocator); m_extension = extension; }
    void tfilepath_t::setExtension(crunes_t const& extensionstr)
    {
        troot_t* root = m_dirpath.m_device->m_root;
        tname_t* out_extension = root->register_extension(extensionstr);
        root->release_extension(m_extension);
        m_extension = out_extension->incref();
    }

    tdirpath_t tfilepath_t::root() const { return m_dirpath.root(); }
    tdirpath_t tfilepath_t::dirpath() const { return m_dirpath; }
    tname_t*   tfilepath_t::dirname() const { return m_dirpath.getname(); }
    tname_t*   tfilepath_t::filename() const { return m_filename; }
    tname_t*   tfilepath_t::extension() const { return m_extension; }

    tfilepath_t tfilepath_t::prepend(tname_t* folder)
    {
        tfilepath_t f(*this);
        f.m_dirpath.prepend(folder);
        return f;
    }

    tfilepath_t tfilepath_t::prepend(crunes_t const& folder)
    {
        tfilepath_t f(*this);
        f.m_dirpath.prepend(folder);
        return f;
    }

    tfilepath_t tfilepath_t::append(tname_t* folder)
    {
        tfilepath_t f(*this);
        f.m_dirpath.append(folder);
        return f;
    }

    tfilepath_t tfilepath_t::append(crunes_t const& folder)
    {
        tfilepath_t f(*this);
        f.m_dirpath.append(folder);
        return f;
    }

    void tfilepath_t::to_string(runes_t& str) const
    {
        m_dirpath.to_string(str);
        xcore::concatenate(str, m_filename->m_name);
        str += (utf32::rune)'.';
        xcore::concatenate(str, m_extension->m_name);
    }

}; // namespace xcore