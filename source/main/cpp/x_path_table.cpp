#include "xbase/x_allocator.h"
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
        u64         m_hash;
        s32         m_refs;
        s32         m_len;
        utf32::rune m_name[2];

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
            u32 const strlen = name.size();
            u32 const strcap = strlen - 1;

            void*    name_mem = allocator->allocate(sizeof(tname_t) + (sizeof(utf32::rune) * strcap), sizeof(void*));
            tname_t* pname    = new (name_mem) tname_t(strcap);

            pname->m_hash   = hname;
            pname->m_refs   = 0;
            runes_t dststr(pname->m_name, pname->m_name, pname->m_name + strlen);
            copy(name, dststr);

            return pname;
        }

        template <class T> static void destruct(alloc_t* allocator, T*& name)
        {
            allocator->deallocate(name);
            name = nullptr;
        }

        XCORE_CLASS_PLACEMENT_NEW_DELETE
    };

    class tdirpath_t;

    struct tdevice_t
    {
        troot_t*      m_root; // If alias, tdevice_t ("work") | nullptr
        filedevice_t* m_fd;   // nullptr                      | xfiledevice("e")
        tname_t*      m_name;
        tdirpath_t*   m_path; // "xfilesystem\\"              | "dev.go\\src\\github.com\\jurgen-kluft\\"

        void              init(troot_t* owner);

        static tdevice_t* construct(alloc_t* allocator, troot_t* owner);
        static void       destruct(alloc_t* allocator, tdevice_t*& device);
    };

    struct tname_table_t
    {
        s32                 m_len;
        s32                 m_cap;
        s32                 m_freelist;
        s32*                m_remap;
        tname_t**           m_table;

        void initialize(alloc_t* allocator, s32 cap)
        {
            m_len = 0;
            m_cap = cap;
            m_table = (tname_t**)allocator->allocate(sizeof(tname_t*) * cap);
        }

        s32 find(u64 hash);
        s32 next(u64 hash, s32& prev);
        tname_t* get(s32 index) const;

        void assign(tname_t* name);
        void remove(tname_t* name);
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
        static troot_t*   sNilRoot;

        void initialize(alloc_t* allocator)
        {
            m_allocator = allocator;

            m_filename_table.initialize(m_allocator, 4096);
            m_extension_table.initialize(m_allocator, 512);
        }

        void release_filename(tname_t* name) 
        {
            if (name == sNilName)
                return;

            if (tname_t::release(name))
            {
                m_filename_table.remove(name);
                tname_t::destruct(m_allocator, name);
            }
        }

        void release_extension(tname_t* name) 
        {
            if (name == sNilName)
                return;

            if (tname_t::release(name))
            {
                m_extension_table.remove(name);
                tname_t::destruct(m_allocator, name);
            }
        }

        void release_array(s32* arr)
        {
            m_allocator->deallocate(arr);
        }

        void release_device(tdevice_t* dev) {}

        tname_t* get_name(s32 index_of_name)
        {
            return sNilName;
        }

        tname_t* get_empty_name()
        {
            return sNilName;
        }

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
            const u64 hname = calchash(filename);
            s32 iname = m_filename_table.find(hname);
            tname_t* name = m_extension_table.get(iname);
            while (iname != -1 && xcore::compare(filename, name->m_name) != 0)
            {
                iname = m_filename_table.next(hname, iname);
                name = m_extension_table.get(iname);
            }
            if (iname == -1)
            {
                name = tname_t::construct(m_allocator, hname, filename);
                m_filename_table.assign(name);
            }
            return name;
        }

        tname_t* register_extension(crunes_t const& extension) 
        { 
            const u64 hname = calchash(extension);
            s32 iname = m_extension_table.find(hname);
            tname_t* name = m_extension_table.get(iname);
            while (iname != -1 && xcore::compare(extension, name->m_name) != 0)
            {
                iname = m_extension_table.next(hname, iname);
                name = m_extension_table.get(iname);
            }
            if (iname == -1)
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
        s32*       m_path;
        s16        m_len;
        s16        m_cap;
        s32        m_dummy;
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
        m_path = nullptr;
        m_len = 0;
        m_cap = 0;
    }
    tdirpath_t::tdirpath_t() : m_device(troot_t::sNilDevice)
    {
        m_path = nullptr;
    }
    tdirpath_t::tdirpath_t(tdirpath_t const& other) : m_device(other.m_device)
    {
        // @TODO: copy the other stuff
    }
    tdirpath_t::~tdirpath_t()
    {
        troot_t* root = m_device->m_root;
        root->release_device(m_device);
        root->release_array(m_path);
    }

    void tdirpath_t::clear()
    {
        troot_t* root = m_device->m_root;
        root->release_device(m_device);
        root->release_array(m_path);
        m_device  = troot_t::sNilDevice;
        m_path = nullptr;
        m_len = 0;
        m_cap = 0;
    }

    bool tdirpath_t::isEmpty() const { return m_device == troot_t::sNilDevice && m_len == 0; }

    void tdirpath_t::makeRelativeTo(const tdirpath_t& dirpath) {}
    void tdirpath_t::makeAbsoluteTo(const tdirpath_t& dirpath) {}

    tdirpath_t tdirpath_t::root() const { return tdirpath_t(m_device); }

    tname_t*   tdirpath_t::getname() const 
    { 
        troot_t* root = m_device->m_root;
        if (m_len > 0)
            return root->get_name(m_path[0]);
        return root->get_empty_name();
    }

    tdirpath_t tdirpath_t::prepend(tname_t* folder)
    {   
        tdirpath_t d;
        return d;
    }

    tdirpath_t tdirpath_t::append(tname_t* folder)
    {
        tdirpath_t d;
        return d;
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
        troot_t* root = troot_t::sNilRoot;
        root->release_filename(m_filename);
        root->release_extension(m_extension);
    }

    bool tfilepath_t::isEmpty() const { return m_dirpath.isEmpty() && m_filename == troot_t::sNilName && m_extension == troot_t::sNilName; }

    void tfilepath_t::setFilename(tname_t* filename) { m_filename = filename; }
    void tfilepath_t::setExtension(tname_t* extension) { m_extension = extension; }

    tdirpath_t tfilepath_t::root() const { return m_dirpath.root(); }
    tdirpath_t tfilepath_t::dirpath() const { return m_dirpath; }
    tname_t*   tfilepath_t::dirname() const { return m_dirpath.getname(); }
    tname_t*   tfilepath_t::filename() const { return m_filename; }
    tname_t*   tfilepath_t::extension() const { return m_extension; }

    tfilepath_t tfilepath_t::prepend(tname_t* folder)
    {
        tfilepath_t f;
        return f;
    }

    tfilepath_t tfilepath_t::append(tname_t* folder)
    {
        tfilepath_t f;
        return f;
    }

}; // namespace xcore