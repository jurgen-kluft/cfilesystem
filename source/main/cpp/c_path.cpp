#include "cbase/c_allocator.h"
#include "cbase/c_binary_search.h"
#include "cbase/c_buffer.h"
#include "ccore/c_debug.h"
#include "cbase/c_hash.h"
#include "cbase/c_runes.h"
#include "ccore/c_target.h"

#include "cfilesystem/private/c_filedevice.h"
#include "cfilesystem/private/c_filesystem.h"
#include "cfilesystem/c_dirpath.h"
#include "cfilesystem/c_filepath.h"

namespace ncore
{
    //  - Sharing of underlying strings
    //  - Easy manipulation of dirpath, can easily go to parent or child directory (if it exists), without doing any allocations
    //  - You can prime it which then results in no or a lot less allocations when you are using existing filepaths and/or dirpaths
    //  - Combining dirpath with filepath becomes very straightforward
    //  - No need to deal with forward or backward slash
    //
    //  Use cases:
    //  - From filesys_t* you can ask for the root directory of a device
    //    - filesys_t* root = filesys_t::root();
    //    - dirpath_t appdir = root->device_root("appdir");
    //  - So now with an existing dirpath_t dir, you could do the following:
    //    - dirpath_t bins = appdir.down("bin") // even if this folder doesn't exist, it will be 'added'
    //    - filepath_t coolexe = bins.file("cool.exe");
    //    - pathnode_t* datafilename; pathnode_t* dataextension;
    //    - root->filename("data.txt", datafilename, dataextension);
    //    - filepath_t datafilepath = bins.file(datafilename, dataextension);
    //    - filestream_t datastream = datafilepath.open(); // if this file doesn't exist, it will be created
    //    - .. read from the datastream
    //    - datastream.close();

    // So how do we go about implementing this?
    // The main goal is to share strings, so we are going to need a rb-tree to keep track of all the directory strings, filename strings, and extension strings.
    // When we add a directory to this tree, we will need to split the directory string into parts, and then add each part to the tree.
    // Example:
    //  - "E:\documents\books\sci-fi\", the following parts will be added to the tree:
    //    -> "E:\"
    //    -> "documents\"
    //    -> "books\"
    //    -> "sci-fi\"
    //
    // So when we add a directory like "E:\documents\music", we will need to split the string into parts, and then add each part to the tree starting from the root.
    // This means that we need to be able to refer to a parent, so that when we add "E:\documents\music", we can find "E:\" and then "documents\" and then add "music"
    // with a reference to "documents\".
    //

    class filesys_t;
    struct pathdevice_t;
    struct pathnode_t;

    // -------------------------------------------------------------------------------------------
    // fullpath parser
    //

    class fullpath_parser_utf32
    {
    public:
        crunes_t m_device;
        crunes_t m_path;
        crunes_t m_filename;
        crunes_t m_extension;
        crunes_t m_first_folder;

        void parse(const crunes_t& fullpath);

        bool has_device() const { return !m_device.is_empty(); }
        bool has_path() const { return !m_path.is_empty(); }
        bool has_filename() const { return !m_filename.is_empty(); }
        bool has_extension() const { return !m_extension.is_empty(); }

        crunes_t deviceAndPath() const;
        crunes_t path() const;

        crunes_t iterate_folder() const { return m_first_folder; }
        bool     next_folder(crunes_t& folder) const;
    };

    void fullpath_parser_utf32::parse(const crunes_t& fullpath)
    {
        utf32::rune slash_chars[] = {'\\', '/', '\0'};
        crunes_t    slash(slash_chars);
        utf32::rune devicesep_chars[] = {':', '\\', '\0'};
        crunes_t    devicesep(devicesep_chars);

        m_device          = findSelectUntilIncludedAbortAtOneOf(fullpath, devicesep, slash_chars);
        crunes_t filepath = selectAfterExclude(fullpath, m_device);
        m_path            = findLastSelectUntilIncluded(filepath, slash);
        m_filename        = selectAfterExclude(fullpath, m_path);
        m_filename        = findLastSelectUntil(m_filename, '.');
        m_extension       = selectAfterExclude(fullpath, m_filename);
        m_first_folder    = findSelectUntilIncluded(m_path, slash);
    }

    crunes_t fullpath_parser_utf32::deviceAndPath() const
    {
        // example: e:\projects\binary_reader\bin\book.pdf, "e:\projects\binary_reader\bin\"
        return selectFromToInclude(m_device, m_device, m_path);
    }

    crunes_t fullpath_parser_utf32::path() const
    {
        // example: e:\projects\binary_reader\bin\book.pdf, "projects\binary_reader\bin\"
        return m_path;
    }

    bool fullpath_parser_utf32::next_folder(crunes_t& folder) const
    {
        // example: projects\binary_reader\bin\, "projects\" -> "binary_reader\" -> "bin\"
        folder = selectAfterExclude(m_path, folder);
        trimLeft(folder, '\\');
        folder = findSelectUntil(folder, '\\');
        return !folder.is_empty();
    }

    // -------------------------------------------------------------------------------------------
    //
    // filesys_t functions
    //
    pathdevice_t* filesys_t::sNilDevice;
    pathstr_t*    filesys_t::sNilName;
    pathnode_t*   filesys_t::sNilNode;

    void filesys_t::init(alloc_t* allocator)
    {
        m_allocator = allocator;

        m_paths = m_allocator->construct<paths_t>(m_allocator);
        m_paths->init(m_allocator, 65536 * 32768);

        m_filehandles_free  = (filehandle_t**)m_allocator->allocate(sizeof(filehandle_t*) * m_max_open_files);
        m_filehandles_array = (filehandle_t*)m_allocator->allocate(sizeof(filehandle_t) * m_max_open_files);
        m_filehandles_count = m_max_open_files;
        for (s32 i = 0; i < m_filehandles_count; ++i)
        {
            m_filehandles_free[i] = &m_filehandles_array[i];
        }

        sNilName = m_paths->get_nil_name();
        sNilNode = m_paths->get_nil_node();

        sNilDevice               = &m_tdevice[0];
        sNilDevice->m_root       = this;
        sNilDevice->m_alias      = sNilName;
        sNilDevice->m_deviceName = sNilName;
        sNilDevice->m_devicePath = sNilNode;
        sNilDevice->m_redirector = nullptr;
        sNilDevice->m_fileDevice = nullptr;
        m_num_devices            = 1;
    }

    void filesys_t::exit(alloc_t* allocator)
    {
        for (s32 i = 0; i < m_num_devices; ++i)
        {
            release_name(m_tdevice[i].m_alias);
            release_name(m_tdevice[i].m_deviceName);
            release_path(m_tdevice[i].m_devicePath);
            m_tdevice[i].m_root       = nullptr;
            m_tdevice[i].m_alias      = nullptr;
            m_tdevice[i].m_deviceName = nullptr;
            m_tdevice[i].m_devicePath = nullptr;
            m_tdevice[i].m_fileDevice = nullptr;
            m_tdevice[i].m_redirector = nullptr;
        }
        m_num_devices = 0;

        m_allocator->deallocate(m_filehandles_free);
        m_allocator->deallocate(m_filehandles_array);

        m_paths->release(m_allocator);
    }

    void filesys_t::release_name(pathstr_t* p)
    {
        if (p == sNilName)
            return;
        // m_paths->detach(p);
    }

    void filesys_t::release_name(pathstr_t* p)
    {
        if (p == sNilName)
            return;
        m_paths->detach(p);
    }

    void filesys_t::release_filename(pathstr_t* p)
    {
        if (p == sNilName)
            return;
        m_paths->detach(p);
    }

    void filesys_t::release_extension(pathstr_t* p)
    {
        if (p == sNilName)
            return;
        m_paths->detach(p);
    }

    void filesys_t::release_path(pathnode_t* p)
    {
        if (p == sNilNode)
            return;
        m_paths->detach(p);
    }

    void filesys_t::release_device(pathdevice_t* dev)
    {
        if (dev == sNilDevice)
            return;
        m_paths->detach(dev);
    }

    pathstr_t* filesys_t::find_name(crunes_t const& namestr) const
    {
        pathstr_t* name = nullptr;

        return name;
    }

    void filesys_t::register_name(crunes_t const& namestr, pathstr_t*& outname) { outname = nullptr; }

    void filesys_t::register_fulldirpath(crunes_t const& fulldirpath, pathstr_t*& outdevicename, pathnode_t*& outnode)
    {
        fullpath_parser_utf32 parser;
        parser.parse(fulldirpath);

        outdevicename = sNilName;
        if (parser.has_device())
        {
            outdevicename = m_paths->findOrInsert(parser.m_device);
        }

        outnode = sNilNode;
        if (parser.has_path())
        {
            crunes_t    folder      = parser.iterate_folder();
            pathnode_t* parent_node = nullptr;
            do
            {
                pathstr_t*  folder_pathstr = m_paths->findOrInsert(folder);
                pathnode_t* folder_node    = m_paths->findOrInsert(parent_node, folder_pathstr);
                parent_node                = folder_node;
            } while (parser.next_folder(folder));
            outnode = parent_node;
        }
    }

    void filesys_t::register_dirpath(crunes_t const& dirpath, pathnode_t*& outnode)
    {
        fullpath_parser_utf32 parser;
        parser.parse(dirpath);

        outnode = sNilNode;
        if (parser.has_path())
        {
            crunes_t    folder      = parser.iterate_folder();
            pathnode_t* parent_node = nullptr;
            do
            {
                pathstr_t*  folder_pathstr = m_paths->findOrInsert(folder);
                pathnode_t* folder_node    = m_paths->findOrInsert(parent_node, folder_pathstr);
                parent_node                = folder_node;
            } while (parser.next_folder(folder));
            outnode = parent_node;
        }
    }

    void filesys_t::register_filename(crunes_t const& namestr, pathstr_t*& out_filename, pathstr_t*& out_extension)
    {
        crunes_t filename_str  = namestr;
        filename_str           = findLastSelectUntil(filename_str, '.');
        crunes_t extension_str = selectAfterExclude(namestr, filename_str);
        out_filename           = m_paths->findOrInsert(filename_str);
        out_extension          = m_paths->findOrInsert(extension_str);
    }

    void filesys_t::register_fullfilepath(crunes_t const& fullfilepath, pathstr_t*& out_device, pathnode_t*& out_path, pathstr_t*& out_filename, pathstr_t*& out_extension)
    {
        fullpath_parser_utf32 parser;
        parser.parse(fullfilepath);

        out_device = sNilName;
        out_path   = sNilNode;
        if (parser.has_device())
        {
            register_fulldirpath(parser.deviceAndPath(), out_device, out_path);
        }
        else if (parser.has_path())
        {
            register_dirpath(parser.path(), out_path);
        }

        out_filename = sNilName;
        if (parser.has_filename())
        {
            register_name(parser.m_filename, out_filename);
        }

        out_extension = sNilName;
        if (parser.has_extension())
        {
            register_name(parser.m_extension, out_extension);
        }
    }

    pathdevice_t* filesys_t::find_device(pathstr_t* devicename) const
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

    pathdevice_t* filesys_t::register_device(pathstr_t* devicename)
    {
        pathdevice_t* device = find_device(devicename);
        if (device == nullptr)
        {
            if (m_num_devices < 64)
            {
                m_tdevice[m_num_devices].m_root       = this;
                m_tdevice[m_num_devices].m_alias      = sNilName;
                m_tdevice[m_num_devices].m_deviceName = devicename;
                m_tdevice[m_num_devices].m_devicePath = sNilNode;
                m_tdevice[m_num_devices].m_redirector = nullptr;
                m_tdevice[m_num_devices].m_fileDevice = nullptr;
                device                                = &m_tdevice[m_num_devices];
                m_num_devices++;
            }
            else
            {
                device = sNilDevice;
            }
        }
        return device;
    }

    pathnode_t* filesys_t::get_parent_path(pathnode_t* path) { return path->m_parent; }

    // -------------------------------------------------------------------------------------------
    //
    // paths_t implementations
    //
    void paths_t::init(alloc_t* allocator, u32 cap)
    {
        // allocate text entries using an average of 32 bytes per string excluding the name_t header
        // @todo; these should be virtual buffers
        m_text_data_cap  = cap * (32 + sizeof(name_t));
        m_text_data      = (name_t*)allocator->allocate(m_text_data_cap, sizeof(void*));
        m_text_data_size = sizeof(name_t);

        m_node_array      = (folder_t*)allocator->allocate(sizeof(folder_t) * cap, sizeof(void*));
        m_node_free_head  = nullptr;
        m_node_free_index = 1;

        m_nil_str                = (name_t*)m_text_data;
        m_nil_str->m_hash        = 0;
        m_nil_str->m_len         = 0;
        m_nil_str->m_children[0] = 0; // left
        m_nil_str->m_children[1] = 0; // right

        m_nil_node                = m_node_array;
        m_nil_node->m_children[0] = 0; // left
        m_nil_node->m_children[1] = 0; // right
        m_nil_node->m_parent      = 0;
        m_nil_node->m_name        = m_nil_str;

        m_strings_root = nullptr;
        m_nodes_root   = nullptr;
    }

    void paths_t::release(alloc_t* allocator)
    {
        allocator->deallocate(m_text_data);
        m_text_data      = nullptr;
        m_text_data_size = 0;
        m_text_data_cap  = 0;

        m_node_array      = nullptr;
        m_node_free_head  = nullptr;
        m_node_free_index = 0;

        m_nil_node = nullptr;
        m_nil_str  = nullptr;

        m_strings_root = nullptr;
        m_nodes_root   = nullptr;
    }

    static u32 hash(utf8::pcrune str, utf8::pcrune end)
    {
        u32 hash = 0;
        while (str < end)
        {
            hash = hash + *str++ * 31;
        }
        return hash;
    }

    pathstr_t* paths_t::findOrInsert(crunes_t const& str)
    {
        // write this string to the text buffer as utf8
        name_t* str_entry = (name_t*)((u8*)m_text_data + m_text_data_size);
        str_entry->reset();

        // need a function to write crunes_t to a utf-8 buffer
        utf8::prune str8 = nullptr;
        utf8::prune end8 = nullptr;

        u32 const str_hash = hash(str8, end8);
        u32 const str_len  = end8 - str8;

        // See if we can find the string in the tree
        name_t* it = m_strings_root;
        while (it != m_nil_str)
        {
            s32 c = 0;
            if (str_hash == it->m_hash)
            {
                // binary comparison
                utf8::pcrune ostr8 = it->str();
                utf8::pcrune oend8 = it->str() + it->m_len;
                c                  = compare_buffers(str8, end8, ostr8, oend8);
                if (c == 0)
                    return it;
                c = (c + 1) >> 1;
            }
            else if (str_hash < it->m_hash)
            {
                c = 0;
            }
            else
            {
                c = 1;
            }
            it = (name_t*)((u8*)m_text_data + it->m_children[c]);
        }

        // not found, so create and add it
        it = str_entry;

        // add it to the tree

        return it;
    }

    // -------------------------------------------------------------------------------------------
    //
    // pathdevice_t implementations
    //
    void pathdevice_t::init(filesys_t* owner)
    {
        m_root       = owner;
        m_alias      = owner->sNilName;
        m_deviceName = owner->sNilName;
        m_devicePath = owner->sNilNode;
        m_redirector = nullptr;
        m_fileDevice = gNullFileDevice();
    }

    pathdevice_t* pathdevice_t::construct(alloc_t* allocator, filesys_t* owner)
    {
        void*         device_mem = allocator->allocate(sizeof(pathdevice_t), sizeof(void*));
        pathdevice_t* device     = static_cast<pathdevice_t*>(device_mem);
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
        m_root->m_paths->attach(m_alias);
        m_root->m_paths->attach(m_deviceName);
        m_root->m_paths->attach(m_devicePath);
        if (m_redirector != nullptr)
            m_redirector->attach();
        return this;
    }

    bool pathdevice_t::detach()
    {
        m_root->release_name(m_alias);
        m_root->release_name(m_deviceName);
        m_root->release_path(m_devicePath);
        if (m_redirector != nullptr)
            m_redirector->detach();

        m_alias      = m_root->sNilName;
        m_deviceName = m_root->sNilName;
        m_devicePath = m_root->sNilNode;
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
        s32                 i      = 0;
        pathdevice_t const* device = this;
        pathdevice_t const* devices[32];
        do
        {
            devices[i++] = device;
            device       = device->m_redirector;
        } while (device != nullptr && i < 32);

        device = devices[--i];

        // should be the root device (has filedevice), so first emit the device name.
        // this device should not have any device path.
        m_root->m_paths->to_string(device->m_deviceName, str);

        // the rest of the devices are aliases and should be appending their paths
        while (--i >= 0)
        {
            device = devices[i];
            m_root->m_paths->to_string(device->m_deviceName, str);
        }
    }

    s32 pathdevice_t::to_strlen() const
    {
        s32                 i      = 0;
        pathdevice_t const* device = this;
        pathdevice_t const* devices[32];
        do
        {
            devices[i++] = device;
            device       = device->m_redirector;
        } while (device != nullptr && i < 32);

        device = devices[--i];

        // should be the root device (has filedevice), so first emit the device name.
        // this device should not have any device path.
        s32 len = device->m_deviceName->m_len;

        // the rest of the devices are aliases and should be appending their paths
        while (--i >= 0)
        {
            device = devices[i];
            len += device->m_devicePath->m_name->m_len;
        }
        return len;
    }
}; // namespace ncore