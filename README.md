# __xfilesystem__

## __Cross platform filesystem library__

* filesystem_t
  * filepath_t
  * dirpath_t
  * fileinfo_t
  * dirinfo_t
  * stream_t

## __TODO__

* Implement file device for Win32 and Mac
* Have filepath_t and dirpath_t use path_t
* Debug filesystem init for Win32 and Mac (use crunes_t/runes_t)
* Finish device manager (use path_t)

## __API__ (Prototype)

```c++
// API
struct file_t;
struct info_t;

class filesystem_t
{
public:
    enum emode
    {
        READ       = 0x1,
        WRITE      = 0x2,
        OPEN       = 0x10 | READ,
        CREATE     = 0x20 | WRITE,
        DISCARD    = 0x40 | WRITE,
        APPEND     = 0x80 | WRITE,
        INVALID    = 0xffffffff,
    };

    file_t*     open(filepath_t const& filename, emode mode);
    writer_t*   writer(file_t*);
    reader_t*   reader(file_t*);

    void        close(file_t*);
    void        close(info_t*);
    void        close(reader_t*);
    void        close(writer_t*);

    s64         get_pos(file_t*);
    s64         set_pos(file_t*, s64 pos);

    info_t*     info(filepath_t const& path);
    bool        exists(info_t*);
    s64         size(info_t*);
    file_t*     open(info_t*, emode mode);
    void        rename(info_t*, filepath_t const&);
    void        move(info_t* src, info_t* dst);
    void        copy(info_t* src, info_t* dst);
    void        remove(info_t*);

    s32         read(reader_t*, buffer_t&);
    s32         write(writer_t*, cbuffer_t const&);

    void        read_async(reader_t*, buffer_t&);
    s32         wait_async(reader_t*);
};
```

// ==================================================================================
// USE CASE: Initialization of a file-system

```c++
filesyscfg_t cfg;
cfg.slash = '\\';                           // The filesystem slash of the system
cfg.allocator = alloc_t::get_system();       // You can also give it its own allocator
filesystem_t* fs = filesystem_t::create(config)
```

// ======================================================================  
// USE CASE: Opening reading/writing files  

```c++
// NOTE: You can use forward and/or backwards slashes, they will be corrected  
//       automatically  
// NOTE: Reader and Writer API does not expose seek functionality, if you need  
//       this you should create a stream.

// SYNC

file_t* f = fs->open("d:/test.bin", filesystem_t::READ);
reader_t* reader = fs->reader(f);
s32 numbytesread = fs->read(reader, buffer_t& data);
fs->close(reader);
fs->close(f);

file_t* f = fs->open("d:/test.bin", filesystem_t::READ_WRITE);
writer_t* writer = fs->writer(f);
s32 numbyteswritten = fs->write(writer, cbuffer_t& data);
fs->close(writer);
fs->close(f);

// ASYNC

file_t* f = fs->open("d:/test.bin", filesystem_t::READ);
reader_t* reader = fs->reader(f);
fs->read_async(reader, buffer_t& data);
fs->wait_async(reader);
fs->close(reader);
fs->close(f);
```

// =====================================================================  
// USE CASE: Attributes and times of a file

```c++
fileinfo_t finfo = fs->info("E:/Dev.c++/main.cpp");
fileattrs_t fattrs = finfo->getAttrs();
filetimes_t ftimes = finfo->getTimes();

fattrs.setArchive(true);
finfo->setAttrs(fattrs);
```

// ===================================================================  
// USE CASE: Globbing, navigating and searching files

```c++
dirpath_t dir = fs->dirpath("E:/data");
fs->search(dir, [](fileinfo_t const* fileinfo, dirinfo_t const* dirinfo, s32 dirdepth){
    if (dirinfo!=nullptr)
    {
        // Do not recurse into any directory
        return false;
    } else if (fileinfo!=nullptr) {
        if (fileinfo->getFilepath() == "README.md")
        {
            // Do something with it and end the search
            return false;
        }
    }
    return true;    // Continue the search
});
```

// =====================================================================  
// USE CASE: Text based reading  

```c++
// NOTE: This requires you to add a dependency on xtext

file_t* f = fs->open("d:\\test.bin", filesystem_t::READ);
reader_t* reader = fs->reader(f);
//NOTE: textreader_t in the xtext package ?
textreader_t* textreader = xnew<textreader_t>(reader);
//textreader_t* textreader = fs->create<textreader>(reader); // When you want to use xfilesystem memory

string_t line;
while (textreader.readLine(line))
{
    // Do something with 'line'
}

textreader->close();
xdelete<>(textreader);

fs->close(reader);
fs->close(f);
```
