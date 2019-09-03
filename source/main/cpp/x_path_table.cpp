#include "xbase/x_target.h"
#include "xbase/x_allocator.h"
#include "xbase/x_debug.h"
#include "xbase/x_runes.h"

#include "xfilesystem/x_filepath.h"
#include "xfilesystem/x_dirpath.h"
#include "xfilesystem/private/x_filesystem.h"
#include "xfilesystem/private/x_filedevice.h"

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

	class xpath_parser_ascii
	{
	public:
		ascii::crunes	m_device;
		ascii::crunes	m_path;
		ascii::crunes	m_filename;
		ascii::crunes	m_extension;

		ascii::crunes	m_first_folder;

		void parse(ascii::crunes const& fullpath)
		{
			ascii::crunes slash("\\");
			ascii::crunes devicesep(":\\");
			m_device = ascii::findSelectUntilIncluded(fullpath, devicesep);
			ascii::crunes filepath = ascii::selectUntilEndExcludeSelection(fullpath, m_device);
			m_path = ascii::findLastSelectUntilIncluded(filepath, slash);
			m_filename = ascii::selectUntilEndExcludeSelection(fullpath, m_path);
			m_filename = ascii::findLastSelectUntil(m_filename, '.');
			m_extension = ascii::selectUntilEndExcludeSelection(fullpath, m_filename);
			m_first_folder = ascii::findSelectUntil(m_path, slash);

			ascii::trimRight(m_device, devicesep);
		}

		bool has_device() const { return !m_device.is_empty(); }
		bool has_path() const { return !m_path.is_empty(); }
		bool has_filename() const { return !m_filename.is_empty(); }
		bool has_extension() const { return !m_extension.is_empty(); }
		
		ascii::crunes	iterate_folder() const { return m_first_folder; }
		bool			next_folder(ascii::crunes& folder) const 
		{
			// example: projects\binary_reader\bin\ 
			folder = ascii::selectUntilEndExcludeSelection(m_path, folder);
			ascii::crunes slash("\\");
			ascii::trimLeft(folder, slash);
			return !folder.is_empty();
		}
	};

	class xpath_parser_utf32
	{
	public:
		utf32::crunes	m_device;
		utf32::crunes	m_path;
		utf32::crunes	m_filename;
		utf32::crunes	m_extension;

		utf32::crunes	m_first_folder;
		utf32::crunes	m_last_folder;

		void parse(const utf32::runes& fullpath)
		{
			utf32::rune devicesep_chars[] = {':', '\\', '\0' };
			utf32::crunes devicesep(devicesep_chars);
			m_device = utf32::findSelectUntilIncluded(fullpath, devicesep);
			utf32::crunes filepath = utf32::selectUntilEndExcludeSelection(fullpath, m_device);
			utf32::rune slash_chars[] = {'\\', '\0' };
			utf32::crunes slash(slash_chars);
			m_path = utf32::findLastSelectUntilIncluded(filepath, slash);
			m_filename = utf32::selectUntilEndExcludeSelection(fullpath, m_path);
			utf32::trimLeft(m_filename, '\\');
			utf32::trimRight(m_device, devicesep);
			m_filename = utf32::findLastSelectUntil(m_filename, '.');
			m_extension = utf32::selectUntilEndExcludeSelection(fullpath, m_filename);
		}

		bool has_device() const { return !m_device.is_empty(); }
		bool has_path() const { return !m_path.is_empty(); }
		bool has_filename() const { return !m_filename.is_empty(); }
		bool has_extension() const { return !m_extension.is_empty(); }
		
		utf32::crunes	iterate_folder() const { return m_first_folder; }
		bool			next_folder(utf32::crunes& folder) const 
		{
			return false;
		}
	};

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

    class tname_t
    {
	public:
        s32          m_refs;
        s32          m_len;
        u64          m_hash;
        utf32::rune m_name[1];

        void init(s32 strlen)
        {
            m_refs = 0;
            m_len  = strlen;
            m_hash = 0;
            m_name[0] = utf32::TERMINATOR;
        }

        bool isEmpty() const { return m_hash == 0; }

        s32 compare(const tname_t* other) const
        {
            if (m_hash == other->m_hash)
                return utf::compare(m_name, other->m_name);
            if (m_hash < other->m_hash)
                return -1;
            return 1;
        }

		s32 compare(ascii::crunes const& name) const { return utf::compare(m_name, name.m_str); }
        s32 compare(utf32::crunes const& name) const { return utf::compare(m_name, name.m_str); }

        bool operator==(const tname_t& other) const { return compare(&other) == 0; }
        bool operator!=(const tname_t& other) const { return compare(&other) != 0; }

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

		template<class T>
        static T* construct(xalloc* allocator, s32 strlen)
        {
            // Allocate a tname_t
            void* name_mem = allocator->allocate(sizeof(T) + (sizeof(utf32::rune) * strlen), sizeof(void*)); 
            T* name = static_cast<T*>(name_mem); 
            name->init(strlen); 
            return name;
        }
        static void destruct(xalloc* allocator, tname_t*& name)
        {
            allocator->deallocate(name);
            name = nullptr;
        }
    };

    class tfolder_t : public tname_t
	{
	public:
	};

    class tfilename_t : public tname_t
	{
	public:
	};
    
    class textension_t : public tname_t
	{
	public:
	};

    class tpath_t
    {
	public:
        s32        m_refs;
        s32        m_count;
        u64        m_hash;
        tfolder_t* m_folders[1];

        void init();
        void init(s32 folder_count);
        bool isEmpty() const;

		tpath_t* get_range(s32 start, s32 count);

        tfolder_t*  operator[](s32 index) const;
        tfolder_t*& operator[](s32 index);

        s32 compare(const tpath_t& other) const;

        s32 compare(utf32::crunes const& path) const;
        s32 compare(ascii::crunes const& path) const;

        bool operator==(const tpath_t& other) const;
        bool operator!=(const tpath_t& other) const;

        void reference();
        static bool release(xalloc* allocator, tpath_t*& path);
        static tpath_t* construct(xalloc* allocator, s32 folder_count);
        static void destruct(xalloc* allocator, tpath_t*& path);
    };

	// @NOTE: This class could be merged with the objects (T) themselves so as to remove
	// an additional allocation.
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

        inline s32   to_index(u64 hash) const;
        void         init(xalloc* allocator, s32 size_as_bits = 8);
        void         assign(u64 hash, T* data);
        hentry_t<T>* find(u64 hash, hentry_t<T>*& prev) const;
        void         remove(u64 hash, hentry_t<T>* head, hentry_t<T>* item);
    };

    // xroot table
    class troot_t
    {
    public:
        // API proto
        troot_t(xalloc* allocator, utf32::alloc* string_allocator) : m_allocator(allocator), m_stralloc(string_allocator) {}

		s32           m_num_devices;
		s32           m_max_devices;
        tdevice_t*    m_tdevice[64];
        xalloc*       m_allocator;
        utf32::alloc* m_stralloc;

        static tdevice_t*    sNilDevice;
        static tfolder_t*    sNilFolder;
        static tpath_t*      sNilPath;
        static tfilename_t*  sNilFilename;
        static textension_t* sNilExtension;

        htable_t<tfolder_t>    m_folder_table;
        htable_t<tpath_t>      m_path_table;
        htable_t<tfilename_t>  m_filename_table;
        htable_t<textension_t> m_extension_table;

		static troot_t* s_instance;

        void initialize(xalloc* allocator)
        {
            m_folder_table.init(allocator, 9);
            m_path_table.init(allocator, 8);
            m_filename_table.init(allocator, 10);
            m_extension_table.init(allocator, 6);
        }

		tdevice_t*	find_device(ascii::crunes const& folder_name)
		{
			for (s32 i=0; i<m_num_devices; ++i)
			{
				if (utf::compare(folder_name.m_str, m_tdevice[i]->m_name) == 0)
				{
					return m_tdevice[i];
				}
			}
			return sNilDevice;
		}

		tdevice_t*	find_device(utf32::crunes const& folder_name)
		{
			for (s32 i=0; i<m_num_devices; ++i)
			{
				if (utf::compare(folder_name.m_str, m_tdevice[i]->m_name) == 0)
				{
					return m_tdevice[i];
				}
			}
			return sNilDevice;
		}

        tfolder_t* register_folder(ascii::crunes const& folder_name)
        {
            u64 const  hfolder = generate_hash(folder_name);

			hentry_t<tfolder_t>* pprev = nullptr;
            hentry_t<tfolder_t>* pentry = m_folder_table.find(hfolder, pprev);
			// Iterate over all entries and check them against our hash and folder name
			while (pentry!=nullptr)
			{
				if (pentry->m_data->m_hash == hfolder)
				{
					s32 const c = utf::compare(pentry->m_data->m_name, folder_name.m_str);
					if (c == 0)
						break;
				}
				pprev = pentry;
				pentry = pentry->m_next;
			}

			tfolder_t* pfolder = nullptr;
            if (pentry == nullptr)
            {
                u32 const    strlen = sizeof(utf32::rune) * folder_name.size();
                u32 const    strcap = strlen + 1;
                pfolder = tfolder_t::construct<tfolder_t>(m_allocator, strcap);
                pfolder->m_len  = strlen;
                pfolder->m_hash = hfolder;
                pfolder->m_refs = 0;
				utf32::runes dststr(pfolder->m_name, pfolder->m_name, pfolder->m_name + strlen);
                utf::copy(folder_name, dststr);
            }
			else
			{
				pfolder = pentry->m_data;
			}
            return pfolder;
        }
        
		tfilename_t*  register_filename(ascii::crunes const& file_name)
		{
			// @TODO: Implement this!
			return nullptr;
		}

        textension_t* register_extension(ascii::crunes const& extension)
		{
			// @TODO: Implement this!
			return nullptr;
		}

        // ascii
        tpath_t* register_path(ascii::crunes const& fullpath, tdevice_t*& out_device)
        {
			xpath_parser_ascii parser;
			parser.parse(fullpath);

            out_device = find_device(parser.m_device);

			tpath_t*  ppath = nullptr;
			if (parser.has_path())
			{
				u64 phash = 0;
				s32 fcount = 0;
				ascii::crunes folder = parser.iterate_folder();
				// We need to iterate over the folder names to generate the hash!
				while (!folder.is_empty())
				{
					fcount += 1;
					tfolder_t* pfolder = register_folder(folder);
					phash = mix_hash(phash, pfolder->m_hash);
				}

				// Now we can see if a path object with that hash exists
				hentry_t<tpath_t>* pprev = nullptr;
				hentry_t<tpath_t>* pentry = m_path_table.find(phash, pprev);
				while (pentry != nullptr)
				{
					if (pentry->m_data->m_hash == phash && pentry->m_data->m_count == fcount)
					{
						folder = parser.iterate_folder();
						s32 findex = 0;
						do
						{
							u64 const fhash = generate_hash(folder);
							if (fhash != pentry->m_data->m_folders[findex]->m_hash)
							{
								findex = -1;
								break;
							}
							findex += 1;
						} while (parser.next_folder(folder));

						if (findex >= 0)
							break;
					}
					pentry = pentry->m_next;
				}

				if (pentry == nullptr)	// Construct a new tpath_t
				{
					ppath           = tpath_t::construct(m_allocator, fcount);

					s32 findex = 0;
					folder = parser.iterate_folder();
					while (!folder.is_empty())
					{
						tfolder_t* pfolder        = register_folder(folder);
						ppath->m_folders[findex] = pfolder;
						findex++;
					}
					m_path_table.assign(phash, ppath);
				}
				else // We found an existing one
				{
					ppath = pentry->m_data;
				}
			}
            return ppath;
        }



		tpath_t*	construct_path(s32 num_folders)
		{
			tpath_t* ppath = tpath_t::construct(m_allocator, num_folders);
			return ppath;
		}


        // utf32
        tpath_t*	register_path(utf32::crunes const& fullpath, tdevice_t*& device)
		{
			// @TODO: Implement this!
		}

        tfolder_t*    register_folder(utf32::crunes const& folder_name)
		{
			// @TODO: Implement this!
			return nullptr;
		}

        tfilename_t*  register_filename(utf32::crunes const& file_name)
		{
			// @TODO: Implement this!
			return nullptr;
		}

        textension_t* register_extension(utf32::crunes const& extension)
		{
			// @TODO: Implement this!
			return nullptr;
		}


        void release(tdevice_t*& device)
		{
			// Nothing to do
		}
		void release(tpath_t*& path)
		{
			if (tpath_t::release(m_allocator, path))
			{
				// Need to remove it from the table
			}
		}
        void release(tfolder_t*& folder)
		{
			if (tfolder_t::release(m_allocator, folder))
			{
				// Need to remove it from the table
			}
		}
        void release(tfilename_t*& filename)
		{
			if (tfilename_t::release(m_allocator, filename))
			{
				// Need to remove it from the table
			}
		}
        void release(textension_t*& extension)
		{
			if (textension_t::release(m_allocator, extension))
			{
				// Need to remove it from the table
			}
		}

    private:
        u64 generate_hash(ascii::crunes str) const
        {
			// @TODO: Implement this!
            return 0;
        }
        u64 generate_hash(utf32::crunes str) const
        {
			// @TODO: Implement this!
            return 0;
        }
        u64 mix_hash(u64 hash, u64 mix) const
        {
			// @TODO: Implement this!
            return 0;
        }
    };

    //
    // troot_t functions
    //
    void tdevice_t::init()
    {
        m_hash = 0;
        m_name = nullptr;
        m_root = nullptr;
        m_path = troot_t::sNilPath;
        m_fd   = x_NullFileDevice();
    }

    tdevice_t* tdevice_t::construct(xalloc* allocator)
    {
        // Allocate a tdevice_t
        void* device_mem = allocator->allocate(sizeof(tdevice_t), sizeof(void*)); 
        tdevice_t* device = static_cast<tdevice_t*>(device_mem); device->init(); 
        return device;
    }
    void tdevice_t::destruct(xalloc* allocator, tdevice_t*& device)
    {
        allocator->deallocate(device);
        device = nullptr;
    }


    void       tpath_t::init()
    {
        m_refs  = 0;
        m_count = 0;
        m_hash  = 0;
    }
    void tpath_t::init(s32 folder_count)
    {
        m_refs  = 1;
        m_count = folder_count;
        m_hash  = 0;
    }
    bool tpath_t::isEmpty() const { return m_folders == nullptr; }

	tpath_t* tpath_t::get_range(s32 start, s32 count)
	{
		if (count == m_count)
			return this;
		tpath_t* path = troot_t::s_instance->construct_path(count);
		for (s32 i=0; i<count; ++i)
		{
			path->m_folders[i] = m_folders[start + i];
			path->m_folders[i]->reference();
		}
		return path;
	}

    tfolder_t*  tpath_t::operator[](s32 index) const { return m_folders[index]; }
    tfolder_t*& tpath_t::operator[](s32 index) { return m_folders[index]; }

    s32 tpath_t::compare(const tpath_t& other) const
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

    bool tpath_t::operator==(const tpath_t& other) const { return compare(other) == 0; }
    bool tpath_t::operator!=(const tpath_t& other) const { return compare(other) != 0; }

    void tpath_t::reference() { m_refs++; }

    bool tpath_t::release(xalloc* allocator, tpath_t*& path)
    {
        for (s32 i = 0; i < path->m_count; ++i)
            tfolder_t::release(allocator, path->m_folders[i]);

        path->m_refs -= 1;
        if (path->m_refs == 0)
        {
            destruct(allocator, path);
            return true;
        }
        return false;
    }
    tpath_t* tpath_t::construct(xalloc* allocator, s32 folder_count)
    {
        void*    path_mem = allocator->allocate(sizeof(tpath_t) + folder_count * sizeof(void*), sizeof(void*));
        tpath_t* path     = static_cast<tpath_t*>(path_mem);
        path->init(folder_count);
        return path;
    }
    void tpath_t::destruct(xalloc* allocator, tpath_t*& path)
    {
        allocator->deallocate(path);
        path = nullptr;
    }


    // These 2 classes will be like this and as such will always be the same
    // size. If we also incorporate copy-on-write so we can easily copy and modify.
    class tdirpath_t
    {
	protected:
        tdevice_t* m_device; // tdevice_t global("") if device == ""
        tpath_t*   m_path;   // tpath_t global("") if path == ""
		friend class tfilepath_t;

    public:
		tdirpath_t();
		tdirpath_t(tdevice_t* device, tpath_t* path);
		~tdirpath_t();

        bool isEmpty() const;
        bool isRooted() const;

        void makeRelative();
        void makeRelativeTo(const tdirpath_t& dirpath);
        void makeAbsoluteTo(const tdirpath_t& dirpath);

        tdirpath_t getRoot() const;
        tdirpath_t getDirname() const;

        void up();
        void down(tdirpath_t const& dirpath);
    };

    class tfilepath_t
    {
        tdirpath_t    m_dirpath;
        tfilename_t*  m_filename;  // Should always have a filename
        textension_t* m_extension; // textension_t global("") if ext == ""

    public:
		tfilepath_t();
		tfilepath_t(tfilename_t* filename, textension_t* extension);
		~tfilepath_t();

        void clear();
        bool isEmpty() const;
        bool isRooted() const;

        void makeRelative();
        void makeRelativeTo(const tdirpath_t& dirpath);
        void makeAbsoluteTo(const tdirpath_t& dirpath);

        void setDirpath(tdirpath_t const& dirpath);
        void setFilename(tfilepath_t const& filepath);
        void setFilenameWithoutExtension(tfilepath_t const& filepath);
        void setExtension(tfilepath_t const& filepath);

        tdirpath_t  getRoot() const;
        tdirpath_t  getDirpath() const;
        tdirpath_t  getDirname() const;
        tfilepath_t getFilename() const;
        tfilepath_t getFilenameWithoutExtension() const;
        tfilepath_t getExtension() const;

        void up();
        void down(tdirpath_t const& dirpath);
    };


	tdirpath_t::tdirpath_t()
		: m_device(troot_t::sNilDevice)
		, m_path(troot_t::sNilPath)
	{
	}
	tdirpath_t::tdirpath_t(tdevice_t* device, tpath_t* path)
		: m_device(device)
		, m_path(path)
	{
	}
	tdirpath_t::~tdirpath_t()
	{
		troot_t::s_instance->release(m_device);
		troot_t::s_instance->release(m_path);
	}

    bool tdirpath_t::isEmpty() const { return m_device->isEmpty() && m_path->isEmpty(); }
    bool tdirpath_t::isRooted() const { return !m_device->isEmpty(); }

    void tdirpath_t::makeRelative() { m_device = troot_t::sNilDevice; }
    void tdirpath_t::makeRelativeTo(const tdirpath_t& dirpath) {  }
    void tdirpath_t::makeAbsoluteTo(const tdirpath_t& dirpath) {  }

    tdirpath_t tdirpath_t::getRoot() const { return tdirpath_t(m_device, troot_t::sNilPath); }
    tdirpath_t tdirpath_t::getDirname() const  { return tdirpath_t(m_device, m_path->get_range(0,1)); }

    void tdirpath_t::up() {}
    void tdirpath_t::down(tdirpath_t const& dirpath) {}


	tfilepath_t::tfilepath_t()
		: m_dirpath()
		, m_filename(troot_t::sNilFilename)
		, m_extension(troot_t::sNilExtension)
	{
	}

	tfilepath_t::tfilepath_t(tfilename_t* filename, textension_t* extension)
		: m_dirpath()
		, m_filename(filename)
		, m_extension(extension)
	{
	}

	tfilepath_t::~tfilepath_t()
	{
		troot_t::s_instance->release(m_filename);
		troot_t::s_instance->release(m_extension);
	}

    void tfilepath_t::clear()
    {
        troot_t* root = troot_t::s_instance;
        root->release(m_dirpath.m_device);
        root->release(m_dirpath.m_path);
        root->release(m_filename);
        root->release(m_extension);
    }

    bool tfilepath_t::isEmpty() const { return m_dirpath.isEmpty() && m_filename->isEmpty(); }
    bool tfilepath_t::isRooted() const { return m_dirpath.isRooted(); }

    void tfilepath_t::makeRelative() { m_dirpath.makeRelative(); }
    void tfilepath_t::makeRelativeTo(const tdirpath_t& dirpath) { m_dirpath.makeRelativeTo(dirpath); }
    void tfilepath_t::makeAbsoluteTo(const tdirpath_t& dirpath) { m_dirpath.makeAbsoluteTo(dirpath); }

    void tfilepath_t::setDirpath(tdirpath_t const& dirpath) { m_dirpath = dirpath; }
    void tfilepath_t::setFilename(tfilepath_t const& filepath)
    {
        m_filename  = filepath.m_filename;
        m_extension = filepath.m_extension;
    }
    void tfilepath_t::setFilenameWithoutExtension(tfilepath_t const& filepath) { m_filename = filepath.m_filename; }
    void tfilepath_t::setExtension(tfilepath_t const& filepath) { m_extension = filepath.m_extension; }

    tdirpath_t  tfilepath_t::getRoot() const { return m_dirpath.getRoot(); }
    tdirpath_t  tfilepath_t::getDirpath() const { return m_dirpath; }
    tdirpath_t  tfilepath_t::getDirname() const { return m_dirpath.getDirname(); }
    tfilepath_t tfilepath_t::getFilename() const { return tfilepath_t(m_filename, m_extension); }
    tfilepath_t tfilepath_t::getFilenameWithoutExtension() const { return tfilepath_t(m_filename, troot_t::sNilExtension); }
    tfilepath_t tfilepath_t::getExtension() const { return tfilepath_t(troot_t::sNilFilename, m_extension); }

    void tfilepath_t::up() { m_dirpath.up(); }
    void tfilepath_t::down(tdirpath_t const& dirpath) { m_dirpath.down(dirpath); }


	template <class T>
    inline s32 htable_t<T>::to_index(u64 hash) const
    {
        // Take N bits from the hash to come up with an index
        u64 const mask = (1 << m_size) - 1;
        return (s32)((hash & (mask << 8)) >> 8);
    }

	template <class T>
    void htable_t<T>::init(xalloc* allocator, s32 size_as_bits = 8)
    {
        m_allocator = allocator;
        m_size      = size_as_bits;
        m_data      = static_cast<hentry_t<T>**>(allocator->allocate(sizeof(hentry_t<T>*) * m_size, sizeof(void*)));
    }

	template <class T>
    void htable_t<T>::assign(u64 hash, T* data)
    {
        // NOTE: Make sure this hash/data hasn't been added before!
        s32          index = to_index(hash);
        hentry_t<T>* ptr   = static_cast<hentry_t<T>*>(m_allocator->allocate(sizeof(hentry_t<T>*), sizeof(void*)));
        ptr->init(data, m_data[index]);
        m_data[index] = ptr;
    }

	template<class T>
    hentry_t<T>* htable_t<T>::find(u64 hash, hentry_t<T>*& prev) const
    {
        s32 index = to_index(hash);
        if (m_data[index] == nullptr)
            return nullptr;

        prev = nullptr;
        hentry_t<T>* ptr = m_data[index];
        while (ptr != nullptr)
        {
            if (ptr->m_data->m_hash == hash)
            {
                return ptr;
            }
			prev = ptr;
            ptr = ptr->m_next;
        }
		prev = nullptr;
        return nullptr;
    }

	template <class T>
    void htable_t<T>::remove(u64 hash, hentry_t<T>* previous, hentry_t<T>* current)
    {
        s32 index = to_index(hash);
        if (m_data[index] == nullptr)
            return nullptr;
		if (previous == nullptr)
		{
			m_data[index] = current->m_next;
			m_allocator->deallocate(current);
		}
		else
		{
			previous->m_next = current->m_next;
			m_allocator->deallocate(current);
		}
    }

}; // namespace xcore