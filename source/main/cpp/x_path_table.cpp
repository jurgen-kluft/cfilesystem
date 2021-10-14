#include "xbase/x_allocator.h"
#include "xbase/x_debug.h"
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
    //    - appdir->scan(); // populate dirs and files
    //    - tdirpath_t bins = dir->down("bin") // even if this folder doesn't exist, it will be 'added'
    //    - tfilepath_t coolexe = bins->file("cool.exe");
    //    - tfilepath_t config = bins
    //    - tname_t* datafilename = root->filename("data.txt");
    //    - tfilepath_t datafilepath = bins->file(datafilename);
    //    - stream_t datastream = datafilepath->open();
    //      datastream.close();

    struct tname_t;
    struct tdevice_t;

#define FNV1_64_OFFSET_BASIS ((u64)14695981039346656037u)
#define FNV_64_PRIME ((u64)1099511628211u)

    static u64 generate_hash(crunes_t const& cstr)
    {
        // Use FNV-1a for now
        u64          hash      = FNV1_64_OFFSET_BASIS;
        u64 const    fnv_prime = FNV_64_PRIME;
        const uchar* str       = (const uchar*)cstr.m_runes.m_ascii.m_str;
        const uchar* end       = (const uchar*)cstr.m_runes.m_ascii.m_end;
        while (str < end)
        {
            uchar32 c = str[0];
            hash ^= c;
            hash *= fnv_prime;
            str++;
        }
        return hash;
    }

    static u64 mix_hash(u64 hash, u64 mix)
    {
        u64 const    fnv_prime = FNV_64_PRIME;
        xbyte const* data      = (xbyte const*)&mix;
        for (s32 i = 0; i < 8; i++)
        {
            hash ^= data[i];
            hash *= fnv_prime;
        }
        return hash;
    }

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
        s32         m_refs;
        s32         m_len;
        u64         m_hash;
        utf32::rune m_name[1];

        tname_t(s32 strlen) : m_refs(0), m_len(strlen), m_hash(0)
        {
            m_name[0]      = utf32::TERMINATOR;
            m_name[strlen] = utf32::TERMINATOR;
        }

        bool isEmpty() const { return m_hash == 0; }

        static s32 compare(const tname_t* name, const tname_t* other)
        {
            if (name->m_hash == other->m_hash)
            {
                crunes_t name(name->m_name, name->m_len);
                crunes_t othername(other->m_name, other->m_len);
                return xcore::compare(name, othername);
            }
            if (name->m_hash < other->m_hash)
                return -1;
            return 1;
        }

        s32 compare(const tname_t* other) const
        {
            if (m_hash == other->m_hash)
            {
                crunes_t name(m_name, m_len);
                crunes_t othername(other->m_name, other->m_len);
                return xcore::compare(name, othername);
            }
            if (m_hash < other->m_hash)
                return -1;
            return 1;
        }

        s32 compare(crunes_t const& name) const { return xcore::compare(crunes_t(m_name, m_len), name); }

        bool operator==(const tname_t& other) const { return compare(&other) == 0; }
        bool operator!=(const tname_t& other) const { return compare(&other) != 0; }

        void        incref() { m_refs++; }
        static bool release(tname_t* name)
        {
            if (name->m_refs > 0)
            {
                name->m_refs -= 1;
                return name->m_refs == 0;
            }
            return false;
        }

        static tname_t* construct(alloc_t* allocator, u64 hname, crunes_t const& name)
        {
            u32 const strlen = sizeof(utf32::rune) * name.size();
            u32 const strcap = strlen + 1;

            void*    folder_mem = allocator->allocate(sizeof(tname_t) + (sizeof(utf32::rune) * strcap), sizeof(void*));
            tname_t* pfolder    = new (folder_mem) tname_t(strcap);

            pfolder->m_hash   = hname;
            pfolder->m_refs   = 0;
            runes_t dststr(pfolder->m_name, pfolder->m_name, pfolder->m_name + strlen);
            copy(name, dststr);

            return pfolder;
        }

        template <class T> static void destruct(alloc_t* allocator, T*& name)
        {
            allocator->deallocate(name);
            name = nullptr;
        }

        XCORE_CLASS_PLACEMENT_NEW_DELETE

        tname_t* m_next;
    };

    struct tdevice_t
    {
        troot_t*      m_root; // If alias, tdevice_t ("work") | nullptr
        filedevice_t* m_fd;   // nullptr                      | xfiledevice("e")
        tname_t*      m_name;
        tfolder_t*    m_path; // "xfilesystem\\"              | "dev.go\\src\\github.com\\jurgen-kluft\\"

        void              init(troot_t* owner);
        static tdevice_t* construct(alloc_t* allocator, troot_t* owner);
        static void       destruct(alloc_t* allocator, tdevice_t*& device);
    };

    template <class T> class htable_t
    {
    public:
        alloc_t* m_allocator;
        s64      m_size;
        T**      m_data;

        inline s32 to_index(u64 hash) const;
        void       init(alloc_t* allocator, s32 size_as_bits = 8);
        void       assign(T* data);
        T*         find(u64 hash) const;
        T*         next(u64 hash, T*& prev) const;
        bool       remove(T* item);
    };

    struct tfolder_t
    {
        tfolder_t* down(tdevice_t* device, tname_t* foldername);

        tfolder_t*  m_parent;
        tname_t*    m_name;
        s32         m_ref_count;
        s32         m_folders_count;
        tfolder_t** m_folders_array;
    };

    // API prototype
    // root table
    class troot_t
    {
    public:
        troot_t(alloc_t* allocator, runes_alloc_t* string_allocator) : m_allocator(allocator), m_stralloc(string_allocator) {}

        alloc_t*            m_allocator;
        runes_alloc_t*      m_stralloc;
        s32                 m_num_devices;
        s32                 m_max_devices;
        tdevice_t           m_tdevice[64];
        htable_t<tfolder_t> m_folder_table;
        htable_t<tname_t>   m_filename_table;
        htable_t<tname_t>   m_extension_table;

        static tdevice_t* sNilDevice;
        static tfolder_t* sNilFolder;
        static tname_t*   sNilFilename;
        static tname_t*   sNilExtension;
        static troot_t*   sNilRoot;

        void initialize(alloc_t* allocator)
        {
            m_folder_table.init(allocator, 9);
            m_filename_table.init(allocator, 10);
            m_extension_table.init(allocator, 6);
        }

        void release_filename(tname_t* name) 
        {
            if (tname_t::release(name))
            {
                m_filename_table.remove(name);
                tname_t::destruct(m_allocator, name);
            }
        }

        void release_extension(tname_t* name) 
        {
            if (tname_t::release(name))
            {
                m_extension_table.remove(name);
                tname_t::destruct(m_allocator, name);
            }
        }

        void release_device(tdevice_t* dev) {}
        void release_folder(tfolder_t* folder) {}

        bool register_filename(crunes_t const& fullfilename, tname_t*& out_filename, tname_t*& out_extension)
        {
            // split filename into name+extension
            crunes_t filename;
            crunes_t extension;
            out_filename = register_filename_without_extension(filename);
            out_extension = register_extension(extension);
            return false;
        }

        tname_t* register_filename_without_extension(crunes_t const& filename) 
        { 
            const u64 hname = generate_hash(filename);
            tname_t* name = m_filename_table.find(hname);
            while (name != nullptr && xcore::compare(filename, name->m_name) != 0)
            {
                name = m_filename_table.next(hname, name);
            }
            if (name == nullptr)
            {
                name = tname_t::construct(m_allocator, hname, filename);
                m_filename_table.assign(name);
            }
            return name;
        }

        tname_t* register_extension(crunes_t const& extension) 
        { 
            const u64 hname = generate_hash(extension);
            tname_t* name = m_extension_table.find(hname);
            while (name != nullptr && xcore::compare(extension, name->m_name) != 0)
            {
                name = m_extension_table.next(hname, name);
            }
            if (name == nullptr)
            {
                name = tname_t::construct(m_allocator, hname, extension);
                m_extension_table.assign(name);
            }
            return name;
        }

        tdevice_t* register_device(crunes_t const& device)
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
    };

    //
    // troot_t functions
    //
    void tdevice_t::init(troot_t* owner)
    {
        m_name = nullptr;
        m_root = owner;
        m_path = troot_t::sNilFolder;
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
        tdevice_t* m_device;  // tdevice_t global("") if device == ""
        tfolder_t* m_path[2]; // tfolder_t global("") if path == ""
        friend class tfilepath_t;

    public:
        tdirpath_t();
        tdirpath_t(tdevice_t* device, tfolder_t* proot, tfolder_t* pend);
        ~tdirpath_t();

        static const s32 cHead = 0;
        static const s32 cTail = 1;

        void clear();
        bool isEmpty() const;

        void makeRelativeTo(const tdirpath_t& dirpath);
        void makeAbsoluteTo(const tdirpath_t& dirpath);

        tname_t* getname() const;

        tdirpath_t root() const;
        tdirpath_t full();
        tdirpath_t up(s32 folder);
        tdirpath_t down(s32 folder, tname_t* dirname);

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
        void setExtension(tname_t* extension);

        tdirpath_t  root() const;
        tdirpath_t  dirpath() const;
        tname_t*    dirname() const;
        tname_t*    filename() const;
        tname_t*    extension() const;
        tfilepath_t up(s32 folder);
        tfilepath_t down(tname_t* dirname, s32 folder);

        void to_string(runes_t& str) const;
    };

    tdirpath_t::tdirpath_t() : m_device(troot_t::sNilDevice)
    {
        m_path[0] = (troot_t::sNilFolder);
        m_path[1] = (troot_t::sNilFolder);
    }
    tdirpath_t::tdirpath_t(tdevice_t* device, tfolder_t* proot, tfolder_t* pend) : m_device(device)
    {
        m_path[0] = proot;
        m_path[1] = pend;
    }
    tdirpath_t::~tdirpath_t()
    {
        troot_t* root = m_device->m_root;
        root->release(m_device);
        root->release(m_path[0]);
        root->release(m_path[1]);
    }

    void tdirpath_t::clear()
    {
        troot_t* root = m_device->m_root;
        root->release(m_device);
        root->release(m_path[0]);
        root->release(m_path[1]);
        m_device  = troot_t::sNilDevice;
        m_path[0] = troot_t::sNilFolder;
        m_path[1] = troot_t::sNilFolder;
    }

    bool tdirpath_t::isEmpty() const { return m_device == troot_t::sNilDevice && m_path[0] == troot_t::sNilFolder && m_path[1] == troot_t::sNilFolder; }

    void tdirpath_t::makeRelativeTo(const tdirpath_t& dirpath) {}
    void tdirpath_t::makeAbsoluteTo(const tdirpath_t& dirpath) {}

    tdirpath_t tdirpath_t::root() const { return tdirpath_t(m_device, troot_t::sNilFolder, troot_t::sNilFolder); }
    tdirpath_t tdirpath_t::full()
    {
        tfolder_t* head;
        do {
            head = m_path[0];
            m_path[0] = m_path[0]->m_parent;
        } while (m_path[0] != head);
    }

    tname_t*   tdirpath_t::getname() const { return m_path[1]->m_name; }

    tdirpath_t tdirpath_t::up(s32 folder)
    {
        tdirpath_t d;
        d.m_device  = m_device;
        d.m_path[0] = m_path[0];
        d.m_path[1] = m_path[1];
        if (folder == tdirpath_t::cHead)
        {
            d.m_path[0] = m_path[0]->m_parent;
        }
        else
        {
            if (d.m_path[1] != d.m_path[0])
            {
                d.m_path[1] = m_path[1]->m_parent;
            }
        }
        return d;
    }

    tdirpath_t tdirpath_t::down(s32 folder, tname_t* dirname) 
    {
        tdirpath_t d;
        d.m_device  = m_device;
        d.m_path[1] = m_path[1];
        if (folder == tdirpath_t::cHead)
        {   
            tfolder_t* t = m_path[0];
            tfolder_t* f = m_path[1];
            while (f != t && f->m_parent != t)
            {
                f = f->m_parent;
            }
            m_path[0] = f;
        }
        else
        {
            d.m_path[0] = m_path[0];
            if (d.m_path[1] != d.m_path[0])
            {
                d.m_path[1] = m_path[1]->down(m_device, dirname);
            }
        }
        return d;
    }

    tfilepath_t::tfilepath_t() : m_dirpath(), m_filename(troot_t::sNilFilename), m_extension(troot_t::sNilExtension) {}
    tfilepath_t::tfilepath_t(tname_t* filename, tname_t* extension) : m_dirpath(), m_filename(filename), m_extension(extension) {}
    tfilepath_t::tfilepath_t(tdirpath_t dirpath, tname_t* filename, tname_t* extension) : m_dirpath(dirpath), m_filename(filename), m_extension(extension) {}

    tfilepath_t::~tfilepath_t()
    {
        troot_t::sNilRoot->release(m_filename);
        troot_t::sNilRoot->release(m_extension);
    }

    void tfilepath_t::clear()
    {
        m_dirpath.clear();
        troot_t* root = troot_t::sNilRoot;
        root->release(m_filename);
        root->release(m_extension);
    }

    bool tfilepath_t::isEmpty() const { return m_dirpath.isEmpty() && m_filename == troot_t::sNilFilename && m_extension == troot_t::sNilExtension; }

    void tfilepath_t::setFilename(tname_t* filename) { m_filename = filename; }
    void tfilepath_t::setExtension(tname_t* extension) { m_extension = extension; }

    tdirpath_t tfilepath_t::root() const { return m_dirpath.root(); }
    tdirpath_t tfilepath_t::dirpath() const { return m_dirpath; }
    tname_t*   tfilepath_t::dirname() const { return m_dirpath.getname(); }
    tname_t*   tfilepath_t::filename() const { return m_filename; }
    tname_t*   tfilepath_t::extension() const { return m_extension; }

    tfilepath_t tfilepath_t::up(s32 folder = tdirpath_t::cTail) { m_dirpath = m_dirpath.up(folder); }
    tfilepath_t tfilepath_t::down(tname_t* dirname, s32 folder = tdirpath_t::cTail) { m_dirpath = m_dirpath.down(folder, dirname); }

    template <class T> inline s32 htable_t<T>::to_index(u64 hash) const
    {
        // Take N bits from the hash to come up with an index
        u64 const mask = (1 << m_size) - 1;
        return (s32)((hash & (mask << 8)) >> 8);
    }

    template <class T> void htable_t<T>::init(alloc_t* allocator, s32 size_as_bits = 8)
    {
        m_allocator = allocator;
        m_size      = size_as_bits;
        m_data      = static_cast<T**>(allocator->allocate(sizeof(T*) * m_size, sizeof(void*)));
    }

    template <class T> void htable_t<T>::assign(T* item)
    {
        s32 index     = to_index(item->m_hash);
        item->m_next  = m_data[index];
        m_data[index] = item;
    }

    template <class T> T* htable_t<T>::find(u64 hash) const
    {
        s32 index = to_index(hash);
        if (m_data[index] == nullptr)
            return nullptr;

        T* ptr = m_data[index];
        while (ptr != nullptr)
        {
            if (ptr->m_data->m_hash == hash)
            {
                return ptr;
            }
            ptr = ptr->m_next;
        }
        return nullptr;
    }

    template <class T> T* htable_t<T>::next(u64 hash, T*& prev) const
    {
        if (prev == nullptr)
            return nullptr;

        T* ptr = prev->m_next;
        while (ptr != nullptr)
        {
            if (ptr->m_data->m_hash == hash)
            {
                return ptr;
            }
            prev = ptr;
            ptr  = ptr->m_next;
        }
        prev = nullptr;
        return nullptr;
    }

    template <class T> bool htable_t<T>::remove(T* item)
    {
        s32 index = to_index(item->m_hash);
        if (m_data[index] != nullptr)
        {
            T* prev = nullptr;
            T* iter = m_data[index];
            while (iter != item)
            {
                prev = iter;
                iter = iter->m_next;
            }
            if (iter == item)
            {
                if (prev == nullptr)
                {
                    m_data[index] = item->m_next;
                }
                else
                {
                    prev->m_next = iter->m_next;
                }
                return true;
            }
        }
        return false;
    }

}; // namespace xcore