#include "xbase/x_target.h"
#include "xbase/x_debug.h"
#include "xbase/x_runes.h"

#include "xfilesystem/x_filepath.h"
#include "xfilesystem/x_dirpath.h"
#include "xfilesystem/private/x_filesystem.h"

namespace xcore
{
    /*
        What are the benifits of using a table like below to manage filepaths and dirpaths?
        - Sharing of strings
        - Easy manipulation of dirpath, can easily go to parent or child directory, likely without doing any allocations
        - You can prime the table which results in no allocations when you are using filepaths and dirpaths
        - Combining dirpath with filepath becomes very easy
        -
        
        How to quickly find existing filename, folder, extension or even path objects ?
        Hash table ?
    */
    class troot_t;

    struct tdevice_t
    {
        utf32::rune  m_runes[32]; // "appdir"
        u64          m_hash;
        tdevice_t*   m_device; // If alias, tdevice_t ("work") | nullptr
        tpath_t*     m_path;   // "xfilesystem\\"              | "dev.go\\src\\github.com\\jurgen-kluft\\"
        xfiledevice* m_fd;     // nullptr                      | xfiledevice("e")
    };

    struct tfolder_t
    {
        s32          m_refs;
        s32          m_len;
        u64          m_hash;
        utf32::prune m_name;
    };

    struct tpath_t
    {
        s32         m_refs;
        s32         m_count;
        u64         m_hash;
        tfolder_t** m_folders;
    };

    struct tfilename_t
    {
        s32          m_refs;
        s32          m_len;
        u64          m_hash;
        utf32::prune m_name;
    };

    struct textension_t
    {
        s32          m_refs;
        s32          m_len;
        u64          m_hash;
        utf32::prune m_name;
    };

    // This 2 classes become like this and as such will always be the same
    // size. If we also incorporate copy-on-write we can copy and modify.
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

        void up() {  }
        void down(xdirpath const& dirpath) {  }
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

        void getRoot(xdirpath& dirpath) const { m_dirpath.getRoot(dirpath); }
        void getDirname(xdirpath& dirpath) const { m_dirpath.getDirName(dirpath); }
        void setFilename(xfilepath const& filepath) { m_filename = filepath.m_filename; m_extension = filepath.m_extension; }
        void getFilename(xfilepath& filepath) const { filepath = xfilepath(m_dirpath.m_root, m_filename, m_extension); }        
        void setFilenameWithoutExtension(xfilepath const& filepath) { m_filename = filepath.m_filename; }
        void getFilenameWithoutExtension(xfilepath& filepath) const { filepath = xfilepath(m_dirpath.m_root, m_filename, troot_t::sNilExtension); }
        void setExtension(xfilepath const& filepath) { m_extension = filepath.m_extension; }
        void getExtension(xfilepath& filepath) const { filepath = xfilepath(m_dirpath.m_root, troot_t::sNilFilename, m_extension); }

        void up() { m_dirpath.up(); }
        void down(xdirpath const& dirpath) { m_dirpath.down(dirpath); }
    };

    // xroot table
    // You could initialize this class with a specialized allocator if
    // it can be considered that there will be no changes to folders
    // and filenames.
    // However, if you are going to do a lot of renaming of filenames
    // and folders it is advised to use a heap allocator.
    // But for something like "appdir:\" or "data:\" where you only
    // do reading operations the best possible allocator is a
    // stack allocator.
    class troot_t
    {
    public:
        // API proto
        troot_t(tdevice_t* device, xalloc* allocator);

        static tdevice_t*    sNilDevice;
        static tfolder_t*    sNilFolder;
        static tpath_t*      sNilPath;
        static tfilename_t*  sNilFilename;
        static textension_t* sNilExtension;

        // ascii
        void          register_path(ascii::crunes const& fullpath, tdevice_t*& device, tpath_t*& path, tfilename_t*& filename, textension_t*& extension);
        tfolder_t*    register_folder(ascii::crunes const& folder_name);
        tfilename_t*  register_filename(ascii::crunes const& file_name);
        textension_t* register_extension(ascii::crunes const& extension);

        // utf32
        void          register_path(utf32::crunes const& fullpath, tdevice_t*& device, tpath_t*& path, tfilename_t*& filename, textension_t*& extension);
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
        u64 hash(ascii::crunes str) const;
        u64 hash(utf32::crunes str) const;
    };
}; // namespace xcore