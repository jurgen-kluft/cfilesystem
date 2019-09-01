#include "xbase/x_target.h"
#include "xbase/x_allocator.h"
#include "xbase/x_debug.h"
#include "xbase/x_runes.h"

#include "xfilesystem/x_filepath.h"
#include "xfilesystem/x_dirpath.h"
#include "xfilesystem/private/x_filesystem.h"

namespace xcore
{
    /*
        What are the benefits of using a table like below to manage filepaths and dirpaths?
        - Sharing of strings
        - Easy manipulation of dirpath, can easily go to parent or child directory, likely without doing any allocations
        - You can prime the table which then results in no allocations when you are using existing filepaths and dirpaths
        - Combining dirpath with filepath becomes very easy
        -
    */
    class troot_t;
    class tpath_t;

    struct tdevice_t
    {
        u64          m_hash;
        utf32::prune m_name;
        troot_t*     m_root; // If alias, tdevice_t ("work") | nullptr
        tpath_t*     m_path; // "xfilesystem\\"              | "dev.go\\src\\github.com\\jurgen-kluft\\"
        xfiledevice* m_fd;   // nullptr                      | xfiledevice("e")

        void              init();
        bool              isEmpty() const { return m_name == nullptr; }
        static tdevice_t* construct(xalloc* allocator);
        static void       destruct(xalloc* allocator, tdevice_t*& device);
    };

    struct tname_t
    {
        s32          m_refs;
        s32          m_len;
        u64          m_hash;
        utf32::prune m_name;

        void init()
        {
            m_refs = 0;
            m_len  = 0;
            m_hash = 0;
            m_name = (utf32::prune) "\0\0\0\0";
        }

        bool isEmpty() const { return m_hash == 0; }

        s32 compare(const tname_t& other) const
        {
            if (m_hash == other.m_hash)
                return utf::compare(m_name, other.m_name);
            if (m_hash < other.m_hash)
                return -1;
            return 1;
        }

        s32 compare(utf32::pcrune path) const { return utf::compare(m_name, other.m_name); }

        bool operator==(const tname_t& other) const { return compare(other) == 0; }
        bool operator!=(const tname_t& other) const { return compare(other) != 0; }

        void        reference() { m_refs++; }
        static bool release(xalloc* allocator, tname_t* name)
        {
            if (--name->m_refs == 0)
            {
                destruct(allocator, name);
                return true;
            }
            return false;
        }
        static tname_t* construct(xalloc* allocator)
        {
            // Allocate a tname_t
            void* name_mem = allocator->allocate(sizeof(tname_t, sizeof(void*)); 
            tname_t* name = static_cast<tname_t>(name_mem); 
            name->init(); 
            return name;
        }
        static void destruct(xalloc* allocator, tname_t*& name)
        {
            allocator->deallocate(name);
            name = nullptr;
        }
    };

    typedef tname_t tfolder_t;
    typedef tname_t tfilename_t;
    typedef tname_t textension_t;

    // Iterate over 'path' and return a folder in 'out_folder'
    // When no more folder return 0, otherwise 1.
    static s32 path_iterator(crunes& path, crunes& out_folder) { return 0; }

    struct tpath_t
    {
        s32        m_refs;
        s32        m_count;
        u64        m_hash;
        tfolder_t* m_folders[1];
        void       init()
        {
            m_refs  = 0;
            m_count = 0;
            m_hash  = 0;
        }
        void init(s32 folder_count)
        {
            m_refs  = 1;
            m_count = folder_count;
            m_hash  = 0;
        }
        bool isEmpty() const { return m_folders == nullptr; }

        tfolder_t*  operator[](s32 index) const { return m_folders[index]; }
        tfolder_t*& operator[](s32 index) { return m_folders[index]; }

        s32 compare(const tpath_t& other) const
        {
            if (m_hash == other.m_hash && m_count == other.m_count)
            {
                for (s32 i = 0; i < m_count; ++i)
                {
                    s32 const c = m_folders[i]->compare(other.m_folders[i]);
                    if (c != 0)
                        return c;
                }
            }
            if (m_hash < other.m_hash)
                return -1;
            return 1;
        }

        s32 compare(utf32::pcrune path) const
        {
            // go over path -> folder and compare them to our array of folders
            return 0;
        }
        s32 compare(ascii::pcrune path) const
        {
            // go over path -> folder and compare them to our array of folders
            return 0;
        }

        s32 count(utf32::pcrune path) const
        {
            // go over path and count the folders
            return 0;
        }
        s32 count(ascii::pcrune path) const
        {
            // go over path and count the folders
            return 0;
        }

        bool operator==(const tname_t& other) const { return compare(other) == 0; }
        bool operator!=(const tname_t& other) const { return compare(other) != 0; }

        void reference() { m_refs++; }

        static bool release(xalloc* allocator, tpath_t*& path)
        {
            path->m_refs -= 1;
            if (path->m_refs == 0)
            {
                destruct(allocator, path);
                return true;
            }
            return false;
        }
        static tpath_t* construct(xalloc* allocator, s32 folder_count)
        {
            void*    path_mem = allocator->allocate(sizeof(tpath_t) + folder_count * sizeof(void*), sizeof(void*));
            tpath_t* path     = static_cast<tpath_t>(path_mem);
            path->init(folder_count);
            return path;
        }
        static void destruct(xalloc* allocator, tpath_t*& path)
        {
            for (s32 i = 0; i < m_count; ++i)
                path->m_folders[i]->release();
            allocator->deallocate(path);
            path = nullptr;
        }
    };

    // xroot table
    class troot_t
    {
    public:
        // API proto
        troot_t(tdevice_t* device, xalloc* allocator, utf32::alloc string_allocator) : m_tdevice(device), m_allocator(allocator), m_stralloc(string_allocator) {}

        tdevice_t*   m_tdevice;
        xalloc*      m_allocator;
        utf32::alloc m_stralloc;

        static tdevice_t*    sNilDevice;
        static tfolder_t*    sNilFolder;
        static tpath_t*      sNilPath;
        static tfilename_t*  sNilFilename;
        static textension_t* sNilExtension;

        template <class T> class hentry_t
        {
        public:
            T*           m_data;
            hentry_t<T>* m_next;
            void         init(T* data, hentry_t<T>* next)
            {
                m_data = data;
                m_next = next;
            }
        };

        template <class T> class htable_t
        {
        public:
            xalloc*       m_allocator;
            s64           m_size;
            hentry_t<T>** m_data;

            inline s32 to_index(u64 hash) const
            {
                // Take N bits from the hash to come up with an index
                u64 const mask = (1 << m_size) - 1;
                return (hash & (mask << 8)) >> 8;
            }

            void init(xalloc* allocator, s32 size_as_bits = 8)
            {
                m_allocator = allocator;
                m_size      = size_as_bits;
                m_data      = static_cast<hentry_t<T>**> allocator->allocate(sizeof(hentry_t<T>*) * m_size, sizeof(void*));
            }

            void assign(u64 hash, T* data)
            {
                // NOTE: Make sure this hash/data hasn't been added before!
                s32          index = to_index(hash);
                hentry_t<T>* ptr   = static_cast<hentry_t<T>*>(m_allocator->allocate(sizeof(hentry_t<T>*), sizeof(void*)));
                ptr->init(data, m_data[index]);
                m_data[index] = ptr;
            }

            template <typename S> T* find(u64 hash, S name) const
            {
                s32 index = to_index(hash);
                if (m_data[index] == nullptr)
                    return nullptr;

                hentry_t<T>* ptr = m_data[index];
                while (ptr != nullptr)
                {
                    if (ptr->m_hash == hash)
                    {
                        if (ptr->m_data->compare(name) == 0)
                        {
                            return ptr->m_data;
                        }
                    }
                    ptr = ptr->m_next;
                }
                return nullptr;
            }

            template <typename S> T* remove(u64 hash, S name)
            {
                s32 index = to_index(hash);
                if (m_data[index] == nullptr)
                    return nullptr;

                hentry_t<T>* prv = nullptr;
                hentry_t<T>* ptr = m_data[index];
                while (ptr != nullptr)
                {
                    if (ptr->m_hash == hash)
                    {
                        if (ptr->m_data->compare(name) == 0)
                        {
                            if (prv == nullptr)
                            {
                                m_data[index] = nullptr;
                            }
                            else
                            {
                                prv->m_next = ptr->m_next;
                            }
                            T* data = ptr->m_data;
                        }
                    }
                    prv = ptr;
                    ptr = ptr->m_next;
                }
                return nullptr;
            }
        };

        htable_t<tfolder_t>    m_folder_table;
        htable_t<tpath_t>      m_path_table;
        htable_t<tfilename_t>  m_filename_table;
        htable_t<textension_t> m_extension_table;

        void initialize(xalloc* allocator)
        {
            m_folder_table.init(allocator, 9);
            m_path_table.init(allocator, 8);
            m_filename_table.init(allocator, 10);
            m_extension_table.init(allocator, 6);
        }

        // ascii
        void register_path(ascii::crunes const& fullpath, tdevice_t*& out_device, tpath_t*& out_path)
        {
            // Split the path into parts:
            // - device
            // - folders
            // - filename
            // - extension
            // We do not need to split the path, we only need to find an existing tpath_t*
            out_device = m_tdevice;

            u64 const fphash = generate_hash(fullpath);
            tpath_t*  ppath  = m_path_table.find<ascii::pcrune>(fphash, fullpath.m_str);
            if (ppath == nullptr)
            {
                // Construct a new tpath_t
                s32 num_folders = tpath_t::count(fullpath);
                ppath           = tpath_t::construct(m_allocator, num_folders);
                ascii::crunes pathcstr(fullpath);
                ascii::crunes foldercstr;
                s32           folderindex = 0;
                while (path_iterator(pathcstr, foldercstr) == 1)
                {
                    tfolder_t* folder            = register_folder(folder_cstr);
                    ppath->m_folder[folderindex] = folder;
                    folderindex++;
                }
                m_path_table.assign(fphash, ppath);
            }
            out_path = ppath;
        }

        tfolder_t* register_folder(ascii::crunes const& folder_name)
        {
            u64 const  hfolder = generate_hash(folder_name);
            tfolder_t* pfolder = m_folder_table.find<ascii::pcrune>(hfolder, folder_name.m_str);
            if (pfolder == nullptr)
            {
                u32 const    strlen = sizeof(utf32::rune) * folder_name.size();
                u32 const    strcap = strlen + 1;
                utf32::prune strptr = m_stralloc->allocate(strlen, strcap);
                utf32::runes str(strptr, strptr, strptr + strlen);
                utf::copy(folder_name, str);
                pfolder         = tfolder_t::construct(m_allocator);
                pfolder->m_len  = strlen;
                pfolder->m_hash = hfolder;
                pfolder->m_name = strptr;
                pfolder->m_refs = 0;
            }
            return folder;
        }
        tfilename_t*  register_filename(ascii::crunes const& file_name);
        textension_t* register_extension(ascii::crunes const& extension);

        // utf32
        void          register_path(utf32::crunes const& fullpath, tdevice_t*& device, tpath_t*& path);
        tfolder_t*    register_folder(utf32::crunes const& folder_name);
        tfilename_t*  register_filename(utf32::crunes const& file_name);
        textension_t* register_extension(utf32::crunes const& extension);

        // path construction
        tpath_t* construct_path(s32 depth);
        void     register_folder(tpath_t* path, tfolder_t* folder);
        tpath_t* register_path(tpath_t* path);

        void release(tdevice_t*);
        void release(tpath_t*);
        void release(tfolder_t*);
        void release(tfilename_t*);
        void release(textension_t*);

    private:
        u64 generate_hash(ascii::crunes str) const
        {
            // TODO: A good hash algorithm
            return 0;
        }
        u64 generate_hash(utf32::crunes str) const;
        {
            // TODO: A good hash algorithm
            return 0;
        }
    };

    //
    // troot_t functions
    //
    void troot_t::init()
    {
        m_hash = 0;
        m_name = nullptr;
        m_root = nullptr;
        m_path = troot_t::sNilPath;
        m_fd   = x_NullFileDevice();
    }

    static tdevice_t* troot_t::construct(xalloc* allocator)
    {
        // Allocate a tdevice_t
        void* device_mem = allocator->allocate(sizeof(tdevice_t, sizeof(void*)); 
        tdevice_t* device = static_cast<tname_t>(device_mem); device->init(); 
        return device;
    }
    static void troot_t::destruct(xalloc* allocator, tdevice_t*& device)
    {
        allocator->deallocate(device);
        device = nullptr;
    }

    // These 2 classes will be like this and as such will always be the same
    // size. If we also incorporate copy-on-write so we can easily copy and modify.
    class tdirpath_t
    {
    public:
        troot_t*   m_root;   // my owner
        tdevice_t* m_device; // tdevice_t global("") if device == ""
        tpath_t*   m_path;   // tpath_t global("") if path == ""

        bool isEmpty() const { return m_device->isEmpty() && m_path->isEmpty(); }
        bool isRooted() const { return !m_device->isEmpty(); }

        void makeRelative() { m_device = troot_t::sNilDevice; }

        void getRoot(xdirpath& dirpath) const { dirpath = xdirpath(m_root, m_device); }
        void getDirname(xdirpath& dirpath) const { dirpath = xdirpath(m_root, m_path.getName()); }

        void up() {}
        void down(xdirpath const& dirpath) {}
    };

    class tfilepath_t
    {
    public:
        tdirpath_t    m_dirpath;
        tfilename_t*  m_filename;  // Should always have a filename
        textension_t* m_extension; // textension_t global("") if ext == ""

        void clear()
        {
            troot_t* root = m_dirpath.m_root;
            root->release(m_dirpath.m_device);
            root->release(m_dirpath.m_path);
            root->release(m_filename);
            root->release(m_extension);
        }

        bool isEmpty() const { return m_dirpath.isEmpty() && m_filename->isEmpty(); }
        bool isRooted() const { return m_dirpath.isRooted(); }

        void makeRelative() { m_dirpath.makeRelative(); }
        void makeRelativeTo(const xdirpath& dirpath) { m_dirpath.makeRelativeTo(dirpath); }
        void makeAbsoluteTo(const xdirpath& dirpath) { m_dirpath.makeAbsoluteTo(dirpath); }

        void setDirpath(xdirpath const& dirpath) { m_dirpath = dirpath; }
        void setFilename(xfilepath const& filepath)
        {
            m_filename  = filepath.m_filename;
            m_extension = filepath.m_extension;
        }
        void setFilenameWithoutExtension(xfilepath const& filepath) { m_filename = filepath.m_filename; }
        void setExtension(xfilepath const& filepath) { m_extension = filepath.m_extension; }

        xdirpath  getRoot() const { return m_dirpath.getRoot(); }
        xdirpath  getDirpath() const { return m_dirpath; }
        xdirpath  getDirname() const { return m_dirpath.getDirName(); }
        xfilepath getFilename() const { return xfilepath(m_dirpath.m_root, m_filename, m_extension); }
        xfilepath getFilenameWithoutExtension() const { return xfilepath(m_dirpath.m_root, m_filename, troot_t::sNilExtension); }
        xfilepath getExtension() const { return xfilepath(m_dirpath.m_root, troot_t::sNilFilename, m_extension); }

        void up() { m_dirpath.up(); }
        void down(xdirpath const& dirpath) { m_dirpath.down(dirpath); }
    };

}; // namespace xcore